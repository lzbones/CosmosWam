# Module 1: Uploader (输入上传模块)

The `uploader` module manages the transfer of local driving scene images to the remote server to serve as the initial input for video generation.

## 📌 Core Purpose / 核心作用
负责将本地待推演的场景路面图片上传至远端服务器指定的工作空间目录。

## ⛓️ Core Interfaces / 核心接口
```cpp
namespace cosmos_wam {

class Uploader {
public:
    explicit Uploader(std::shared_ptr<RemoteExecutor> executor);
    ~Uploader() = default;

    // 上传本地图片到远程路径
    bool uploadImage(const std::string& local_path, const std::string& remote_path);
};

}
```

## ⚙️ Implementation & Algorithms / 实现原理与细节
1. **原子性写入与重命名协议 (Atomic Write Protocol)**：
   为避免远程服务器上的文件监听脚本（如检测新图片并自动触发大模型的脚本）在文件还在传输时就抢先读取导致破损，Uploader 采用了原子写入逻辑：
   * 优先将图片以 `.tmp` 扩展名上传（如 `/tmp/InputImage.png.tmp`）。
   * 传输顺利完成后，通过远程 SSH 命令 `mv` 将其重命名为最终格式（`/tmp/InputImage.png`）。
   * 此操作确保了远端文件产生的瞬间即是完整可读的。
2. **连接性校验**：
   在上传前确认 `RemoteExecutor` 的有效性，若网络中断或 SCP 异常将立即终止上传并上报 Scheduler。
