#ifndef COSMOS_WAM_TARGET_TRACKER_H
#define COSMOS_WAM_TARGET_TRACKER_H

#include <string>
#include <vector>
#include <array>

namespace cosmos_wam {

struct Detection {
    std::string label;
    std::array<double, 4> box; // [ymin, xmin, ymax, xmax]
};

struct TrackedDetection {
    double t_s;
    std::array<double, 4> box; // [ymin, xmin, ymax, xmax]
};

struct Track {
    std::string id;
    std::string label;
    std::array<double, 4> last_box;
    int last_frame;
    std::vector<TrackedDetection> trajectory;
};

struct DetectionFrame {
    int frame_index;
    double t_s;
    std::vector<Detection> detections;
};

class TargetTracker {
public:
    TargetTracker();
    ~TargetTracker() = default;

    // Parses the LocateAnything output JSON file.
    bool parseDetections(const std::string& filepath, std::vector<DetectionFrame>& frames);

    // Runs a greedy IoU tracker over the parsed frames to generate tracks.
    std::vector<Track> trackObjects(const std::vector<DetectionFrame>& frames);

private:
    double computeIoU(const std::array<double, 4>& boxA, const std::array<double, 4>& boxB) const;
};

} // namespace cosmos_wam

#endif // COSMOS_WAM_TARGET_TRACKER_H
