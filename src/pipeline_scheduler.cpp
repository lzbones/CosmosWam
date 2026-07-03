#include "cosmos_wam/pipeline_scheduler.h"
#include "cosmos_wam/uploader.h"
#include "cosmos_wam/fd_runner.h"
#include "cosmos_wam/la_runner.h"
#include "cosmos_wam/downloader.h"
#include "cosmos_wam/target_tracker.h"
#include "cosmos_wam/coord_transformer.h"
#include "cosmos_wam/risk_assessor.h"
#include <iostream>
#include <fstream>

namespace cosmos_wam {

PipelineScheduler::PipelineScheduler(const std::string& host, const std::string& user, const std::string& pass)
    : host_(host), user_(user), pass_(pass) {
    if (host != "local") {
        executor_ = std::make_shared<RemoteExecutor>(host, user, pass);
    }
}

bool PipelineScheduler::checkRemoteFileExists(const std::string& filepath) {
    if (!executor_) return false;
    return executor_->fileExists(filepath);
}

bool PipelineScheduler::checkLocalFileExists(const std::string& filepath) {
    std::ifstream f(filepath);
    return f.good();
}

int PipelineScheduler::runEndToEnd(const std::string& image_path, float duration, int fps, float initial_velocity) {
    std::cout << ">>> Running end-to-end CosmosWAM software pipeline..." << std::endl;

    // 1. Establish and check SSH connection
    std::cout << "Checking SSH connectivity to remote server..." << std::endl;
    std::string check_output;
    bool connected = executor_->executeCommand("whoami && hostname", check_output);
    if (!connected) {
        std::cerr << ">>> Connection check failed! Output:\n" << check_output << std::endl;
        return 1;
    }
    std::cout << ">>> Remote Connection OK! Running on remote host as: " << check_output;

    // 2. Module 1: Upload image
    Uploader uploader(executor_);
    std::string remote_input_dir = "/home/qingxu/software/cosmos-Cosmos3/Input";
    std::string remote_input_file = remote_input_dir + "/InputImage.png";

    if (!uploader.uploadImage(image_path, remote_input_dir)) {
        std::cerr << "Pipeline stopped: Module 1 (Image Upload) failed." << std::endl;
        return 2;
    }
    // Verify Module 1 output (InputImage.png must exist and not be tmp)
    if (!checkRemoteFileExists(remote_input_file)) {
        std::cerr << "Step verification failed: Uploaded image " << remote_input_file << " was not found on remote server." << std::endl;
        return 22;
    }
    std::cout << ">>> [Step 1 Verified] InputImage.png uploaded successfully." << std::endl;

    // 3. Module 2: FD Video Generation
    FDRunner fd_runner(executor_);
    std::string remote_video;
    std::string remote_cosmos_script = "/home/qingxu/software/cosmos-Cosmos3/run_cosmos.sh";
    if (!fd_runner.runForwardDynamics(remote_cosmos_script, duration, fps, initial_velocity, remote_video)) {
        std::cerr << "Pipeline stopped: Module 2 (Forward Dynamics) failed." << std::endl;
        return 3;
    }
    // Verify Module 2 output (OutputVideo.mp4 must exist and not be tmp)
    if (!checkRemoteFileExists(remote_video)) {
        std::cerr << "Step verification failed: Generated video " << remote_video << " was not found on remote server." << std::endl;
        return 33;
    }
    std::cout << ">>> [Step 2 Verified] Remote video generated successfully: " << remote_video << std::endl;

    // 4. Module 3: Target Detection
    LARunner la_runner(executor_);
    std::string remote_json;
    std::string remote_locate_script = "/home/qingxu/software/cosmos-Cosmos3/run_locateAnything.sh";
    if (!la_runner.runDetection(remote_locate_script, remote_json)) {
        std::cerr << "Pipeline stopped: Module 3 (Target Detection) failed." << std::endl;
        return 4;
    }
    // Verify Module 3 output (OutputVideo_detected.json must exist and not be tmp)
    if (!checkRemoteFileExists(remote_json)) {
        std::cerr << "Step verification failed: Detection JSON " << remote_json << " was not found on remote server." << std::endl;
        return 44;
    }
    std::cout << ">>> [Step 3 Verified] Remote target detection completed successfully: " << remote_json << std::endl;

    // 5. Module 4: Downloader
    Downloader downloader(executor_);
    std::string local_output_dir = "./output";
    std::string remote_ego = "/home/qingxu/software/cosmos-Cosmos3/Output/OutputVideo_ego_trajectory.json";
    if (!downloader.downloadResults(remote_video, remote_json, remote_ego, local_output_dir)) {
        std::cerr << "Pipeline stopped: Module 4 (Downloader) failed." << std::endl;
        return 5;
    }

    std::string local_json = local_output_dir + "/OutputVideo_detected.json";
    std::string local_ego = local_output_dir + "/OutputVideo_ego_trajectory.json";

    // Verify Module 4 outputs (Downloaded files must exist and not be tmp)
    if (!checkLocalFileExists(local_json) || !checkLocalFileExists(local_ego)) {
        std::cerr << "Step verification failed: Downloaded detection files were not found locally." << std::endl;
        return 55;
    }
    std::cout << ">>> [Step 4 Verified] Outputs downloaded locally to: " << local_output_dir << std::endl;

    // 6. Module 5: Target Tracker
    std::cout << "\n>>> [Module 5] Performing Target Tracking in C++..." << std::endl;
    TargetTracker tracker;
    std::vector<DetectionFrame> frames;
    if (!tracker.parseDetections(local_json, frames)) {
        std::cerr << "Pipeline stopped: Module 5 (Target Tracker) failed to parse detections." << std::endl;
        return 6;
    }
    auto tracks = tracker.trackObjects(frames);
    std::cout << ">>> Target Tracking Completed: Associated " << tracks.size() << " unique vehicle trajectories." << std::endl;

    // 7. Module 6: Coordinate Transformer
    std::cout << "\n>>> [Module 6] Performing Coordinate Transformation (IPM -> ENU -> WGS-84) in C++..." << std::endl;
    CoordTransformer transformer;
    ReferencePoint ref_point;
    std::vector<EgoPose> ego_trajectory;
    if (!transformer.parseEgoTrajectory(local_ego, ref_point, ego_trajectory)) {
        std::cerr << "Pipeline stopped: Module 6 (Coordinate Transformer) failed to parse ego trajectory." << std::endl;
        return 7;
    }
    std::string output_trajectories_json = local_output_dir + "/OutputVideo_trajectories.json";
    if (!transformer.transformAndSave(tracks, ego_trajectory, ref_point, output_trajectories_json)) {
        std::cerr << "Pipeline stopped: Module 6 (Coordinate Transformer) failed to save trajectories." << std::endl;
        return 8;
    }
    // Verify Module 6 output (OutputVideo_trajectories.json must exist and not be tmp)
    if (!checkLocalFileExists(output_trajectories_json)) {
        std::cerr << "Step verification failed: Trajectories file " << output_trajectories_json << " was not found." << std::endl;
        return 88;
    }
    std::cout << ">>> [Step 6 Verified] Coordinate trajectories saved successfully." << std::endl;

    // 8. Module 7: Risk Assessor
    std::cout << "\n>>> [Module 7] Performing Risk Assessment in C++..." << std::endl;
    RiskAssessor risk_assessor;
    std::string output_risk_json = local_output_dir + "/frame_result.json";
    if (!risk_assessor.assessRiskAndSave(output_trajectories_json, output_risk_json)) {
        std::cerr << "Pipeline stopped: Module 7 (Risk Assessor) failed to save risk map." << std::endl;
        return 9;
    }
    // Verify Module 7 output (frame_result.json must exist and not be tmp)
    if (!checkLocalFileExists(output_risk_json)) {
        std::cerr << "Step verification failed: Risk grid file " << output_risk_json << " was not found." << std::endl;
        return 99;
    }
    std::cout << ">>> [Step 7 Verified] Risk assessor completed successfully." << std::endl;

    std::cout << "\n=========================================================" << std::endl;
    std::cout << "            Pipeline Completed Successfully!            " << std::endl;
    std::cout << "=========================================================" << std::endl;

    return 0;
}

int PipelineScheduler::runLocalOnly() {
    std::cout << ">>> Running in LOCAL ONLY mode (skipping remote SSH execution)..." << std::endl;
    std::string local_output_dir = "./output";
    std::string local_json = local_output_dir + "/OutputVideo_detected.json";
    std::string local_ego = local_output_dir + "/OutputVideo_ego_trajectory.json";

    // Verify local inputs exist
    if (!checkLocalFileExists(local_json) || !checkLocalFileExists(local_ego)) {
        std::cerr << "Pipeline stopped: Local input files " << local_json << " or " << local_ego << " not found." << std::endl;
        return 55;
    }

    // 1. Module 5: Target tracking in C++
    std::cout << "\n>>> [Module 5] Performing Target Tracking in C++..." << std::endl;
    TargetTracker tracker;
    std::vector<DetectionFrame> frames;
    if (!tracker.parseDetections(local_json, frames)) {
        std::cerr << "Pipeline stopped: Module 5 (Target Tracker) failed to parse detections." << std::endl;
        return 6;
    }
    auto tracks = tracker.trackObjects(frames);
    std::cout << ">>> Target Tracking Completed: Associated " << tracks.size() << " unique vehicle trajectories." << std::endl;

    // 2. Module 6: Coordinate Transformer
    std::cout << "\n>>> [Module 6] Performing Coordinate Transformation (IPM -> ENU -> WGS-84) in C++..." << std::endl;
    CoordTransformer transformer;
    ReferencePoint ref_point;
    std::vector<EgoPose> ego_trajectory;
    if (!transformer.parseEgoTrajectory(local_ego, ref_point, ego_trajectory)) {
        std::cerr << "Pipeline stopped: Module 6 (Coordinate Transformer) failed to parse ego trajectory." << std::endl;
        return 7;
    }
    std::string output_trajectories_json = local_output_dir + "/OutputVideo_trajectories.json";
    if (!transformer.transformAndSave(tracks, ego_trajectory, ref_point, output_trajectories_json)) {
        std::cerr << "Pipeline completed but Module 6 (Coordinate Transformer) failed to save trajectories." << std::endl;
        return 8;
    }
    if (!checkLocalFileExists(output_trajectories_json)) {
        std::cerr << "Step verification failed: Trajectories file " << output_trajectories_json << " was not found." << std::endl;
        return 88;
    }

    // 3. Module 7: Risk Assessor
    std::cout << "\n>>> [Module 7] Performing Risk Assessment in C++..." << std::endl;
    RiskAssessor risk_assessor;
    std::string output_risk_json = local_output_dir + "/frame_result.json";
    if (!risk_assessor.assessRiskAndSave(output_trajectories_json, output_risk_json)) {
        std::cerr << "Pipeline completed but Module 7 (Risk Assessor) failed to save risk map." << std::endl;
        return 9;
    }
    if (!checkLocalFileExists(output_risk_json)) {
        std::cerr << "Step verification failed: Risk grid file " << output_risk_json << " was not found." << std::endl;
        return 99;
    }

    std::cout << "\n=========================================================" << std::endl;
    std::cout << "       Local-Only Pipeline Completed Successfully!       " << std::endl;
    std::cout << "=========================================================" << std::endl;

    return 0;
}

} // namespace cosmos_wam
