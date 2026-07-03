#ifndef COSMOS_WAM_UPLOADER_H
#define COSMOS_WAM_UPLOADER_H

#include "cosmos_wam/remote_executor.h"
#include <memory>

namespace cosmos_wam {

class Uploader {
public:
    explicit Uploader(std::shared_ptr<RemoteExecutor> executor);
    ~Uploader() = default;

    // Uploads a local image file to a remote directory.
    // Handles the .tmp suffix during transfer and renames it to final name on success.
    bool uploadImage(const std::string& local_path, const std::string& remote_dir);

private:
    std::shared_ptr<RemoteExecutor> executor_;
};

} // namespace cosmos_wam

#endif // COSMOS_WAM_UPLOADER_H
