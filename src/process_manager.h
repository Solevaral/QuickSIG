#pragma once

#include <optional>
#include <string>
#include <vector>

namespace linux_killer
{

    enum class KillOutcome
    {
        SentSigterm,
        EscalatedToSigkill,
        NotFound,
        PermissionDenied,
        Timeout,
        Failed,
    };

    struct ProcessInfo
    {
        int pid = -1;
        std::string name;
        std::string cmdline;
        std::string state;
        long vm_rss_kb = 0;
    };

    struct KillResult
    {
        int pid = -1;
        KillOutcome outcome = KillOutcome::Failed;
        std::string message;
    };

    std::vector<ProcessInfo> list_all_processes();
    std::vector<ProcessInfo> find_processes_by_name(const std::string &query);
    std::optional<ProcessInfo> find_process_by_pid(int pid);
    KillResult terminate_process(int pid, bool allow_force, int timeout_seconds = 3);

} // namespace linux_killer
