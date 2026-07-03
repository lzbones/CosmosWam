#include "cosmos_wam/downloader.h"
#include <iostream>
#include <sys/stat.h>
#include <cstdio>

namespace cosmos_wam {

Downloader::Downloader(std::shared_ptr<RemoteExecutor> executor)
    : executor_(std::move(executor)) {}

bool Downloader::downloadResults(const std::string& remote_video_path, 
                                 const std::string& remote_json_path, 
                                 const std::string& remote_ego_path,
                                 const std::string& local_destination_dir) {
    std::cout << "\n>>> [Module 4] Downloading pipeline outputs to local directory: " << local_destination_dir << std::endl;

    // Create the local destination directory if it does not exist
#ifdef _WIN32
    mkdir(local_destination_dir.c_str());
#else
    mkdir(local_destination_dir.c_str(), 0777);
#endif

    // LocateAnything script outputs annotated video with _detected.mp4 suffix
    std::string annotated_video_remote = "/home/qingxu/software/cosmos-Cosmos3/Output/OutputVideo_detected.mp4";
    std::string local_video = local_destination_dir + "/OutputVideo_detected.mp4";
    std::string local_json = local_destination_dir + "/OutputVideo_detected.json";
    std::string local_ego = local_destination_dir + "/OutputVideo_ego_trajectory.json";

    std::string local_video_tmp = local_video + ".tmp";
    std::string local_json_tmp = local_json + ".tmp";
    std::string local_ego_tmp = local_ego + ".tmp";

    // Clean up any stale tmp files
    std::remove(local_video_tmp.c_str());
    std::remove(local_json_tmp.c_str());
    std::remove(local_ego_tmp.c_str());

    bool video_ok = executor_->copyFromRemote(annotated_video_remote, local_video_tmp);
    if (video_ok) {
        std::rename(local_video_tmp.c_str(), local_video.c_str());
        std::cout << "Successfully downloaded annotated video to: " << local_video << std::endl;
    } else {
        std::cerr << "Failed to download video from " << remote_video_path << std::endl;
    }

    bool json_ok = executor_->copyFromRemote(remote_json_path, local_json_tmp);
    if (json_ok) {
        std::rename(local_json_tmp.c_str(), local_json.c_str());
        std::cout << "Successfully downloaded target detections to: " << local_json << std::endl;
    } else {
        std::cerr << "Failed to download detection JSON from " << remote_json_path << std::endl;
    }

    bool ego_ok = executor_->copyFromRemote(remote_ego_path, local_ego_tmp);
    if (ego_ok) {
        std::rename(local_ego_tmp.c_str(), local_ego.c_str());
        std::cout << "Successfully downloaded ego trajectory to: " << local_ego << std::endl;
    } else {
        std::cerr << "Failed to download ego trajectory from " << remote_ego_path << std::endl;
    }

    return (video_ok && json_ok && ego_ok);
}

} // namespace cosmos_wam
