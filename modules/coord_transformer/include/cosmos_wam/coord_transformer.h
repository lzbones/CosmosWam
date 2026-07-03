#ifndef COSMOS_WAM_COORD_TRANSFORMER_H
#define COSMOS_WAM_COORD_TRANSFORMER_H

#include "cosmos_wam/target_tracker.h"
#include <string>
#include <vector>
#include <map>

namespace cosmos_wam {

struct ReferencePoint {
    double latitude;
    double longitude;
    double altitude;
};

struct EgoPose {
    int frame_index;
    double timestamp;
    double x;
    double y;
    double z;
    double latitude;
    double longitude;
    double altitude;
    double roll;
    double pitch;
    double yaw;
};

struct EnvVehiclePoint {
    double timestamp;
    double x;
    double y;
    double z;
    double latitude;
    double longitude;
    double altitude;
};

class CoordTransformer {
public:
    CoordTransformer();
    ~CoordTransformer() = default;

    // Parses the ego vehicle's trajectory output JSON file.
    bool parseEgoTrajectory(const std::string& filepath, ReferencePoint& ref_point, std::vector<EgoPose>& ego_trajectory);

    // Transforms tracked 2D bounding boxes to 3D local ENU and WGS-84 geodesic coordinates,
    // and saves the final integrated trajectory file.
    bool transformAndSave(const std::vector<Track>& tracks, 
                          const std::vector<EgoPose>& ego_trajectory, 
                          const ReferencePoint& ref_point, 
                          const std::string& output_filepath);

private:
    void enuToGeodetic(double x, double y, double z, 
                       const ReferencePoint& ref, 
                       double& lat, double& lon, double& alt) const;

    // Helper to rotate vector by Euler angles (roll, pitch, yaw in degrees) to ENU frame
    void rotateVectorToENU(double X_cam, double Z_cam, double H_cam, 
                           double roll, double pitch, double yaw, 
                           double& dx, double& dy, double& dz) const;
};

} // namespace cosmos_wam

#endif // COSMOS_WAM_COORD_TRANSFORMER_H
