# FuncCoding - C++ 代码改写与测试 Prompt

本文件夹包含 C++ 面向过程代码改写和测试生成的 Prompt 模板。

## 📁 文件结构

```
FuncCoding/
├── README.md                 # 本说明文件
├── 01_func_coding.md          # C++ 面向过程代码改写规范
└── 02_func_testing.md         # C++ 测试生成与验证规范
```

## 📋 执行顺序

**Step 01 → Step 02** - 必须先执行代码改写，再执行测试生成

---

## 01_func_coding.md - C++ 面向过程代码改写规范

### 核心原则
- **面向过程风格**: 不使用类（class），采用纯面向过程 C 风格
- **One Function Per File**: 每个源文件仅包含一个函数定义
- **SSA 静态单赋值**: 局部变量使用 `const` 且声明时初始化
- **单一出口原则**: 函数仅在末尾有一个 `return`

### 主要规范
| 类别 | 规范要求 |
|------|----------|
| 命名 - 函数/变量 | 小驼峰 `lowerCamelCase` |
| 命名 - 结构体 | 大驼峰 `UpperCamelCase` |
| 命名 - 命名空间 | 小写下划线 `lowercase_with_underscores` |
| 命名 - 宏/常量 | 大写下划线 `UPPERCASE_WITH_UNDERSCORES` |
| 注释语言 | 中文注释 |
| 缩进 | 2 空格 |
| 头文件保护 | `#pragma once` |

### 输出产物
- 改写后的 C++ 源代码：`[ProjectName]/src/func/[FunctionName].cpp`
- 头文件：`[ProjectName]/include/func/[FunctionName].hpp`

---

## 02_func_testing.md - C++ 测试生成与验证规范

### 核心任务
为已按照"面向过程规范"改写的代码库生成完整的测试体系。

### 执行流程
1. **程序验证 (verify/)**: 验证改写代码是否符合 01_func_coding.md 的规范要求
2. **单元测试 (unit/)**: 只进行模块的单元测试
3. **可视化输出 (output/)**: 测试结果的画图程序及其输出

### 验证检查清单
- [ ] 面向过程风格：未使用类（class）
- [ ] One Function Per File：每文件仅一个函数
- [ ] SSA: 局部变量使用 `const`
- [ ] 单一出口原则：仅末尾一个 `return`
- [ ] 命名规范正确
- [ ] 头文件保护使用 `#pragma once`

### 输出产物
```
[ProjectName]/tests/funcTest/
├── unit/                   # 单元测试代码和用例数据
│   ├── [Function]_test.cpp
│   └── [Function]_cases.json
├── verify/                 # 程序验证结果
│   ├── [Function]_verify.txt
│   └── coding_standard_check.txt
└── output/                 # 可视化输出和画图程序
    ├── plot_[Function].py
    └── [Function]_plot.png

源代码位置：
- [ProjectName]/src/func/[FunctionName].cpp      # 源文件
- [ProjectName]/include/func/[FunctionName].hpp  # 头文件
```

---

## 🔗 相关链接

- **主入口**: `../00_gaasdPrompt.md` - 全流程编排 Prompt
- **下一步**: `../FuncDesign/03_design_doc_gen.md` - 设计文档生成