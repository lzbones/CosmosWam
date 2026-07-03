# Module 5: Target Tracker (多目标跟踪模块)

The `target_tracker` module is a high-performance, pure C++ target tracking algorithm. It associates discrete 2D bounding boxes across frames into continuous trajectories using Hungarian IoU matching and filters out the ego vehicle's hood.

## 📌 Core Purpose / 核心作用
将云端下载的离散逐帧 2D 目标检测框，通过匈牙利算法与 IoU 交并比结合进行帧间多目标关联跟踪，输出带有唯一物体 ID 的连续轨迹，并过滤自车引擎盖（车头）的噪点检测。

## ⛓️ Core Interfaces / 核心接口
```cpp
namespace cosmos_wam {

struct Detection {
    std::string label;
    std::array<double, 4> box; // [xmin, ymin, xmax, ymax]
};

struct DetectionFrame {
    int frame_index;
    double t_s;
    std::vector<Detection> detections;
};

struct TrackedObject {
    int id;
    std::string label;
    std::vector<std::pair<double, std::array<double, 4>>> path; // time -> box
};

class TargetTracker {
public:
    TargetTracker();
    ~TargetTracker() = default;

    // 解析本地 Bbox JSON 文件
    bool parseDetections(const std::string& filepath, std::vector<DetectionFrame>& frames);

    // 运行跟踪关联核心算法
    std::vector<TrackedObject> trackObjects(const std::vector<DetectionFrame>& frames);
};

}
```

## ⚙️ Implementation & Algorithms / 实现原理与细节
1. **匈牙利算法与 IoU 匹配 (Hungarian IoU Matching)**：
   * 计算当前帧中各个检测框与上一帧所有活跃 Tracking 目标预测位置的 **IoU (交并比)**。
   * 以 `1.0 - IoU` 构建代价矩阵（Cost Matrix）。
   * 使用高效的**匈牙利算法 (Hungarian Algorithm)** 进行最优二分图匹配（Bipartite Matching）。
   * 设置 IoU 匹配阈值门限（如 `IoU > 0.3`），未成功匹配的检测框将作为新目标初始化，长时间丢失匹配的追踪目标从活跃列表中剪枝销毁。
2. **自车车头 ROI 过滤 (Ego Hood Masking)**：
   * 自车的引擎盖常被误检测为周围车辆，产生持久性的遮挡噪点。
   * Tracker 引入了坐标 ROI 掩码：若检测框处于图像底部区域（`ymin > 300.0` 且宽度 `width > 350.0`），则视作自车车头干扰框并直接抛弃。
3. **时域滑动窗口平滑 (Temporal Bbox Smoothing)**：
   * 目标检测框在时域上常由于遮挡或噪点出现大小突变。
   * Tracker 采用时域滑动均值窗口，对同一追踪轨迹 ID 的 `[xmin, ymin, xmax, ymax]` 坐标变化进行低通滤波平滑处理。
