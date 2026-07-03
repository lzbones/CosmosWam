#ifndef COSMOS_WAM_LA_RUNNER_H
#define COSMOS_WAM_LA_RUNNER_H

#include "cosmos_wam/remote_executor.h"
#include <memory>

namespace cosmos_wam {

class LARunner {
public:
    explicit LARunner(std::shared_ptr<RemoteExecutor> executor);
    ~LARunner() = default;

    // Triggers run_locateAnything.sh on the remote host, displays its output in real-time,
    // and verifies that the output JSON file exists.
    bool runDetection(const std::string& script_path, std::string& remote_json_output);

private:
    std::shared_ptr<RemoteExecutor> executor_;
};

} // namespace cosmos_wam

#endif // COSMOS_WAM_LA_RUNNER_H
