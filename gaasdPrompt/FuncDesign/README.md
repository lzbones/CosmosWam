# FuncDesign - 设计文档生成 Prompt

本文件夹包含 LaTeX 格式函数设计文档生成的 Prompt 模板。

## 📁 文件结构

```
FuncDesign/
├── README.md                 # 本说明文件
└── 03_design_doc_gen.md      # LaTeX 格式函数设计文档生成
```

---

## 03_design_doc_gen.md - LaTeX 格式函数设计文档生成

### 核心任务
根据 C/C++ 源代码自动生成符合企业规范的 LaTeX 格式函数设计文档，并编译为 PDF。

### 执行步骤

| 步骤 | 任务 | 输出 |
|------|------|------|
| Step 0 | 用户信息与项目参数收集 | 用户信息汇总表格 |
| Step 1 | 代码理解与分析 | 代码分析摘要（300 字以内） |
| Step 2 | 功能需求提炼 | 需求描述段落（200 字以内） |
| Step 3 | 算法流程设计 | 流程步骤列表 + TikZ 流程图代码框架 |
| Step 4 | 接口与架构设计 | 接口定义 + 3 个 TikZ 图形代码 |
| Step 5 | 测试用例设计 | 测试用例表（正常 + 异常）+ 符号含义对照表 |
| Step 6 | LaTeX 文档生成 | 完整的 .tex 源文件 |
| Step 7 | PDF 编译与交付 | PDF 文件路径 + 编译成功确认 |

### 运行模式
- **交互式 (Step-by-step)**: 严格按顺序执行，每步需用户确认
- **自动独立运行 (Autonomous)**: 一口气完成所有步骤（默认）

### 文档模板特点
- 使用自定义中文文档模板
- 包含 TikZ 流程图、模块图、架构图
- 测试用例表分正常场景和异常场景
- 自动生成封面、目录、页眉页脚

### 输出产物
```
[ProjectName]/doc/
└── [根据源文件路径映射的子目录]/
    ├── [FunctionName].tex    # LaTeX 源文件
    └── [FunctionName].pdf    # 编译后的 PDF 文档
```

### 环境要求
- **LaTeX**: 需要 xelatex 和 TeX Live/MacTeX 环境
- **Python**: 使用 `~/.ai-env` 虚拟环境运行编译脚本

---

## 🔗 相关链接

- **主入口**: `../00_gaasdPrompt.md` - 全流程编排 Prompt（总调度入口）
- **上一步**: `../FuncCoding/` - C++ 代码改写与测试
- **下一步**: `../MbdRefactor/04_mbd_refactor.md` - MBD 架构重构
