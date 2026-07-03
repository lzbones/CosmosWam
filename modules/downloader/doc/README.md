# Module 4: Downloader (数据下载模块)

The `downloader` module downloads the remote inference results (detected video, bounding boxes JSON, and ego trajectory JSON) to the local cache directory.

## 📌 Core Purpose / 核心作用
安全稳定地将云端检测输出的 JSON 文件、自车轨迹文件以及带有检测框的视频文件下载至本地的 `./output/` 文件夹。

## ⛓️ Core Interfaces / 核心接口
```cpp
namespace cosmos_wam {

class Downloader {
public:
    explicit Downloader(std::shared_ptr<RemoteExecutor> executor);
    ~Downloader() = default;

    // 从远端并行或串行拷贝检测结果到本地目录
    bool downloadResults(const std::string& remote_video_path, 
                         const std::string& remote_json_path, 
                         const std::string& remote_ego_path,
                         const std::string& local_destination_dir);
};

}
```

## ⚙️ Implementation & Algorithms / 实现原理与细节
1. **原子性重命名协议 (Atomic Rename)**：
   为了防止本地 C++ 后续分析模块在文件尚未完全传输结束时读取导致解析错误，Downloader 采用如下机制：
   * 下载目标文件为 `OutputVideo_detected.json.tmp`。
   * 完全下载成功并断开连接后，通过 `std::rename` 将其更名为正式名 `OutputVideo_detected.json`。
2. **本地路径自动建立**：
   通过 C++ `mkdir` 自动在本地项目根目录下建立 `output` 文件夹，配置合适的访问权限（Unix 下为 `0777`），以便于后续算法模块直接使用。
3. **关键数据对齐保障**：
   下载结束后，通过本地文件流的状态查询校验，判定下载文件的大小及可读性。一旦核心的 detection json 或 ego trajectory json 缺失，直接返回失败中断流水线。
