#include "cosmos_wam/la_runner.h"
#include <iostream>

namespace cosmos_wam {

LARunner::LARunner(std::shared_ptr<RemoteExecutor> executor)
    : executor_(std::move(executor)) {}

bool LARunner::runDetection(const std::string& script_path, std::string& remote_json_output) {
    std::cout << "\n>>> [Module 3] Triggering NVIDIA LocateAnything Detection on Generated Video..." << std::endl;

    // 1. Clean up any stale remote output files from previous runs
    std::string cleanup_cmd = "rm -f /home/qingxu/software/cosmos-Cosmos3/Output/OutputVideo_detected.mp4 "
                              "/home/qingxu/software/cosmos-Cosmos3/Output/OutputVideo_detected.mp4.tmp "
                              "/home/qingxu/software/cosmos-Cosmos3/Output/OutputVideo_detected.json "
                              "/home/qingxu/software/cosmos-Cosmos3/Output/OutputVideo_detected.json.tmp";
    std::cout << "Cleaning up remote stale outputs: " << cleanup_cmd << std::endl;
    std::string cleanup_out;
    executor_->executeCommand(cleanup_cmd, cleanup_out);

    // 2. Construct remote execution command
    std::string cmd = "bash " + script_path;
    std::cout << "Executing Remote command: " << cmd << std::endl;

    std::string output;
    bool success = executor_->executeCommand(cmd, output);

    std::cout << "----- LocateAnything Script Output Start -----" << std::endl;
    std::cout << output << std::endl;
    std::cout << "----- LocateAnything Script Output End -----" << std::endl;

    if (!success) {
        std::cerr << ">>> Error: LocateAnything Detection Command execution failed." << std::endl;
        return false;
    }

    // 3. Verify that the output JSON file exists and is not a .tmp file
    std::string expected_json = "/home/qingxu/software/cosmos-Cosmos3/Output/OutputVideo_detected.json";
    if (executor_->fileExists(expected_json)) {
        remote_json_output = expected_json;
        std::cout << ">>> LocateAnything Detection Completed Successfully. Results saved at: " << remote_json_output << std::endl;
        return true;
    } else {
        std::cerr << ">>> Error: Expected JSON file " << expected_json << " was not found." << std::endl;
        return false;
    }
}

} // namespace cosmos_wam
