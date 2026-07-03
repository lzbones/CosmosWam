#ifndef COSMOS_WAM_PIPELINE_SCHEDULER_H
#define COSMOS_WAM_PIPELINE_SCHEDULER_H

#include "cosmos_wam/remote_executor.h"
#include <string>
#include <memory>

namespace cosmos_wam {

class PipelineScheduler {
public:
    PipelineScheduler(const std::string& host, const std::string& user, const std::string& pass);

    // Runs the end-to-end 7-module pipeline.
    // Returns 0 on success, or a non-zero error code indicating which module failed.
    int runEndToEnd(const std::string& image_path, float duration, int fps, float initial_velocity);

    // Runs the local-only pipeline (Modules 5-7).
    int runLocalOnly();

private:
    // Step checks to verify if previous module outputs are complete and valid
    bool checkRemoteFileExists(const std::string& filepath);
    bool checkLocalFileExists(const std::string& filepath);

    std::string host_;
    std::string user_;
    std::string pass_;
    std::shared_ptr<RemoteExecutor> executor_;
};

} // namespace cosmos_wam

#endif // COSMOS_WAM_PIPELINE_SCHEDULER_H
