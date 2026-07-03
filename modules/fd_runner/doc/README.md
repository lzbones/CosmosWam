# Module 2: FD Runner (正向推演模块)

The `fd_runner` module triggers the remote world model script to predict and generate future driving scene videos. It also downloads the raw forecast video to the local workspace directory.

## 📌 Core Purpose / 核心作用
在云端 GPU 服务器上触发基于世界模型（Cosmos-3）的前向动力学（Forward Dynamics）预测，生成未来指定时间跨度内的场景预测视频，并在生成后自动将原始视频同步拉取到本地工作目录。

## ⛓️ Core Interfaces / 核心接口
```cpp
namespace cosmos_wam {

class FDRunner {
public:
    explicit FDRunner(std::shared_ptr<RemoteExecutor> executor);
    ~FDRunner() = default;

    // 触发远程 Cosmos-3 推演，并校验/下载生成视频
    bool runForwardDynamics(const std::string& script_path, 
                            float duration, 
                            int fps, 
                            float initial_velocity, 
                            std::string& remote_video_output);
};

}
```

## ⚙️ Implementation & Algorithms / 实现原理与细节
1. **远程清理与初始化**：
   在执行前清理上一次运行产生的旧文件（如 `OutputVideo.mp4` 和 `.tmp` 临时文件），防止状态错乱或因重名产生读写冲突。
2. **多参数远程调用**：
   通过 SSH 管道将本地输入的仿真时长 (`duration`)、帧率 (`fps`)、自车初始车速 (`initial_velocity`) 作为参数传递给远端的 `run_cosmos.sh`，动态控制云端推理生成过程。
3. **本地同步下载**：
   远端视频生成后，利用 `RemoteExecutor` 的 SCP 复制功能将视频文件以 `.tmp` 后缀下载到本地 `./output/OutputVideo.mp4.tmp`。传输完成后重命名为 `OutputVideo.mp4`，供本地直接播放和查验。
