#include "cosmos_wam/coord_transformer.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <iomanip>

namespace cosmos_wam {

// Standard constant for degrees/radians conversions
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

CoordTransformer::CoordTransformer() {}

void CoordTransformer::enuToGeodetic(double x, double y, double z, 
                                     const ReferencePoint& ref, 
                                     double& lat, double& lon, double& alt) const {
    double lat_rad = ref.latitude * M_PI / 180.0;
    // Earth radius approximation in meters per degree
    double m_per_deg_lat = 111132.92 - 559.82 * std::cos(2.0 * lat_rad) + 1.175 * std::cos(4.0 * lat_rad);
    double m_per_deg_lon = 111412.84 * std::cos(lat_rad) - 93.5 * std::cos(3.0 * lat_rad);

    lat = ref.latitude + (y / m_per_deg_lat);
    lon = ref.longitude + (x / m_per_deg_lon);
    alt = ref.altitude + z;
}

void CoordTransformer::rotateVectorToENU(double X_cam, double Z_cam, double H_cam, 
                                         double roll, double pitch, double yaw, 
                                         double& dx, double& dy, double& dz) const {
    double r = roll * M_PI / 180.0;
    double p = pitch * M_PI / 180.0;
    double y = yaw * M_PI / 180.0;

    // Rotation matrices
    // Rx (Roll)
    double Rx[3][3] = {
        {1.0, 0.0, 0.0},
        {0.0, std::cos(r), -std::sin(r)},
        {0.0, std::sin(r), std::cos(r)}
    };

    // Ry (Pitch)
    double Ry[3][3] = {
        {std::cos(p), 0.0, std::sin(p)},
        {0.0, 1.0, 0.0},
        {-std::sin(p), 0.0, std::cos(p)}
    };

    // Rz (Yaw)
    double Rz[3][3] = {
        {std::cos(y), -std::sin(y), 0.0},
        {std::sin(y), std::cos(y), 0.0},
        {0.0, 0.0, 1.0}
    };

    // Camera vector: P_cam = [X_cam, Z_cam, -H_cam]
    double P_cam[3] = {X_cam, Z_cam, -H_cam};

    // Tmp1 = Ry @ Rx
    double Tmp1[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            Tmp1[i][j] = 0.0;
            for (int k = 0; k < 3; ++k) {
                Tmp1[i][j] += Ry[i][k] * Rx[k][j];
            }
        }
    }

    // R = Rz @ Tmp1
    double R[3][3];
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            R[i][j] = 0.0;
            for (int k = 0; k < 3; ++k) {
                R[i][j] += Rz[i][k] * Tmp1[k][j];
            }
        }
    }

    // P_offset = R @ P_cam
    dx = R[0][0]*P_cam[0] + R[0][1]*P_cam[1] + R[0][2]*P_cam[2];
    dy = R[1][0]*P_cam[0] + R[1][1]*P_cam[1] + R[1][2]*P_cam[2];
    dz = R[2][0]*P_cam[0] + R[2][1]*P_cam[1] + R[2][2]*P_cam[2];
}

bool CoordTransformer::parseEgoTrajectory(const std::string& filepath, 
                                          ReferencePoint& ref_point, 
                                          std::vector<EgoPose>& ego_trajectory) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open ego trajectory file: " << filepath << std::endl;
        return false;
    }

    std::string line;
    bool in_ref = false;
    bool in_ego = false;
    EgoPose current_pose;
    current_pose.frame_index = -1;

    while (std::getline(file, line)) {
        // Trim whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.rfind("\"reference_point\":", 0) == 0) {
            in_ref = true;
        } else if (line.rfind("\"ego_trajectory\":", 0) == 0) {
            in_ref = false;
            in_ego = true;
        } else if (in_ref) {
            if (line.rfind("\"latitude\":", 0) == 0) {
                ref_point.latitude = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"longitude\":", 0) == 0) {
                ref_point.longitude = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"altitude\":", 0) == 0) {
                ref_point.altitude = std::stod(line.substr(line.find(":") + 1));
            }
        } else if (in_ego) {
            if (line.rfind("{", 0) == 0) {
                current_pose = EgoPose();
                current_pose.frame_index = -1;
            } else if (line.rfind("}", 0) == 0 || line.rfind("},", 0) == 0) {
                if (current_pose.frame_index != -1) {
                    ego_trajectory.push_back(current_pose);
                }
            } else if (line.rfind("\"frame_index\":", 0) == 0) {
                current_pose.frame_index = std::stoi(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"timestamp\":", 0) == 0) {
                current_pose.timestamp = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"x\":", 0) == 0) {
                current_pose.x = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"y\":", 0) == 0) {
                current_pose.y = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"z\":", 0) == 0) {
                current_pose.z = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"latitude\":", 0) == 0) {
                current_pose.latitude = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"longitude\":", 0) == 0) {
                current_pose.longitude = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"altitude\":", 0) == 0) {
                current_pose.altitude = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"roll\":", 0) == 0) {
                current_pose.roll = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"pitch\":", 0) == 0) {
                current_pose.pitch = std::stod(line.substr(line.find(":") + 1));
            } else if (line.rfind("\"yaw\":", 0) == 0) {
                current_pose.yaw = std::stod(line.substr(line.find(":") + 1));
            }
        }
    }
    return true;
}

bool CoordTransformer::transformAndSave(const std::vector<Track>& tracks, 
                                        const std::vector<EgoPose>& ego_trajectory, 
                                        const ReferencePoint& ref_point, 
                                        const std::string& output_filepath) {
    // 1. Determine image resolution dynamically
    double max_x = 640.0;
    double max_y = 480.0;
    for (const auto& track : tracks) {
        for (const auto& det : track.trajectory) {
            max_x = std::max({max_x, det.box[0], det.box[2]});
            max_y = std::max({max_y, det.box[1], det.box[3]});
        }
    }

    double W = (max_x > 640.0 || max_y <= 480.0) ? 800.0 : 640.0;
    double H = (max_x > 640.0 || max_y <= 480.0) ? 480.0 : 640.0;
    std::cout << "Coordinate Transformer inferred resolution: " << W << "x" << H << std::endl;

    double H_cam = 1.5;
    double f_x = W;
    double f_y = W;
    double c_x = W / 2.0;
    double c_y = H / 2.0;

    std::map<std::string, std::vector<EnvVehiclePoint>> environmental_vehicles;

    for (const auto& track : tracks) {
        std::vector<EnvVehiclePoint> env_pts;

        for (const auto& det : track.trajectory) {
            double t_s = det.t_s;
            double xmin = det.box[0];
            [[maybe_unused]] double ymin = det.box[1];
            double xmax = det.box[2];
            double ymax = det.box[3];

            double u_mid = (xmin + xmax) / 2.0;
            double v_bottom = ymax;

            // 2D to 3D camera projection
            double v_bottom_clipped = std::max(v_bottom, c_y + 10.0);
            double Z_cam = (f_y * H_cam) / (v_bottom_clipped - c_y);
            double X_cam = Z_cam * (u_mid - c_x) / f_x;

            // Find closest ego pose
            EgoPose ego_pose;
            bool found = false;
            double min_diff = 1e9;
            for (const auto& ep : ego_trajectory) {
                double diff = std::abs(ep.timestamp - t_s);
                if (diff < min_diff) {
                    min_diff = diff;
                    ego_pose = ep;
                    found = true;
                }
            }

            double x_ego = found ? ego_pose.x : 0.0;
            double y_ego = found ? ego_pose.y : 0.0;
            double z_ego = found ? ego_pose.z : 0.0;
            double roll = found ? ego_pose.roll : 0.0;
            double pitch = found ? ego_pose.pitch : 0.0;
            double yaw = found ? ego_pose.yaw : 0.0;

            // Rotate vector and translate to absolute ENU
            double dx = 0.0, dy = 0.0, dz = 0.0;
            rotateVectorToENU(X_cam, Z_cam, H_cam, roll, pitch, yaw, dx, dy, dz);

            double X_target = x_ego + dx;
            double Y_target = y_ego + dy;
            double Z_target = z_ego + dz;

            // ENU to Geodetic
            double lat = 0.0, lon = 0.0, alt = 0.0;
            enuToGeodetic(X_target, Y_target, Z_target, ref_point, lat, lon, alt);

            EnvVehiclePoint pt;
            pt.timestamp = t_s;
            pt.x = X_target;
            pt.y = Y_target;
            pt.z = Z_target;
            pt.latitude = lat;
            pt.longitude = lon;
            pt.altitude = alt;
            env_pts.push_back(pt);
        }

        // Apply a centered 3-point moving average filter to smooth out pixel-level detection jitter
        if (env_pts.size() >= 3) {
            std::vector<EnvVehiclePoint> smoothed = env_pts;
            for (size_t i = 0; i < env_pts.size(); ++i) {
                double sx = 0.0, sy = 0.0, sz = 0.0;
                if (i == 0) {
                    sx = (2.0 * env_pts[0].x + env_pts[1].x) / 3.0;
                    sy = (2.0 * env_pts[0].y + env_pts[1].y) / 3.0;
                    sz = (2.0 * env_pts[0].z + env_pts[1].z) / 3.0;
                } else if (i == env_pts.size() - 1) {
                    sx = (env_pts[i-1].x + 2.0 * env_pts[i].x) / 3.0;
                    sy = (env_pts[i-1].y + 2.0 * env_pts[i].y) / 3.0;
                    sz = (env_pts[i-1].z + 2.0 * env_pts[i].z) / 3.0;
                } else {
                    sx = (env_pts[i-1].x + env_pts[i].x + env_pts[i+1].x) / 3.0;
                    sy = (env_pts[i-1].y + env_pts[i].y + env_pts[i+1].y) / 3.0;
                    sz = (env_pts[i-1].z + env_pts[i].z + env_pts[i+1].z) / 3.0;
                }
                smoothed[i].x = sx;
                smoothed[i].y = sy;
                smoothed[i].z = sz;
                
                // Re-derive WGS-84 geodesic latitude/longitude from smoothed ENU offsets
                double lat = 0.0, lon = 0.0, alt = 0.0;
                enuToGeodetic(sx, sy, sz, ref_point, lat, lon, alt);
                smoothed[i].latitude = lat;
                smoothed[i].longitude = lon;
                smoothed[i].altitude = alt;
            }
            env_pts = smoothed;
        }

        if (env_pts.size() >= 2) {
            environmental_vehicles[track.id] = env_pts;
        }
    }

    // 4. Assemble final integrated JSON manually via a temp file
    std::string tmp_filepath = output_filepath + ".tmp";
    std::remove(tmp_filepath.c_str());
    std::ofstream out(tmp_filepath);
    if (!out.is_open()) {
        std::cerr << "Failed to open output trajectories JSON file: " << tmp_filepath << std::endl;
        return false;
    }

    out << std::setprecision(14);
    out << "{\n";
    out << "  \"reference_point\": {\n";
    out << "    \"latitude\": " << ref_point.latitude << ",\n";
    out << "    \"longitude\": " << ref_point.longitude << ",\n";
    out << "    \"altitude\": " << ref_point.altitude << "\n";
    out << "  },\n";

    out << "  \"ego_trajectory\": [\n";
    for (size_t i = 0; i < ego_trajectory.size(); ++i) {
        const auto& ep = ego_trajectory[i];
        out << "    {\n";
        out << "      \"frame_index\": " << ep.frame_index << ",\n";
        out << "      \"timestamp\": " << ep.timestamp << ",\n";
        out << "      \"x\": " << ep.x << ",\n";
        out << "      \"y\": " << ep.y << ",\n";
        out << "      \"z\": " << ep.z << ",\n";
        out << "      \"latitude\": " << ep.latitude << ",\n";
        out << "      \"longitude\": " << ep.longitude << ",\n";
        out << "      \"altitude\": " << ep.altitude << ",\n";
        out << "      \"roll\": " << ep.roll << ",\n";
        out << "      \"pitch\": " << ep.pitch << ",\n";
        out << "      \"yaw\": " << ep.yaw << "\n";
        out << "    }" << (i == ego_trajectory.size() - 1 ? "" : ",") << "\n";
    }
    out << "  ],\n";

    out << "  \"environmental_vehicles\": {\n";
    size_t v_count = 0;
    for (auto it = environmental_vehicles.begin(); it != environmental_vehicles.end(); ++it) {
        out << "    \"" << it->first << "\": [\n";
        const auto& pts = it->second;
        for (size_t i = 0; i < pts.size(); ++i) {
            out << "      {\n";
            out << "        \"timestamp\": " << pts[i].timestamp << ",\n";
            out << "        \"x\": " << pts[i].x << ",\n";
            out << "        \"y\": " << pts[i].y << ",\n";
            out << "        \"z\": " << pts[i].z << ",\n";
            out << "        \"latitude\": " << pts[i].latitude << ",\n";
            out << "        \"longitude\": " << pts[i].longitude << ",\n";
            out << "        \"altitude\": " << pts[i].altitude << "\n";
            out << "      }" << (i == pts.size() - 1 ? "" : ",") << "\n";
        }
        out << "    ]" << (++v_count == environmental_vehicles.size() ? "" : ",") << "\n";
    }
    out << "  },\n";

    out << "  \"environmental_obstacles\": {}\n";
    out << "}\n";
    out.close();

    std::remove(output_filepath.c_str());
    if (std::rename(tmp_filepath.c_str(), output_filepath.c_str()) != 0) {
        std::cerr << "Failed to rename temp trajectories file to: " << output_filepath << std::endl;
        return false;
    }

    std::cout << "Successfully wrote C++ integrated trajectories to: " << output_filepath << std::endl;
    return true;
}

} // namespace cosmos_wam
