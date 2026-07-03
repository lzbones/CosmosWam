#include "cosmos_wam/target_tracker.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <cmath>

namespace cosmos_wam {

TargetTracker::TargetTracker() {}

double TargetTracker::computeIoU(const std::array<double, 4>& boxA, const std::array<double, 4>& boxB) const {
    double xA = std::max(boxA[0], boxB[0]);
    double yA = std::max(boxA[1], boxB[1]);
    double xB = std::min(boxA[2], boxB[2]);
    double yB = std::min(boxA[3], boxB[3]);

    double interArea = std::max(0.0, xB - xA) * std::max(0.0, yB - yA);
    double boxAArea = (boxA[2] - boxA[0]) * (boxA[3] - boxA[1]);
    double boxBArea = (boxB[2] - boxB[0]) * (boxB[3] - boxB[1]);

    return interArea / (boxAArea + boxBArea - interArea + 1e-6);
}

bool TargetTracker::parseDetections(const std::string& filepath, std::vector<DetectionFrame>& frames) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open detections file: " << filepath << std::endl;
        return false;
    }

    std::string line;
    DetectionFrame current_frame;
    current_frame.frame_index = -1;
    bool in_detections = false;
    bool in_box = false;
    Detection current_det;
    std::vector<double> box_vals;

    while (std::getline(file, line)) {
        // Trim leading and trailing whitespace
        line.erase(0, line.find_first_not_of(" \t\r\n"));
        line.erase(line.find_last_not_of(" \t\r\n") + 1);

        if (line.rfind("\"frame_index\":", 0) == 0) {
            if (current_frame.frame_index != -1) {
                frames.push_back(current_frame);
            }
            current_frame = DetectionFrame();
            current_frame.frame_index = std::stoi(line.substr(line.find(":") + 1));
        } else if (line.rfind("\"t_s\":", 0) == 0) {
            current_frame.t_s = std::stod(line.substr(line.find(":") + 1));
        } else if (line.rfind("\"detections\": [", 0) == 0) {
            in_detections = true;
        } else if (in_detections && (line.rfind("}", 0) == 0 || line.rfind("},", 0) == 0)) {
            if (!current_det.label.empty() && box_vals.size() == 4) {
                double xmin = box_vals[0];
                double ymin = box_vals[1];
                double xmax = box_vals[2];
                double ymax = box_vals[3];
                
                double width = xmax - xmin;
                // ROI Filter: if the bounding box spans the bottom and is extremely wide, it's the ego hood.
                if (ymin > 300.0 && width > 350.0) {
                    std::cout << ">>> [ROI Filter] Discarded ego hood detection: " << current_det.label 
                              << " [" << xmin << ", " << ymin << ", " << xmax << ", " << ymax << "]" << std::endl;
                } else {
                    current_det.box = {xmin, ymin, xmax, ymax};
                    current_frame.detections.push_back(current_det);
                }
            }
            current_det = Detection();
            box_vals.clear();
        } else if (in_detections && !in_box && (line.rfind("]", 0) == 0 || line.rfind("],", 0) == 0)) {
            // End of detections list for this frame
            in_detections = false;
        } else if (in_detections) {
            if (line.rfind("\"label\":", 0) == 0) {
                size_t start_quote = line.find("\"", line.find(":"));
                size_t end_quote = line.find("\"", start_quote + 1);
                current_det.label = line.substr(start_quote + 1, end_quote - start_quote - 1);
            } else if (line.rfind("\"box\": [", 0) == 0) {
                in_box = true;
            } else if (in_box) {
                if (line.rfind("]", 0) == 0 || line.rfind("],", 0) == 0) {
                    in_box = false;
                } else {
                    // Extract numeric value (remove comma if present)
                    std::string val_str = line;
                    if (val_str.back() == ',') {
                        val_str.pop_back();
                    }
                    if (!val_str.empty()) {
                        box_vals.push_back(std::stod(val_str));
                    }
                }
            }
        }
    }
    if (current_frame.frame_index != -1) {
        frames.push_back(current_frame);
    }
    return true;
}

std::vector<Track> TargetTracker::trackObjects(const std::vector<DetectionFrame>& frames) {
    std::vector<Track> active_tracks;
    std::vector<Track> finished_tracks;
    int track_counter = 100;
    const int max_age = 2; // Keep track alive for at most 2 missing frames

    for (size_t f_idx = 0; f_idx < frames.size(); ++f_idx) {
        const auto& frame = frames[f_idx];
        double t_s = frame.t_s;
        const auto& detections = frame.detections;

        // Bipartite match using greedy matching on IoU
        struct MatchCandidate {
            double iou;
            int det_idx;
            int track_idx;
        };
        std::vector<MatchCandidate> candidates;

        for (size_t d = 0; d < detections.size(); ++d) {
            for (size_t t = 0; t < active_tracks.size(); ++t) {
                if (detections[d].label == active_tracks[t].label) {
                    double iou = computeIoU(detections[d].box, active_tracks[t].last_box);
                    if (iou > 0.1) {
                        candidates.push_back({iou, (int)d, (int)t});
                    }
                }
            }
        }

        // Sort candidates in descending IoU
        std::sort(candidates.begin(), candidates.end(), [](const MatchCandidate& a, const MatchCandidate& b) {
            return a.iou > b.iou;
        });

        std::vector<bool> matched_dets(detections.size(), false);
        std::vector<bool> matched_tracks(active_tracks.size(), false);

        for (const auto& cand : candidates) {
            if (!matched_dets[cand.det_idx] && !matched_tracks[cand.track_idx]) {
                matched_dets[cand.det_idx] = true;
                matched_tracks[cand.track_idx] = true;

                auto& track = active_tracks[cand.track_idx];
                track.last_box = detections[cand.det_idx].box;
                track.last_frame = (int)f_idx;
                track.trajectory.push_back({t_s, detections[cand.det_idx].box});
            }
        }

        // Handle unmatched active tracks and deactivate dead ones
        std::vector<Track> remaining_tracks;
        for (size_t t = 0; t < active_tracks.size(); ++t) {
            if (matched_tracks[t] || ((int)f_idx - active_tracks[t].last_frame) <= max_age) {
                remaining_tracks.push_back(active_tracks[t]);
            } else {
                finished_tracks.push_back(active_tracks[t]);
            }
        }
        active_tracks = std::move(remaining_tracks);

        // Spawn new tracks
        for (size_t d = 0; d < detections.size(); ++d) {
            if (!matched_dets[d]) {
                track_counter++;
                Track new_track;
                new_track.id = detections[d].label + "_" + std::to_string(track_counter);
                new_track.label = detections[d].label;
                new_track.last_box = detections[d].box;
                new_track.last_frame = (int)f_idx;
                new_track.trajectory.push_back({t_s, detections[d].box});
                active_tracks.push_back(new_track);
            }
        }
    }

    finished_tracks.insert(finished_tracks.end(), active_tracks.begin(), active_tracks.end());

    // Filter out short tracks (noise) with less than 2 detections
    std::vector<Track> valid_tracks;
    for (auto& track : finished_tracks) {
        if (track.trajectory.size() >= 2) {
            std::sort(track.trajectory.begin(), track.trajectory.end(), [](const TrackedDetection& a, const TrackedDetection& b) {
                return a.t_s < b.t_s;
            });
            valid_tracks.push_back(track);
        }
    }

    return valid_tracks;
}

} // namespace cosmos_wam
