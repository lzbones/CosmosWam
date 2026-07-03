#include "cosmos_wam/uploader.h"
#include <iostream>

namespace cosmos_wam {

Uploader::Uploader(std::shared_ptr<RemoteExecutor> executor)
    : executor_(std::move(executor)) {}

bool Uploader::uploadImage(const std::string& local_path, const std::string& remote_dir) {
    std::cout << "\n>>> [Module 1] Uploading image to " << remote_dir << "..." << std::endl;
    
    // Remote final filename expected by run_cosmos.sh is InputImage.png
    std::string remote_tmp_path = remote_dir + "/InputImage.png.tmp";
    std::string remote_final_path = remote_dir + "/InputImage.png";

    // 1. Clean up any previous stale tmp/final files in the directory
    std::string cleanup_cmd = "rm -f " + remote_tmp_path + " " + remote_final_path;
    std::string cleanup_out;
    executor_->executeCommand(cleanup_cmd, cleanup_out);

    // 2. Upload with a .tmp extension
    std::cout << "Uploading local image: " << local_path << " -> " << remote_tmp_path << std::endl;
    if (!executor_->copyToRemote(local_path, remote_tmp_path)) {
        std::cerr << ">>> Error: Upload failed." << std::endl;
        return false;
    }

    // 3. Rename it to the final filename upon completion
    std::cout << "Renaming tmp file: " << remote_tmp_path << " -> " << remote_final_path << std::endl;
    std::string rename_cmd = "mv -f " + remote_tmp_path + " " + remote_final_path;
    std::string output;
    if (!executor_->executeCommand(rename_cmd, output)) {
        std::cerr << ">>> Error: Rename failed: " << output << std::endl;
        return false;
    }

    std::cout << ">>> Image Uploaded and Renamed Successfully!" << std::endl;
    return true;
}

} // namespace cosmos_wam
