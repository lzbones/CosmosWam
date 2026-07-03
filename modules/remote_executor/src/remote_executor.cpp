#include "cosmos_wam/remote_executor.h"
#include <cstdio>
#include <iostream>
#include <memory>
#include <array>
#include <sys/wait.h>

namespace cosmos_wam {

RemoteExecutor::RemoteExecutor(std::string host, std::string user, std::string password)
    : host_(std::move(host))
    , user_(std::move(user))
    , password_(std::move(password)) {}

bool RemoteExecutor::executeCommand(const std::string& cmd, std::string& output) const {
    // Escape special characters for nesting inside the double-quoted SSH command
    std::string escaped_cmd;
    for (char c : cmd) {
        if (c == '"' || c == '\\' || c == '$' || c == '`') {
            escaped_cmd += '\\';
        }
        escaped_cmd += c;
    }

    // Build expect command with pseudo-terminal allocation (-tt) and infinite/high timeout (600 seconds)
    // to allow long-running deep learning inference pipelines to finish completely.
    std::string full_cmd = "expect -c '\n"
                           "set timeout 600\n"
                           "spawn ssh -tt -o StrictHostKeyChecking=no " + user_ + "@" + host_ + " \"" + escaped_cmd + "\"\n"
                           "expect {\n"
                           "  \"*password:\" { send \"" + password_ + "\\r\"; exp_continue }\n"
                           "  eof\n"
                           "}\n"
                           "catch wait result\n"
                           "exit [lindex $result 3]\n"
                           "'";

    return runLocalSystemCommand(full_cmd, output);
}

bool RemoteExecutor::copyFromRemote(const std::string& remote_path, const std::string& local_path) const {
    std::string full_cmd = "expect -c '\n"
                           "set timeout 600\n"
                           "spawn scp -o StrictHostKeyChecking=no " + user_ + "@" + host_ + ":" + remote_path + " " + local_path + "\n"
                           "expect {\n"
                           "  \"*password:\" { send \"" + password_ + "\\r\"; exp_continue }\n"
                           "  eof\n"
                           "}\n"
                           "catch wait result\n"
                           "exit [lindex $result 3]\n"
                           "'";
    std::string output;
    bool success = runLocalSystemCommand(full_cmd, output);
    if (!success) {
        std::cerr << "SCP Copy From Remote Failed: " << output << std::endl;
    }
    return success;
}

bool RemoteExecutor::copyToRemote(const std::string& local_path, const std::string& remote_path) const {
    std::string full_cmd = "expect -c '\n"
                           "set timeout 600\n"
                           "spawn scp -o StrictHostKeyChecking=no " + local_path + " " + user_ + "@" + host_ + ":" + remote_path + "\n"
                           "expect {\n"
                           "  \"*password:\" { send \"" + password_ + "\\r\"; exp_continue }\n"
                           "  eof\n"
                           "}\n"
                           "catch wait result\n"
                           "exit [lindex $result 3]\n"
                           "'";
    std::string output;
    bool success = runLocalSystemCommand(full_cmd, output);
    if (!success) {
        std::cerr << "SCP Copy To Remote Failed: " << output << std::endl;
    }
    return success;
}

bool RemoteExecutor::fileExists(const std::string& remote_path) const {
    std::string output;
    std::string cmd = "test -f " + remote_path;
    return executeCommand(cmd, output);
}

bool RemoteExecutor::runLocalSystemCommand(const std::string& full_cmd, std::string& output) const {
    std::array<char, 256> buffer;
    output.clear();

    // Redirect stderr to stdout to capture everything
    std::string cmd_with_stderr = full_cmd + " 2>&1";
    FILE* pipe = popen(cmd_with_stderr.c_str(), "r");
    if (!pipe) {
        output = "Error: popen execution failed.";
        return false;
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        output += buffer.data();
    }

    int return_code = pclose(pipe);
    int status = WIFEXITED(return_code) ? WEXITSTATUS(return_code) : return_code;
    return (status == 0);
}

} // namespace cosmos_wam
