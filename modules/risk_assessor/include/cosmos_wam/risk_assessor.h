#ifndef COSMOS_WAM_RISK_ASSESSOR_H
#define COSMOS_WAM_RISK_ASSESSOR_H

#include <string>
#include <vector>

namespace cosmos_wam {

struct RiskPoint {
    double timestamp;
    double x;
    double y;
    double z;
    double vx;
    double vy;
    double speed;
    double heading; // heading angle in radians
};

struct EgoState {
    int frame_index;
    double timestamp;
    double x;
    double y;
    double z;
    double roll;
    double pitch;
    double yaw; // in degrees
    double vx;
    double vy;
    double speed;
    double ax;
    double ay;
    double accel_mag;
    double stability_risk;
};

struct EnvVehicleTrajectory {
    std::string id;
    std::vector<RiskPoint> points;
};

struct RiskGrid {
    double t_s;
    int grid_size_x;
    int grid_size_y;
    double resolution_m;
    std::vector<double> data; // row-major flat array of size grid_size_x * grid_size_y
};

class RiskAssessor {
public:
    RiskAssessor();
    ~RiskAssessor() = default;

    // Evaluates risk fields over the combined trajectory JSON and saves the BEV risk map.
    bool assessRiskAndSave(const std::string& input_trajectories_json, 
                           const std::string& output_risk_json);

private:
    // Computes velocities and accelerations for ego and environment vehicles
    void reconstructKinematics(std::vector<EgoState>& ego_states, 
                               std::vector<EnvVehicleTrajectory>& env_trajectories);
};

} // namespace cosmos_wam

#endif // COSMOS_WAM_RISK_ASSESSOR_H
