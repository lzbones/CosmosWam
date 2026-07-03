# 🧩 gaasdPrompt - 全流程代码工程管线 Prompt 集合

本仓库包含一套完整的 Prompt 模板，用于指导 AI 完成从 C++ 代码改写 → 测试生成 → 设计文档 → MBD 重构 → MBD 测试的完整闭环。

---

## 📁 目录结构

```
.
├── 00_gaasdPrompt.md         # 全流程编排主入口
├── FuncCoding/                # C++ 代码改写与测试
│   ├── 01_func_coding.md      # C++ 面向过程代码改写规范
│   └── 02_func_testing.md     # C++ 测试生成与验证规范
├── FuncDesign/                # 设计文档生成
│   └── 03_design_doc_gen.md  # LaTeX 格式函数设计文档生成
├── MbdRefactor/              # MBD 重构与测试
│   ├── 04_mbd_refactor.md    # FuncModule 架构重构规范
│   └── 05_mbd_testing.md     # MBD 测试生成与验证规范
├── script/                   # 自动化编译与管线脚本
│   ├── check_funcmodule_arch.py # MBD FuncModule 架构规范静态校验脚本
│   ├── compile_latex.py      # LaTeX 自动编译、修正与清理脚本
│   ├── run_pipeline.py       # 自动化管线构建与测试验证脚本
│   └── README.md             # Script 目录说明文档
├── comparison_report.md      # 提示词模板版本修改与优化对比报告
└── README.md                 # 本说明文件
```

---

## 🚀 快速开始

1. **打开 `00_gaasdPrompt.md`** - 这是全流程编排的主入口文件
2. **勾选需要执行的步骤** - 根据需求选择 Step 01-05
3. **将内容复制给 AI 助手** - AI 会自动加载对应的 Prompt 模板并引导完成每个步骤

---

## 📋 执行流程

| 步骤 | 功能 | 输入 | 输出 |
|------|------|------|------|
| Step 01 | C++ 代码改写 | 原始 C++ 代码 | `[ProjectName]/src/func/`, `[ProjectName]/include/func/` |
| Step 02 | C++ 测试生成 | Step 01 输出的代码 | `[ProjectName]/tests/funcTest/` - 报告、用例与图表 |
| Step 03 | 设计文档生成 | Step 01 输出的代码 | `[ProjectName]/doc/` - LaTeX 与 PDF 设计文档 |
| Step 04 | MBD 架构重构 | Step 01 输出的代码 | `[ProjectName]/src/mbd/`, `[ProjectName]/include/mbd/`, `[ProjectName]/models/` |
| Step 05 | MBD 测试验证 | Step 04 输出的代码 | `[ProjectName]/tests/mbdTest/` - 报告、用例与图表 |

---

## 📂 各文件夹说明

### FuncCoding/
包含 C++ 面向过程代码改写和测试生成的 Prompt 模板。
- **01_func_coding.md**: 定义面向过程编程规范（SSA、单一出口原则等），输出到 `[ProjectName]/src/func/`, `[ProjectName]/include/func/`
- **02_func_testing.md**: 生成单元测试、验证报告和可视化脚本，输出到 `[ProjectName]/tests/funcTest/`

### FuncDesign/
包含 LaTeX 格式函数设计文档生成的 Prompt 模板。
- **03_design_doc_gen.md**: 根据源代码自动生成符合企业规范的 LaTeX 设计文档，输出到 `[ProjectName]/doc/`

### MbdRefactor/
包含 MBD FuncModule 架构重构和测试的 Prompt 模板。
- **04_mbd_refactor.md**: 将原始 C++ 代码重构为 C++20 FuncModule 架构，输出到 `[ProjectName]/src/mbd/`, `[ProjectName]/include/mbd/`, `[ProjectName]/models/`
- **05_mbd_testing.md**: 为 FuncModule 架构代码生成测试用例和验证报告，输出到 `[ProjectName]/tests/mbdTest/`

### script/
包含用于自动化开发流程、测试验证与文档编译的 Python 脚本。
- **compile_latex.py**: 自动编译 LaTeX 报告为 PDF，并清理编译产生的辅助文件
- **run_pipeline.py**: 一键式构建、测试与生成测试报告和图表，支持多模块的自动化验证
- **README.md**: Script 目录的详细使用说明

---

## 📝 输出产物说明

执行完整流程后，将生成以下产物：

```
project_root/
├── src/                          # 源代码目录（扁平化存放）
│   ├── func/                      # 普通 C++ 源代码（一函数一文件，无子目录）
│   └── mbd/                      # MBD FuncModule 架构代码（一类一文件，无子目录）
├── include/                      # 头文件目录（扁平化存放）
│   ├── func/                      # 普通 C++ 头文件（一函数一头文件，无子目录）
│   └── mbd/                      # MBD FuncModule 架构头文件（一类一头文件，无子目录）
├── models/                       # MBD 拓扑蓝图（仅 MBD 模块，无子目录）
│   └── [ModuleName].json
├── tests/                        # 测试相关文件（与 src 同级）
│   ├── funcTest/                  # C++ 测试结果
│   │   ├── unit/                 # 函数级单元测试代码和用例数据（按函数名建子目录）
│   │   │   └── [FunctionName]/
│   │   │       └── output/   # 可视化输出子目录（存放绘图脚本与结果图表）
│   │   ├── verify/               # 程序验证结果（包含 [FunctionName]_verify.txt）
│   │   └── Integration/          # 集成测试目录
│   └── mbdTest/                  # MBD 测试结果
│       ├── unit/                 # Traits 级单元测试代码和用例数据（按模块名建子目录）
│       │   └── [ModuleName]/
│       │       └── output/   # 可视化输出子目录（存放绘图脚本与结果图表）
│       ├── verify/               # 架构规范验证结果（含 [ModuleName]_verify.txt 与 architecture_check.txt）
│       └── Integration/          # 集成测试目录
├── doc/                          # 设计文档输出（按函数名建子目录）
│   └── [FunctionName]/
├── ref/                          # 参考资料目录（存放与该模块对应的参考资料，若无则留空）
├── build/                        # 编译输出目录
└── .gitignore                # Git 忽略规则文件（模板自动生成，排除 build 缓存与系统日志）
```

---

## ⚠️ 环境要求

| 工具 | 版本要求 | 用途 |
|------|----------|------|
| **LaTeX** | xelatex (TeX Live / MacTeX) | Step 03 设计文档编译 |
| **Python** | 3.8+ (`~/.ai-env` 虚拟环境) | 编译脚本、测试可视化 |
| **C++ 编译器** | GCC 10+ 或 Clang 12+ (支持 C++20) | MBD 架构代码编译 |
| **CMake** | 3.16+ | 项目构建配置 |

---

## 📖 更多信息

详细的使用说明和注意事项请参阅以下文件：

- **`00_gaasdPrompt.md`** - 全流程编排主入口，包含完整的步骤选择和执行规则
- **`script/README.md`** - Script 目录下各脚本的详细使用说明
- **`comparison_report.md`** - 提示词模块与管线的历史修改与优化对比报告（详见重要特性及 Bug 修复）

### 目录结构说明

普通 C++ 代码和 MBD 架构代码是分离存放的：

| 类型 | 源代码 | 头文件 | 测试文件 |
|------|--------|--------|----------|
| **普通 C++** | `[ProjectName]/src/func/` | `[ProjectName]/include/func/` | `[ProjectName]/tests/funcTest/` |
| **MBD 架构** | `[ProjectName]/src/mbd/` | `[ProjectName]/include/mbd/` | `[ProjectName]/tests/mbdTest/` |