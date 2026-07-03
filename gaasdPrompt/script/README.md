# Script 工具脚本说明

本目录存放 gaasdPrompt 流程中使用的各类自动化脚本。

---

## 📁 文件列表

| 文件名 | 功能描述 |
|--------|----------|
| `compile_latex.py` | LaTeX 文档编译脚本（自动编译两次、标题修正、辅助文件清理） |
| `run_pipeline.py` | 全流程自动化编排脚本（CMake 构建 → 测试 → 绘图 → 文档编译） |

---

## 📄 compile_latex.py - LaTeX 编译脚本

### 功能说明

该脚本用于将 LaTeX 源文件编译为 PDF，并自动完成以下任务：
1. **自动编译两次**：确保交叉引用（`\ref{}`、`\cite{}`、TikZ 标签等）正确解析
2. **标题格式修正**：自动修复 LaTeX 文件中不通顺的标题和冗余词汇
3. **辅助文件清理**：编译完成后自动删除 `.aux`、`.log`、`.toc` 等中间文件

### 使用方法

```bash
# 方式 1：直接传入 .tex 文件路径
python script/compile_latex.py [ProjectName]/doc/[FunctionName]/[FunctionName].tex

# 方式 2：传入源代码路径，自动映射到对应的 .tex 文件
python script/compile_latex.py [ProjectName]/src/func/[FunctionName].cpp

# 方式 3：指定自定义输出目录
python script/compile_latex.py [ProjectName]/doc/[FunctionName]/[FunctionName].tex /custom/output/dir
```

### 路径映射规则

脚本支持将 C/C++ 源代码路径自动映射到组件物理隔离的 `doc/` 目录下的对应 `.tex` 文件：

| 源码路径 | → | LaTeX 路径 |
|----------|---|------------|
| `[ProjectName]/src/func/[FunctionName].cpp` | → | `[ProjectName]/doc/[FunctionName]/[FunctionName].tex` |
| `[ProjectName]/src/mbd/[SubModuleName].cpp` | → | `[ProjectName]/doc/[SubModuleName]/[SubModuleName].tex` |
| `[ProjectName]/include/func/[FunctionName].hpp` | → | `[ProjectName]/doc/[FunctionName]/[FunctionName].tex` |

### 标题自动修正映射

脚本内置了以下标题格式自动修正规则：

| 原始标题格式 | → | 修正后格式 |
|-------------|---|------------|
| `以 xxx 函数设计` | → | `xxx（中文功能名）函数设计` |
| `xxx 函数的函数设计` | → | `xxx（中文功能名）函数设计` |
| `xxx 函数的程序流程图` | → | `xxx 函数程序流程图` |
| `xxx 函数的输入输出模块图` | → | `xxx 函数输入输出模块图` |
| `xxx 函数的函数架构` | → | `xxx 函数架构图` |

### 环境要求

- **Python**: 3.8+（脚本使用 Shebang `#!/Users/qingxu/.ai-env/bin/python3`）
- **LaTeX**: xelatex（需安装 TeX Live / MacTeX）
- **中文字体**: SimSun、SimHei（用于中文文档渲染）

---

## 📄 run_pipeline.py - 全流程自动化脚本

### 功能说明

该脚本用于自动化执行完整的项目验证流程，包括：
1. CMake 项目配置与构建
2. CTest 单元测试运行（C++ & MBD）
3. Python 可视化图表生成（C++ 和 MBD 分别生成）
4. LaTeX 文档编译为 PDF

### 支持的函数/模块列表

脚本默认处理以下函数模块：

| 英文名 | MBD 名 |
|--------|-------|
| add | Add |
| sub | Sub |
| mul | Mul |
| div | Div |
| abs | Abs |
| sign | Sign |
| clamp | Clamp |
| saturate | Saturate |
| sqrt | Sqrt |
| reciprocalSqrt | ReciprocalSqrt |
| pow | Pow |
| exp | Exp |
| log | Log |
| mod | Mod |
| ceil | Ceil |
| floor | Floor |
| round | Round |
| min | Min |
| max | Max |
| bias | Bias |

### 使用方法

```bash
# 在项目根目录运行
python script/run_pipeline.py
```

### 执行流程

对于每个模块，脚本按以下顺序执行：

1. **CMake Configure**: `cmake -S . -B build`
2. **CMake Build**: `cmake --build build`
3. **Run Tests**: `ctest --output-on-failure`
4. **Generate C++ Function Plot**: 运行 `[ProjectName]/tests/funcTest/unit/[name]/output/plot_[name].py`（由脚本自动定位并运行）
5. **Generate MBD Plot**: 运行 `[ProjectName]/tests/mbdTest/unit/[Name]/output/plot_[Name].py`（由脚本自动定位并运行）
6. **Compile LaTeX**: 运行 `compile_latex.py` 编译文档

### 输出示例

```
=== Starting Auto Compilation and Verification Pipeline ===

--------------------------------------------------
Processing Module: add (MBD: Add)
--------------------------------------------------
[add] Configuring CMake...
[add] Building...
[add] Running Tests...
[add] Tests Passed!
[add] Generating C++ Function Plot...
[add] Generating MBD Plot...
[add] Compiling LaTeX document...
[add] LaTeX document compiled successfully!

==================================================
               VERIFICATION SUMMARY               
==================================================
✅ add: All Steps - PASS

Final Result: ALL PASSED ✅
```

### 环境要求

- **Python**: 3.8+（脚本使用 `/Users/qingxu/.ai-env/bin/python3`）
- **CMake**: 3.16+
- **C++ Compiler**: GCC 10+ 或 Clang 12+（支持 C++20）
- **LaTeX**: xelatex

---

## 🛠️ 自定义扩展

### 添加新模块到 run_pipeline.py

在 `FUNCTIONS` 列表中添加新的函数定义：

```python
FUNCTIONS = [
    # ... existing functions ...
    {"name": "newFunc", "Name": "NewFunc"},  # 添加新条目
]
```

### 修改标题映射规则（compile_latex.py）

在 `fix_tex_titles_in_file()` 函数的 `mapping` 字典中添加新的映射：

```python
mapping = {
    # ... existing mappings ...
    "newFunction": "新功能中文名称",
}
```

---

## 📝 注意事项

1. **Python 虚拟环境**: 所有脚本默认使用 `/Users/qingxu/.ai-env` 虚拟环境，如需修改请更新 Shebang 或 `PYTHON` 变量。
2. **LaTeX 编译两次**: `compile_latex.py` 自动执行两次 xelatex 编译，确保交叉引用正确解析。
3. **路径映射**: `compile_latex.py` 支持源码路径到 doc 路径的自动映射，方便直接传入源码路径进行编译。
4. **错误处理**: `run_pipeline.py` 会在每个步骤失败时记录错误信息并在最后输出汇总报告。