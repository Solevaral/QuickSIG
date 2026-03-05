#include "process_manager.h"

#include <cstdlib>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <unistd.h>
#include <vector>

namespace
{

    struct CliArgs
    {
        std::string name_query;
        std::vector<int> pids;
        bool force = false;
        bool assume_yes = false;
        bool list_only = false;
        bool interactive = false;
    };

    void print_help(const char *program)
    {
        std::cout << "Usage:\n"
                  << "  " << program << " --name <query> [--force] [--yes]\n"
                  << "  " << program << " --pid <pid> [--pid <pid> ...] [--force] [--yes]\n"
                  << "  " << program << " --list\n"
                  << "  " << program << " --interactive\n\n"
                  << "Options:\n"
                  << "  -n, --name <query>    Find processes by name or command line\n"
                  << "  -p, --pid <pid>       Target specific PID (can be repeated)\n"
                  << "  -f, --force           If SIGTERM is ignored, use SIGKILL\n"
                  << "  -y, --yes             Do not ask for confirmation\n"
                  << "  -l, --list            List all running processes\n"
                  << "  -i, --interactive     Launch fzf-based selector (quicksig-gui)\n"
                  << "  -h, --help            Show this help\n";
    }

    bool parse_pid(const std::string &value, int &pid)
    {
        char *end = nullptr;
        errno = 0;
        long parsed = std::strtol(value.c_str(), &end, 10);
        if (errno != 0 || end == value.c_str() || *end != '\0' || parsed <= 0 || parsed > INT32_MAX)
        {
            return false;
        }
        pid = static_cast<int>(parsed);
        return true;
    }

    bool parse_args(int argc, char **argv, CliArgs &args)
    {
        for (int i = 1; i < argc; ++i)
        {
            const std::string arg = argv[i];

            if (arg == "-h" || arg == "--help")
            {
                print_help(argv[0]);
                return false;
            }
            if (arg == "-f" || arg == "--force")
            {
                args.force = true;
                continue;
            }
            if (arg == "-y" || arg == "--yes")
            {
                args.assume_yes = true;
                continue;
            }
            if (arg == "-l" || arg == "--list")
            {
                args.list_only = true;
                continue;
            }
            if (arg == "-i" || arg == "--interactive")
            {
                args.interactive = true;
                continue;
            }
            if (arg == "-n" || arg == "--name")
            {
                if (i + 1 >= argc)
                {
                    std::cerr << "Error: --name requires a value\n";
                    return false;
                }
                args.name_query = argv[++i];
                continue;
            }
            if (arg == "-p" || arg == "--pid")
            {
                if (i + 1 >= argc)
                {
                    std::cerr << "Error: --pid requires a value\n";
                    return false;
                }
                int pid = -1;
                if (!parse_pid(argv[++i], pid))
                {
                    std::cerr << "Error: invalid PID\n";
                    return false;
                }
                args.pids.push_back(pid);
                continue;
            }

            std::cerr << "Error: unknown argument: " << arg << "\n";
            return false;
        }

        return true;
    }

    void print_processes(const std::vector<quicksig::ProcessInfo> &processes)
    {
        std::cout << std::left << std::setw(8) << "PID" << std::setw(24) << "NAME" << std::setw(8) << "STATE"
                  << std::setw(12) << "RSS(KB)"
                  << "CMD" << "\n";
        std::cout << std::string(80, '-') << "\n";

        for (const auto &p : processes)
        {
            std::cout << std::left << std::setw(8) << p.pid << std::setw(24) << p.name.substr(0, 23)
                      << std::setw(8) << p.state << std::setw(12) << p.vm_rss_kb
                      << (p.cmdline.empty() ? "-" : p.cmdline) << "\n";
        }
    }

    std::string outcome_to_text(quicksig::KillOutcome outcome)
    {
        switch (outcome)
        {
        case quicksig::KillOutcome::SentSigterm:
            return "SIGTERM";
        case quicksig::KillOutcome::EscalatedToSigkill:
            return "SIGKILL";
        case quicksig::KillOutcome::NotFound:
            return "NOT_FOUND";
        case quicksig::KillOutcome::PermissionDenied:
            return "PERMISSION_DENIED";
        case quicksig::KillOutcome::Timeout:
            return "TIMEOUT";
        default:
            return "FAILED";
        }
    }

    bool ask_confirmation(std::size_t count)
    {
        std::cout << "Kill " << count << " process(es)? [y/N]: " << std::flush;
        std::string response;
        std::getline(std::cin, response);
        return response == "y" || response == "Y" || response == "yes" || response == "YES";
    }

    int launch_interactive()
    {
        int code = std::system("quicksig-gui");
        if (code != 0)
        {
            std::cerr << "Failed to run interactive UI. Ensure quicksig-gui is installed and executable.\n";
            return 1;
        }
        return 0;
    }

} // namespace

int main(int argc, char **argv)
{
    CliArgs args;
    if (!parse_args(argc, argv, args))
    {
        return 1;
    }

    if (args.interactive)
    {
        return launch_interactive();
    }

    if (args.list_only)
    {
        const auto all = quicksig::list_all_processes();
        print_processes(all);
        std::cout << "\nTotal: " << all.size() << " process(es)\n";
        return 0;
    }

    std::vector<quicksig::ProcessInfo> targets;

    if (!args.name_query.empty())
    {
        const auto by_name = quicksig::find_processes_by_name(args.name_query);
        targets.insert(targets.end(), by_name.begin(), by_name.end());
    }

    for (int pid : args.pids)
    {
        auto proc = quicksig::find_process_by_pid(pid);
        if (proc.has_value())
        {
            targets.push_back(*proc);
        }
        else
        {
            std::cerr << "Warning: PID " << pid << " was not found\n";
        }
    }

    if (targets.empty())
    {
        std::cerr << "No matching processes found.\n";
        return 1;
    }

    const int current_pid = static_cast<int>(getpid());
    std::set<int> seen;
    std::vector<quicksig::ProcessInfo> unique;
    for (const auto &process : targets)
    {
        if (process.pid == current_pid)
        {
            continue;
        }

        if (seen.insert(process.pid).second)
        {
            unique.push_back(process);
        }
    }

    if (unique.empty())
    {
        std::cerr << "No matching processes found (self-target filtered for safety).\n";
        return 1;
    }

    print_processes(unique);

    if (!args.assume_yes && !ask_confirmation(unique.size()))
    {
        std::cout << "Cancelled.\n";
        return 0;
    }

    int ok = 0;
    int fail = 0;
    for (const auto &process : unique)
    {
        quicksig::KillResult result = quicksig::terminate_process(process.pid, args.force);
        std::cout << "PID " << process.pid << " -> " << outcome_to_text(result.outcome) << " (" << result.message
                  << ")\n";
        if (result.outcome == quicksig::KillOutcome::SentSigterm ||
            result.outcome == quicksig::KillOutcome::EscalatedToSigkill)
        {
            ++ok;
        }
        else
        {
            ++fail;
        }
    }

    std::cout << "\nResult: success=" << ok << ", failed=" << fail << "\n";
    return fail == 0 ? 0 : 2;
}
