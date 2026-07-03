#include "cosmos_wam/fd_runner.h"
#include <iostream>
#include <sys/stat.h>
#include <cstdio>

namespace cosmos_wam {

FDRunner::FDRunner(std::shared_ptr<RemoteExecutor> executor)
    : executor_(std::move(executor)) {}

bool FDRunner::runForwardDynamics(const std::string& script_path, float duration, int fps, float initial_velocity, std::string& remote_video_output) {
    std::cout << "\n>>> [Module 2] Triggering Forward Dynamics (FD) Video Generation on Remote Host..." << std::endl;

    // 1. Clean up any stale remote output files from previous runs
    std::string cleanup_cmd = "rm -f /home/qingxu/software/cosmos-Cosmos3/Output/OutputVideo.mp4 "
                              "/home/qingxu/software/cosmos-Cosmos3/Output/OutputVideo.tmp";
    std::cout << "Cleaning up remote stale outputs: " << cleanup_cmd << std::endl;
    std::string cleanup_out;
    executor_->executeCommand(cleanup_cmd, cleanup_out);

    // 2. Construct remote execution command with duration, fps and initial_velocity
    std::string cmd = "bash " + script_path + " " 
                    + std::to_string(duration) + " " 
                    + std::to_string(fps) + " " 
                    + std::to_string(initial_velocity);
    std::cout << "Executing Remote command: " << cmd << std::endl;

    std::string output;
    bool success = executor_->executeCommand(cmd, output);

    std::cout << "----- FD Script Output Start -----" << std::endl;
    std::cout << output << std::endl;
    std::cout << "----- FD Script Output End -----" << std::endl;

    if (!success) {
        std::cerr << ">>> Error: FD Video Generation Command execution failed." << std::endl;
        return false;
    }

    // 3. Verify that the output video file exists and is not a .tmp file
    std::string expected_video = "/home/qingxu/software/cosmos-Cosmos3/Output/OutputVideo.mp4";
    if (executor_->fileExists(expected_video)) {
        remote_video_output = expected_video;
        std::cout << ">>> FD Video Generation Completed Successfully. Video saved at: " << remote_video_output << std::endl;

        // Create the local output directory if it does not exist
#ifdef _WIN32
        mkdir("./output");
#else
        mkdir("./output", 0777);
#endif

        // Download the forecast video to local output/ directory
        std::string local_video = "./output/OutputVideo.mp4";
        std::string local_video_tmp = local_video + ".tmp";
        std::remove(local_video_tmp.c_str());

        std::cout << "Downloading forecast video to: " << local_video << std::endl;
        if (executor_->copyFromRemote(expected_video, local_video_tmp)) {
            std::rename(local_video_tmp.c_str(), local_video.c_str());
            std::cout << "Successfully downloaded forecast video to: " << local_video << std::endl;
            return true;
        } else {
            std::cerr << ">>> Error: Failed to download forecast video to " << local_video << std::endl;
            return false;
        }
    } else {
        std::cerr << ">>> Error: Expected video file " << expected_video << " was not found." << std::endl;
        return false;
    }
}

} // namespace cosmos_wam
