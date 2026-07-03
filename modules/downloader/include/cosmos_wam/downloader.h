#ifndef COSMOS_WAM_DOWNLOADER_H
#define COSMOS_WAM_DOWNLOADER_H

#include "cosmos_wam/remote_executor.h"
#include <memory>

namespace cosmos_wam {

class Downloader {
public:
    explicit Downloader(std::shared_ptr<RemoteExecutor> executor);
    ~Downloader() = default;

    // Downloads the annotated video and target detection JSON from the remote host,
    // saving them to a local destination directory (which is created if it does not exist).
    bool downloadResults(const std::string& remote_video_path, 
                         const std::string& remote_json_path, 
                         const std::string& remote_ego_path,
                         const std::string& local_destination_dir);

private:
    std::shared_ptr<RemoteExecutor> executor_;
};

} // namespace cosmos_wam

#endif // COSMOS_WAM_DOWNLOADER_H
