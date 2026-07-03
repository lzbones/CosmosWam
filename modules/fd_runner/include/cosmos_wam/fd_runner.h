#ifndef COSMOS_WAM_FD_RUNNER_H
#define COSMOS_WAM_FD_RUNNER_H

#include "cosmos_wam/remote_executor.h"
#include <memory>

namespace cosmos_wam {

class FDRunner {
public:
    explicit FDRunner(std::shared_ptr<RemoteExecutor> executor);
    ~FDRunner() = default;

    // Triggers run_cosmos.sh on the remote host, displays its output in real-time,
    // and verifies that the output video file exists.
    bool runForwardDynamics(const std::string& script_path, float duration, int fps, float initial_velocity, std::string& remote_video_output);

private:
    std::shared_ptr<RemoteExecutor> executor_;
};

} // namespace cosmos_wam

#endif // COSMOS_WAM_FD_RUNNER_H
