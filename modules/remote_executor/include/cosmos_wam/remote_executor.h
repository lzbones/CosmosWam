#ifndef COSMOS_WAM_REMOTE_EXECUTOR_H
#define COSMOS_WAM_REMOTE_EXECUTOR_H

#include <string>

namespace cosmos_wam {

class RemoteExecutor {
public:
    RemoteExecutor(std::string host, std::string user, std::string password);
    ~RemoteExecutor() = default;

    // Executes a shell command on the remote server via SSH.
    // Returns true on success (exit code 0), false otherwise.
    // Captures the stdout/stderr in the 'output' string.
    bool executeCommand(const std::string& cmd, std::string& output) const;

    // Copies a file from the remote server to the local path.
    bool copyFromRemote(const std::string& remote_path, const std::string& local_path) const;

    // Copies a file from the local path to the remote server.
    bool copyToRemote(const std::string& local_path, const std::string& remote_path) const;

    // Checks if a file exists on the remote host (returns true if yes, false if no).
    bool fileExists(const std::string& remote_path) const;

private:
    // Helper to run a local command using popen and capture its output/status.
    bool runLocalSystemCommand(const std::string& full_cmd, std::string& output) const;

    std::string host_;
    std::string user_;
    std::string password_;
};

} // namespace cosmos_wam

#endif // COSMOS_WAM_REMOTE_EXECUTOR_H
