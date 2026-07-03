# Module: Remote Executor (远程执行器)

The `remote_executor` module provides an abstraction layer for interacting with a remote GPU server over SSH and transferring files over SCP. It uses the local `expect` command line utility to automate interactive password authentication.

## 📌 Core Purpose / 核心作用
在没有密钥配对的情况下，通过本地 `expect` 脚本自动完成 SSH 登录密码输入，在远端 GPU 服务器上执行命令，并与本地进行 SCP 文件上传与下载。

## ⛓️ Core Interfaces / 核心接口
```cpp
namespace cosmos_wam {

class RemoteExecutor {
public:
    RemoteExecutor(std::string host, std::string user, std::string password);
    ~RemoteExecutor() = default;

    // 执行远程终端命令并捕获输出
    bool executeCommand(const std::string& cmd, std::string& output) const;

    // 从远端 SCP 拷贝文件到本地
    bool copyFromRemote(const std::string& remote_path, const std::string& local_path) const;

    // 拷贝本地文件到远端服务器
    bool copyToRemote(const std::string& local_path, const std::string& remote_path) const;

    // 检测远端文件是否存在
    bool fileExists(const std::string& remote_path) const;
};

}
```

## ⚙️ Implementation & Algorithms / 实现原理与细节
1. **expect 脚本自动应答**：
   通过 `expect -c` 构建内联脚本，使用 `spawn` 启动原生的 `ssh`/`scp` 命令。在检测到 `*password:` 提示字样时，自动发送明文密码并回车。
2. **退出状态捕获**：
   使用 `catch wait result` 捕获子进程的实际退出状态码，并通过 `exit [lindex $result 3]` 将其返回给 C++ 程序，实现高精度的状态检测（例如文件校验中的 `test -f` 退出码）。
3. **系统管道重定向**：
   使用 `popen` 创建双向进程管道，将 `stderr` 重定向到 `stdout`，实时读取执行日志并打印到客户端终端。
