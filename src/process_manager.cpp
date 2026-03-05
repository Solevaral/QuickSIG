#include "process_manager.h"

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <sstream>
#include <string>
#include <sys/types.h>
#include <thread>
#include <unistd.h>

namespace quicksig
{
    namespace
    {

        bool is_number(const std::string &s)
        {
            if (s.empty())
            {
                return false;
            }
            return std::all_of(s.begin(), s.end(), [](unsigned char c)
                               { return std::isdigit(c) != 0; });
        }

        std::string to_lower(std::string value)
        {
            std::transform(value.begin(), value.end(), value.begin(),
                           [](unsigned char c)
                           { return static_cast<char>(std::tolower(c)); });
            return value;
        }

        std::string read_file_trimmed(const std::string &path)
        {
            std::ifstream in(path);
            if (!in)
            {
                return "";
            }

            std::ostringstream ss;
            ss << in.rdbuf();
            std::string content = ss.str();

            while (!content.empty() && (content.back() == '\n' || content.back() == '\r' || content.back() == '\0'))
            {
                content.pop_back();
            }
            return content;
        }

        std::string read_cmdline(const std::string &path)
        {
            std::ifstream in(path, std::ios::binary);
            if (!in)
            {
                return "";
            }

            std::string data((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
            for (char &ch : data)
            {
                if (ch == '\0')
                {
                    ch = ' ';
                }
            }

            while (!data.empty() && data.back() == ' ')
            {
                data.pop_back();
            }

            return data;
        }

        long parse_vm_rss_kb(const std::string &status_path)
        {
            std::ifstream in(status_path);
            if (!in)
            {
                return 0;
            }

            std::string line;
            while (std::getline(in, line))
            {
                if (line.rfind("VmRSS:", 0) == 0)
                {
                    std::istringstream ss(line);
                    std::string key;
                    long value = 0;
                    std::string unit;
                    ss >> key >> value >> unit;
                    return value;
                }
            }

            return 0;
        }

        std::string parse_state(const std::string &status_path)
        {
            std::ifstream in(status_path);
            if (!in)
            {
                return "?";
            }

            std::string line;
            while (std::getline(in, line))
            {
                if (line.rfind("State:", 0) == 0)
                {
                    std::istringstream ss(line);
                    std::string key;
                    std::string code;
                    ss >> key >> code;
                    return code.empty() ? "?" : code;
                }
            }

            return "?";
        }

        bool is_alive(int pid)
        {
            if (pid <= 0)
            {
                return false;
            }

            if (kill(pid, 0) == 0)
            {
                return true;
            }
            return errno != ESRCH;
        }

    } // namespace

    std::optional<ProcessInfo> find_process_by_pid(int pid)
    {
        if (pid <= 0)
        {
            return std::nullopt;
        }

        const std::string base = "/proc/" + std::to_string(pid);
        const std::string comm = read_file_trimmed(base + "/comm");
        if (comm.empty())
        {
            return std::nullopt;
        }

        ProcessInfo info;
        info.pid = pid;
        info.name = comm;
        info.cmdline = read_cmdline(base + "/cmdline");
        info.state = parse_state(base + "/status");
        info.vm_rss_kb = parse_vm_rss_kb(base + "/status");
        return info;
    }

    std::vector<ProcessInfo> list_all_processes()
    {
        std::vector<ProcessInfo> result;

        DIR *proc = opendir("/proc");
        if (proc == nullptr)
        {
            return result;
        }

        struct dirent *entry = nullptr;
        while ((entry = readdir(proc)) != nullptr)
        {
            if (entry->d_type != DT_DIR && entry->d_type != DT_UNKNOWN)
            {
                continue;
            }

            const std::string name(entry->d_name);
            if (!is_number(name))
            {
                continue;
            }

            const int pid = std::stoi(name);
            auto process = find_process_by_pid(pid);
            if (process.has_value())
            {
                result.push_back(*process);
            }
        }

        closedir(proc);

        std::sort(result.begin(), result.end(), [](const ProcessInfo &a, const ProcessInfo &b)
                  {
        if (a.name == b.name) {
            return a.pid < b.pid;
        }
        return a.name < b.name; });

        return result;
    }

    std::vector<ProcessInfo> find_processes_by_name(const std::string &query)
    {
        const std::string q = to_lower(query);
        std::vector<ProcessInfo> result;

        for (const auto &process : list_all_processes())
        {
            const std::string lower_name = to_lower(process.name);
            const std::string lower_cmd = to_lower(process.cmdline);
            if (lower_name.find(q) != std::string::npos || lower_cmd.find(q) != std::string::npos)
            {
                result.push_back(process);
            }
        }

        return result;
    }

    KillResult terminate_process(int pid, bool allow_force, int timeout_seconds)
    {
        KillResult result;
        result.pid = pid;

        if (pid <= 0)
        {
            result.outcome = KillOutcome::Failed;
            result.message = "invalid PID";
            return result;
        }

        if (kill(pid, SIGTERM) != 0)
        {
            if (errno == ESRCH)
            {
                result.outcome = KillOutcome::NotFound;
                result.message = "process does not exist";
                return result;
            }
            if (errno == EPERM)
            {
                result.outcome = KillOutcome::PermissionDenied;
                result.message = "permission denied (try sudo)";
                return result;
            }
            result.outcome = KillOutcome::Failed;
            result.message = std::string("SIGTERM failed: ") + std::strerror(errno);
            return result;
        }

        const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(timeout_seconds);
        while (std::chrono::steady_clock::now() < deadline)
        {
            if (!is_alive(pid))
            {
                result.outcome = KillOutcome::SentSigterm;
                result.message = "terminated with SIGTERM";
                return result;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        if (!allow_force)
        {
            result.outcome = KillOutcome::Timeout;
            result.message = "still running after SIGTERM";
            return result;
        }

        if (kill(pid, SIGKILL) != 0)
        {
            if (errno == ESRCH)
            {
                result.outcome = KillOutcome::EscalatedToSigkill;
                result.message = "terminated after timeout";
                return result;
            }
            if (errno == EPERM)
            {
                result.outcome = KillOutcome::PermissionDenied;
                result.message = "permission denied for SIGKILL (try sudo)";
                return result;
            }
            result.outcome = KillOutcome::Failed;
            result.message = std::string("SIGKILL failed: ") + std::strerror(errno);
            return result;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        result.outcome = KillOutcome::EscalatedToSigkill;
        result.message = "killed with SIGKILL after timeout";
        return result;
    }

} // namespace quicksig
