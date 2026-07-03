#include "cosmos_wam/risk_assessor.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <cstdio>
#include <map>

namespace cosmos_wam {

// Degrees to radians helper
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

inline double deg2rad(double deg) {
    return deg * M_PI / 180.0;
}

RiskAssessor::RiskAssessor() {}

void RiskAssessor::reconstructKinematics(std::vector<EgoState>& ego_states, 
                                          std::vector<EnvVehicleTrajectory>& env_trajectories) {
    // 1. Ego vehicle kinematics reconstruction
    for (size_t i = 0; i < ego_states.size(); ++i) {
        if (i == 0) {
            ego_states[i].vx = 0.0;
            ego_states[i].vy = 0.0;
            ego_states[i].speed = 0.0;
        } else {
            double dt = ego_states[i].timestamp - ego_states[i-1].timestamp;
            if (dt > 0.001) {
                ego_states[i].vx = (ego_states[i].x - ego_states[i-1].x) / dt;
                ego_states[i].vy = (ego_states[i].y - ego_states[i-1].y) / dt;
            } else {
                ego_states[i].vx = ego_states[i-1].vx;
                ego_states[i].vy = ego_states[i-1].vy;
            }
            ego_states[i].speed = std::sqrt(ego_states[i].vx * ego_states[i].vx + ego_states[i].vy * ego_states[i].vy);
        }
    }
    // Fill first frame ego velocity with frame 1 velocity
    if (ego_states.size() > 1) {
        ego_states[0].vx = ego_states[1].vx;
        ego_states[0].vy = ego_states[1].vy;
        ego_states[0].speed = ego_states[1].speed;
    }

    // Numerical differentiation for ego acceleration
    for (size_t i = 0; i < ego_states.size(); ++i) {
        if (i < 2) {
            ego_states[i].ax = 0.0;
            ego_states[i].ay = 0.0;
            ego_states[i].accel_mag = 0.0;
        } else {
            double dt = ego_states[i].timestamp - ego_states[i-1].timestamp;
            if (dt > 0.001) {
                ego_states[i].ax = (ego_states[i].vx - ego_states[i-1].vx) / dt;
                ego_states[i].ay = (ego_states[i].vy - ego_states[i-1].vy) / dt;
            } else {
                ego_states[i].ax = ego_states[i-1].ax;
                ego_states[i].ay = ego_states[i-1].ay;
            }
            ego_states[i].accel_mag = std::sqrt(ego_states[i].ax * ego_states[i].ax + ego_states[i].ay * ego_states[i].ay);
        }

        // Friction circle stability index calculation
        double mu = 0.8; // dry asphalt friction coefficient
        double g = 9.81;
        double eta = ego_states[i].accel_mag / (mu * g);
        double R_stab = 0.0;
        if (eta >= 0.6) {
            if (eta > 1.0) {
                R_stab = 1.0;
            } else {
                R_stab = (eta - 0.6) / 0.4;
            }
        }
        ego_states[i].stability_risk = R_stab;
    }

    // 2. Environmental vehicle kinematics reconstruction
    for (auto& env : env_trajectories) {
        auto& pts = env.points;
        for (size_t i = 0; i < pts.size(); ++i) {
            if (i == 0) {
                pts[i].vx = 0.0;
                pts[i].vy = 0.0;
                pts[i].speed = 0.0;
                pts[i].heading = 0.0;
            } else {
                double dt = pts[i].timestamp - pts[i-1].timestamp;
                if (dt > 0.001) {
                    pts[i].vx = (pts[i].x - pts[i-1].x) / dt;
                    pts[i].vy = (pts[i].y - pts[i-1].y) / dt;
                } else {
                    pts[i].vx = pts[i-1].vx;
                    pts[i].vy = pts[i-1].vy;
                }
                pts[i].speed = std::sqrt(pts[i].vx * pts[i].vx + pts[i].vy * pts[i].vy);
                pts[i].heading = std::atan2(pts[i].vy, pts[i].vx);
            }
        }
        if (pts.size() > 1) {
            pts[0].vx = pts[1].vx;
            pts[0].vy = pts[1].vy;
            pts[0].speed = pts[1].speed;
            pts[0].heading = pts[1].heading;
        }
    }
}

bool RiskAssessor::assessRiskAndSave(const std::string& input_trajectories_json, 
                                     const std::string& output_risk_json) {
    std::ifstream file(input_trajectories_json);
    if (!file.is_open()) {
        std::cerr << "Failed to open input trajectories JSON file: " << input_trajectories_json << std::endl;
        return false;
    }

    // Simple line-by-line custom JSON parser for OutputVideo_trajectories.json
    std::string line;
    std::vector<EgoState> ego_states;
    std::vector<EnvVehicleTrajectory> env_trajectories;

    bool in_ego = false;
    bool in_env = false;
    EgoState current_ego;
    current_ego.frame_index = -1;

    std::string current_env_id;
    std::vector<RiskPoint> current_env_pts;
    RiskPoint current_env_pt;
    current_env_pt.timestamp = -1.0;

    while (std::getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.rfind("\"ego_trajectory\":", 0) == 0) {
            in_ego = true;
            in_env = false;
        } else if (line.rfind("\"environmental_vehicles\":", 0) == 0) {
            in_ego = false;
            in_env = true;
        } else if (in_ego) {
            if (line.rfind("{", 0) == 0) {
                current_ego = EgoState();
                current_ego.frame_index = -1;
            } else if (line.rfind("}", 0) == 0 || line.rfind("},", 0) == 0) {
                if (current_ego.frame_index != -1) {
                    ego_states.push_back(current_ego);
                }
            } else if (line.rfind("\"frame_index\":", 0) == 0) {
                current_ego.frame_index = std::stoi(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"timestamp\":", 0) == 0) {
                current_ego.timestamp = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"x\":", 0) == 0) {
                current_ego.x = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"y\":", 0) == 0) {
                current_ego.y = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"z\":", 0) == 0) {
                current_ego.z = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"roll\":", 0) == 0) {
                current_ego.roll = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"pitch\":", 0) == 0) {
                current_ego.pitch = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"yaw\":", 0) == 0) {
                current_ego.yaw = std::stod(line.substr(line.find(":") + 1));
            }
        } else if (in_env) {
            if (line.find("\": [") != std::string::npos && line.find("environmental_vehicles") == std::string::npos) {
                // Starting a new vehicle array
                size_t start = line.find("\"");
                size_t end = line.find("\"", start + 1);
                current_env_id = line.substr(start + 1, end - start - 1);
                current_env_pts.clear();
            } else if (line.rfind("]", 0) == 0 || line.rfind("],", 0) == 0) {
                if (!current_env_id.empty() && !current_env_pts.empty()) {
                    EnvVehicleTrajectory traj;
                    traj.id = current_env_id;
                    traj.points = current_env_pts;
                    env_trajectories.push_back(traj);
                }
                current_env_id.clear();
            } else if (line.rfind("{", 0) == 0) {
                current_env_pt = RiskPoint();
                current_env_pt.timestamp = -1.0;
            } else if (line.rfind("}", 0) == 0 || line.rfind("},", 0) == 0) {
                if (current_env_pt.timestamp != -1.0) {
                    current_env_pts.push_back(current_env_pt);
                }
            } else if (line.rfind("\"timestamp\":", 0) == 0) {
                current_env_pt.timestamp = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"x\":", 0) == 0) {
                current_env_pt.x = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"y\":", 0) == 0) {
                current_env_pt.y = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"z\":", 0) == 0) {
                current_env_pt.z = std::stod(line.substr(line.find(":") + 1));
            }
        }
    }
    file.close();

    std::cout << "Parsed " << ego_states.size() << " ego states and " << env_trajectories.size() << " env trajectories." << std::endl;
    for (const auto& env : env_trajectories) {
        std::cout << "  Vehicle " << env.id << " has " << env.points.size() << " points." << std::endl;
    }

    // 2. Reconstruct Kinematic properties
    reconstructKinematics(ego_states, env_trajectories);

    // 3. Compute Risk Grids for each frame
    std::vector<RiskGrid> risk_grids;
    double max_stab_risk = 0.0;

    for (const auto& ego : ego_states) {
        max_stab_risk = std::max(max_stab_risk, ego.stability_risk);

        RiskGrid grid;
        grid.t_s = ego.timestamp;
        grid.grid_size_x = 200;
        grid.grid_size_y = 400;
        grid.resolution_m = 0.25;
        grid.data.resize(grid.grid_size_x * grid.grid_size_y, 0.0);

        // For each environmental vehicle, find its future points and interpolate to draw a continuous path
        struct FutureTrajectoryPoint {
            double x;
            double y;
            double speed;
            double heading;
            double norm_t; // normalized progress along the remaining trajectory [0.0, 1.0]
        };
        std::vector<FutureTrajectoryPoint> active_trajectory_pts;

        for (const auto& env : env_trajectories) {
            std::vector<RiskPoint> future_pts;
            for (const auto& pt : env.points) {
                if (pt.timestamp >= ego.timestamp) {
                    future_pts.push_back(pt);
                }
            }

            if (!future_pts.empty()) {
                double t_start = future_pts.front().timestamp;
                double t_max = future_pts.back().timestamp;
                double t_span = t_max - t_start;

                for (size_t i = 0; i < future_pts.size(); ++i) {
                    if (i == future_pts.size() - 1 || t_span == 0.0) {
                        // Single point or the final point
                        FutureTrajectoryPoint ftp;
                        ftp.x = future_pts[i].x;
                        ftp.y = future_pts[i].y;
                        ftp.speed = future_pts[i].speed;
                        ftp.heading = future_pts[i].heading;
                        ftp.norm_t = (t_span > 0.0) ? (future_pts[i].timestamp - t_start) / t_span : 0.0;
                        active_trajectory_pts.push_back(ftp);
                    } else {
                        // Dense interpolation between point i and i+1
                        const auto& p1 = future_pts[i];
                        const auto& p2 = future_pts[i+1];
                        double segment_dt = p2.timestamp - p1.timestamp;
                        
                        double step = 0.05; // sample every 0.05s along actual trajectory segment
                        for (double t = p1.timestamp; t < p2.timestamp; t += step) {
                            double ratio = (t - p1.timestamp) / segment_dt;
                            FutureTrajectoryPoint ftp;
                            ftp.x = p1.x + ratio * (p2.x - p1.x);
                            ftp.y = p1.y + ratio * (p2.y - p1.y);
                            ftp.speed = p1.speed + ratio * (p2.speed - p1.speed);
                            
                            // Angle interpolation accounting for wraps
                            double diff_heading = p2.heading - p1.heading;
                            while (diff_heading < -M_PI) diff_heading += 2.0 * M_PI;
                            while (diff_heading > M_PI) diff_heading -= 2.0 * M_PI;
                            ftp.heading = p1.heading + ratio * diff_heading;
                            
                            ftp.norm_t = (t - t_start) / t_span;
                            active_trajectory_pts.push_back(ftp);
                        }
                    }
                }
            }
        }

        // Compute risk at each grid point in 200x400 high-resolution BEV grid
        for (int r = 0; r < grid.grid_size_y; ++r) {
            for (int c = 0; c < grid.grid_size_x; ++c) {
                // Global coordinates of this grid cell relative to ego position
                // Centered laterally (c - 100), longitudinal offset centered at 40 (covers -10m to +90m)
                double dx_cell = (c - 100) * grid.resolution_m;
                double dy_cell = (r - 40) * grid.resolution_m;

                double X_global = ego.x + dx_cell;
                double Y_global = ego.y + dy_cell;

                // 1. Kinetic energy field calculation with spatiotemporal decay
                double max_Ek = 0.0;
                for (const auto& ftp : active_trajectory_pts) {
                    double rx = X_global - ftp.x;
                    double ry = Y_global - ftp.y;

                    // Rotate coordinates into vehicle local frame
                    double x_veh = rx * std::cos(ftp.heading) + ry * std::sin(ftp.heading);
                    double y_veh = -rx * std::sin(ftp.heading) + ry * std::cos(ftp.heading);

                    // Spatial decay bounds
                    double sigma_front = 1.8 * (1.0 + 0.15 * ftp.speed);
                    double sigma_rear = 1.0;
                    double sigma_x = (x_veh >= 0.0) ? sigma_front : sigma_rear;
                    double sigma_y = 1.0;

                    // Compute decaying amplitude: vehicle current position is red (high risk 0.9),
                    // future points fade out exponentially as progress along the path (norm_t) increases.
                    double base_amplitude = 0.9;
                    double amplitude = base_amplitude * std::exp(-1.5 * ftp.norm_t);

                    double Ek = amplitude * std::exp(- (x_veh * x_veh) / (2.0 * sigma_x * sigma_x) 
                                                     - (y_veh * y_veh) / (2.0 * sigma_y * sigma_y));
                    
                    max_Ek = std::max(max_Ek, Ek);
                }

                // Representing only the kinetic field of traffic participants for now
                double R_total = max_Ek;
                
                // Write grid cell
                grid.data[r * grid.grid_size_x + c] = R_total;
            }
        }

        risk_grids.push_back(grid);
    }

    // 4. Save final output frame_result.json via a temp file
    std::string tmp_filepath = output_risk_json + ".tmp";
    std::remove(tmp_filepath.c_str());
    std::ofstream out(tmp_filepath);
    if (!out.is_open()) {
        std::cerr << "Failed to open output risk JSON file: " << tmp_filepath << std::endl;
        return false;
    }

    out << std::setprecision(6);
    out << "{\n";
    out << "  \"scenario_id\": \"cosmos_wam_scenario\",\n";
    out << "  \"prediction_seconds\": 3.0,\n";
    out << "  \"stability_warning\": {\n";
    out << "    \"max_stability_risk\": " << max_stab_risk << ",\n";
    out << "    \"status\": \"" << (max_stab_risk < 0.3 ? "SAFE" : (max_stab_risk < 0.7 ? "WARNING_ACCEL_LIMIT" : "CRITICAL_SKID_RISK")) << "\"\n";
    out << "  },\n";
    out << "  \"risk_grids\": [\n";

    for (size_t g_idx = 0; g_idx < risk_grids.size(); ++g_idx) {
        const auto& grid = risk_grids[g_idx];
        out << "    {\n";
        out << "      \"t_s\": " << grid.t_s << ",\n";
        out << "      \"grid_size\": [" << grid.grid_size_x << ", " << grid.grid_size_y << "],\n";
        out << "      \"resolution_m\": " << grid.resolution_m << ",\n";
        out << "      \"data\": [\n";
        
        // Output row by row to keep formatting compact
        for (int r = 0; r < grid.grid_size_y; ++r) {
            out << "        ";
            for (int c = 0; c < grid.grid_size_x; ++c) {
                out << grid.data[r * grid.grid_size_x + c];
                if (r == grid.grid_size_y - 1 && c == grid.grid_size_x - 1) {
                    out << "\n";
                } else {
                    out << ", ";
                }
            }
            if (r != grid.grid_size_y - 1) {
                out << "\n";
            }
        }
        out << "      ]\n";
        out << "    }" << (g_idx == risk_grids.size() - 1 ? "" : ",") << "\n";
    }

    out << "  ]\n";
    out << "}\n";
    out.close();

    std::remove(output_risk_json.c_str());
    if (std::rename(tmp_filepath.c_str(), output_risk_json.c_str()) != 0) {
        std::cerr << "Failed to rename temp risk file to: " << output_risk_json << std::endl;
        return false;
    }

    std::cout << "Successfully saved dynamic risk map to: " << output_risk_json << std::endl;
    return true;
}

} // namespace cosmos_wam
