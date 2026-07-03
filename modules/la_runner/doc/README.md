# Module 3: LA Runner (目标状态检测模块)

The `la_runner` module triggers the target detection model script on the remote GPU server to extract traffic participant bounding boxes and coordinate info from the forecast video.

## 📌 Core Purpose / 核心作用
在远端 GPU 服务器上运行 NVIDIA LocateAnything 模型，对 Module 2 生成的视频进行逐帧检测，输出目标障碍物的 2D 边界框 (Bounding Box)、类别以及自车（Ego）的位置和加速度仿真轨迹数据。

## ⛓️ Core Interfaces / 核心接口
```cpp
namespace cosmos_wam {

class LARunner {
public:
    explicit LARunner(std::shared_ptr<RemoteExecutor> executor);
    ~LARunner() = default;

    // 触发远程目标检测模型，返回结果 JSON 文件在服务端的存放路径
    bool runDetection(const std::string& script_path, std::string& remote_json_output);
};

}
```

## ⚙️ Implementation & Algorithms / 实现原理与细节
1. **任务链式监听触发**：
   在远程脚本中监听到 `OutputVideo.mp4` 完成生成后，通过命令行启动 LocateAnything 模型程序。
2. **多模态结果输出**：
   模型处理完毕后，会在远端生成三个核心输出文件：
   * `OutputVideo_detected.mp4`：带有检测结果叠加框的可视化视频。
   * `OutputVideo_detected.json`：包含每帧图像中各个目标的 2D 框位置（`[xmin, ymin, xmax, ymax]`）及物体类别的 JSON 数据文件。
   * `OutputVideo_ego_trajectory.json`：存储自车在世界坐标系/局部坐标系下的虚拟行驶轨迹。
3. **输出存在性硬校验**：
   `la_runner` 并不直接进行下载操作，而是通过 `test -f` 校验远程 JSON 检测文件是否真正落盘，从而为 Module 4 下一阶段的拉取做好准备。
