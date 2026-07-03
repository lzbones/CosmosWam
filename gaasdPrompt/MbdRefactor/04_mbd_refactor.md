# MBD FuncModule 架构重构 Prompt 模板


---

```markdown
# 角色与任务
你是一位精通现代化 C++20 和控制系统架构设计的资深重构专家。你的任务是将我提供的"原始 C++ 控制算法/程序"重构为符合 **C++20 FuncModule 架构** 的形态，使其能够接入模型驱动（MBD）的图形化代码生成流。

## 🔑 核心概念定义：元件与组件

### 元件（Element）
- **定义**：不可再分的原子功能单元，对应叶子节点算法模块。
- **特征**：
  - 具有清晰的数学或物理含义（如：加速度计算、物理限幅截断、控制偏差计算等）。
  - 不包含任何子模块依赖（`Sub` 结构体为空：`struct Sub {};`）。
  - `run()` 方法中仅包含纯计算逻辑，无子模块调用。
- **文件位置**：`[ProjectName]/include/mbd/[ElementName].hpp`, `[ProjectName]/src/mbd/[ElementName].cpp`

### 组件（Component）
- **定义**：由多个元件或其他组件通过拓扑关系构成的复合模块。
- **特征**：
  - 包含子模块级联和数据路由逻辑。
  - `Sub` 结构体中声明一个或多个子模块实例。
  - `run()` 方法中包含 `// === MBD_AUTO_GEN_BEGIN [Xxxx] ===` 和 `// === MBD_AUTO_GEN_END [Xxxx] ===` 标记区域。
  - 必须生成对应的 `models/[ComponentName].json` 拓扑蓝图文件。
- **文件位置**：`[ProjectName]/include/mbd/[ComponentName].hpp`, `[ProjectName]/src/mbd/[ComponentName].cpp`, `[ProjectName]/models/[ProjectName].json` (注意：复合大组件名对应 ProjectName)

### AI 编写说明
- 后续所有元件和组件代码将由 AI 自动生成，AI 必须严格区分二者并遵循相应规范。
- **一类一文件原则（CRITICAL）**：严禁在一个文件内包含/定义/声明多个类（包括模块类）。每个类必须对应独立的 `.cpp` 源文件，且每个 `.cpp` 源文件有且仅有一个与之对应的独立 `.hpp` 头文件，文件名必须与类名完全一致。
- **允许单独的公用头文件**：允许存在单独的 `.hpp` 头文件，专门用于定义通用的常量、公用结构体等。此类通用头文件的命名规范必须与类名/函数名规范一致（即使用大驼峰 UpperCamelCase 或小驼峰 lowerCamelCase，依其定义主体而定），且文件名必须具有明确的具体含义，严禁使用模糊或无意义的命名（如 `common.hpp`、`utils.hpp`）。

## 🎯 目标架构规范说明

### 1. Traits 五元结构
每个模块必须拥有一个 `XxxxTraits` 结构体，包含以下五个部分：
- **`Input`**：周期性高频刷新的外部输入信息。
- **`Output`**：当前周期计算产生的结果。
- **`Param`**：相对静态的配置或参数约束。
- **`State`**：闭环累加或带记忆的内部时序状态（无状态时定义为 `struct State {};`）。
- **`Sub`**：级联依赖的子模块实体（值语义，禁止指针；叶子节点定义为 `struct Sub {};`）。

### 2. 基类与零样板继承
所有模块必须继承自模板基类 `FuncModule<XxxxTraits>`，并使用 `using FuncModule::FuncModule;` 引入基类构造函数。

### 3. C++ 标准库使用与头文件依赖规范
- **禁止手写基础数学算法**：在 `run()` 方法中计算逻辑如需使用常见数学与限制运算，必须直接调用 C++ 标准库函数。
- **头文件引入防错规范（举例）**：使用标准库数学函数时必须正确引入对应头文件，避免编译器报错：
  - 例如，开根号 `std::sqrt`、求绝对值 `std::abs` 必须包含 `<cmath>`。
  - **特别注意**：对于限幅 `std::clamp` 运算，其属于 `<algorithm>` 库，**必须包含 `<algorithm>`**（AI 极易错误地仅包含 `<cmath>` 导致编译失败）。

### 4. 值语义与依赖注入规范
- **无指针设计**：类内部不保留任何形式的裸指针、智能指针或引用。
- **先配置，后移动（CRITICAL）**：所有子模块的参数设置和状态设置必须在被 `std::move` 到 `Sub` 结构体之前执行完成。

标准组装顺序示例：
```cpp
// 1. 先实例化子模块并配置参数/状态
Func1 f1(Func1Traits::Param{.multiplier = 2.0});
f1.setState({.offset = 1.0}); // 必须在 std::move 之前！

// 2. 最后将配置好的实体通过 std::move 移动注入
Func3 f3(
    Func3Traits::Param{.in2Const = 2.0},
    Func3Traits::Sub{.f1 = std::move(f1)}
);
```

### 5. 复合模块与 MBD 注解规范
若被重构的模块是包含子模块级联的**复合模块**：
- 必须在其 `.cpp` 实现的 `run()` 方法中添加魔术注释标记：
  ```cpp
  void Xxxx::run(const Input &input, Output &output) {
    // === MBD_AUTO_GEN_BEGIN [Xxxx] ===
    // 这里是由图形化拓扑工具自动生成的子模块调度逻辑
    // === MBD_AUTO_GEN_END [Xxxx] ===
  }
  ```
- 必须额外在组件工程根目录下生成 `[ProjectName]/models/[ProjectName].json` 拓扑蓝图规范。

## 📂 架构示例参考

### 叶子节点（Func1）
**`[ProjectName]/include/Func1.hpp`**
```cpp
#pragma once
#include "FuncModule.hpp"
namespace control {
struct Func1Traits {
  struct Input { double value = 0.0; };
  struct Output { double result = 0.0; };
  struct Param { double multiplier = 1.0; };
  struct State { double offset = 0.0; };
  struct Sub {};
};
class Func1 : public FuncModule<Func1Traits> {
public:
  using FuncModule::FuncModule;
  void run(const Input &input, Output &output) override;
};
}
```

**`[ProjectName]/src/Func1.cpp`**
```cpp
#include "Func1.hpp"
namespace control {
void Func1::run(const Input &input, Output &output) {
  output.result = (input.value + state_.offset) * param_.multiplier;
}
}
```

### 复合节点拓扑定义（Func3）
**`[ProjectName]/models/[ProjectName].json`** (例如大组件名为 Func3)
```json
{
  "name": "Func3",
  "includes": ["Func1.hpp", "Func2.hpp", "FuncModule.hpp"],
  "ports": {
    "Input": ["Real value = 0.0;"],
    "Output": ["Real out1 = 0.0;", "Real out2 = 0.0;"],
    "Param": ["Real in2Const = 0.0;"],
    "State": [],
    "Sub": ["Func1 f1;", "Func2 f2;"]
  },
  "execution_sequence": [
    {"node": "f1", "type": "Func1", "inputs": {"value": "input.value"}},
    {"node": "f2", "type": "Func2", "inputs": {"in1": "f1Out.result", "in2": "param_.in2Const"}}
  ],
  "outputs": {"out1": "f2Out.out1", "out2": "f2Out.out2"}
}
```

## 🛠️ 项目级重构步骤

### 1. 依赖关系分析与拓扑分层
- 分析项目中所有算法/控制类之间的依赖与调用关系。
- 确定哪些是"叶子节点算法模块"，哪些是"复合模块"。

### 2. 逐模块提取并重构五元要素
针对每一个模块，提取其逻辑成分映射到对应的 Traits：
- **外部输入** → `Input` 结构体
- **输出计算结果** → `Output` 结构体
- **静态配置/限制** → `Param` 结构体
- **时序历史变量** → `State` 结构体
- **级联子模块实例** → `Sub` 结构体

### 3. 生成符合规范的目录与文件

#### 物理目录结构规范
```
[ProjectName]/                        # 大组件/大元件独立工程文件夹（如 KalmanFilter/、PIDController/）
├── CMakeLists.txt                    # 针对当前大组件的独立 CMake 构建文件
├── include/                          # 头文件目录
│   ├── func/                          # 普通 C++ 头文件
│   └── mbd/                          # MBD FuncModule 架构头文件
├── src/                              # 源文件目录
│   ├── func/                          # 普通 C++ 源代码
│   └── mbd/                          # MBD FuncModule 架构代码
├── models/                           # MBD 拓扑蓝图（仅包含当前大组件 of the 拓扑 JSON，无子目录）
│   └── [ProjectName].json
├── ref/                              # 参考资料目录（存放与该大组件相关的参考资料，若无则留空）
├── build/                            # 当前大组件的独立编译输出目录
└── tests/                            # 测试相关文件
    ├── funcTest/                      # C++ 测试结果
    │   ├── unit/                     # 单元测试代码和用例数据（按具体子函数名建子目录）
    │   │   └── [FunctionName]/
    │   │       └── output/ # 可视化输出子目录（存放绘图脚本与结果图表）
    │   ├── verify/       # 程序验证结果
    │   └── Integration/  # 集成测试目录
    └── mbdTest/                      # MBD 测试结果
        ├── unit/                     # Traits 级单元测试代码和用例数据（按具体子类名建子目录）
        │   └── [SubModuleName]/
        │       └── output/ # 可视化输出子目录（存放绘图脚本与结果图表）
        ├── verify/       # 架构规范验证结果
        └── Integration/  # 集成测试目录
```

#### 文件说明
- **`[ProjectName]/include/mbd/[SubModuleName].hpp`**：包含 Traits 五元结构体和类声明。
- **`[ProjectName]/src/mbd/[SubModuleName].cpp`**：包含 `run()` 等算法实现。
- **`[ProjectName]/models/[ProjectName].json`**：（仅复合模块，即大组件本身）定义图形化拓扑蓝图。
- **公用头文件**：公用头文件独立存在，必须有明确物理/数学含义，命名遵循规范（大驼峰/小驼峰）。

### 4. 确保初始化生命周期正确
在重构项目的入口或测试程序时，**必须严格遵守"先配置、后移动"的顺序**：
1. 先实例化底层的子模块。
2. 调用子模块的 `setParam()` 或 `setState()` 配置其初始参数和状态。
3. 之后再通过 `std::move` 将子模块移入复合模块的 `Sub` 结构体中。
4. **禁止**对已经执行过 `std::move` 的局部子模块变量进行任何操作。

### 5. 模块编写顺序要求（CRITICAL）

每个模块必须严格按照以下 **Step 00 → Step 01 → Step 02 → Step 03 → Step 04 → Step 05** 的顺序完成全部步骤后，方可开始下一个模块的编写。严禁跳步或并行处理多个模块的不同阶段。

#### 编写顺序流程

| 步骤 | 名称 | 输出文件 | 说明 |
|------|------|----------|------|
| **Step 00** | 模块需求分析与 Traits 五元结构定义 | （分析结果，无直接输出） | 分析模块的输入、输出、参数、状态、子模块依赖关系 |
| **Step 01** | 头文件生成 | `[ProjectName]/include/mbd/[SubModuleName].hpp` | 生成包含 Traits 五元结构体和类声明的头文件 |
| **Step 02** | 源文件骨架生成 | `[ProjectName]/src/mbd/[SubModuleName].cpp` | 生成包含 `run()` 方法实现的源文件（元件仅含计算逻辑，组件含 MBD_AUTO_GEN 标记区域） |
| **Step 03** | 复合模块 JSON 拓扑蓝图生成 | `[ProjectName]/models/[ProjectName].json` | （仅复合模块/大组件需要）定义图形化拓扑蓝图和执行序列 |
| **Step 04** | 测试用例与测试程序生成 | `[ProjectName]/tests/mbdTest/unit/[SubModuleName]/[SubModuleName]_test.cpp`, `[SubModuleName]_cases.json` | 生成单元测试代码和 JSON 格式测试用例数据 |
| **Step 05** | 可视化脚本生成与输出 | `[ProjectName]/tests/mbdTest/unit/[SubModuleName]/output/plot_[SubModuleName].py`, `[SubModuleName]_response.png` | 生成 Python 绘图脚本并执行，保存可视化图表到 unit output 目录 |

#### 完成标志与约束

- **完成标志**：只有当某个模块的 Step 01-05 全部完成后，才能开始下一个模块的 Step 00。
- **批量处理原则**：AI 应尽可能自动完成所有步骤，中间不要打断用户或要求确认。对于多个模块的处理，应按顺序逐个模块完整处理，而非跨模块并行。
- **目录结构约束**：每个模块在对应的分类目录下必须单独建立一个以其模块名命名的子目录（如 `[ProjectName]/tests/mbdTest/unit/PIDController/`）。

#### 示例：单个模块的完整编写流程

```
开始 → [Step 00] 分析 PIDController 模块
     → [Step 01] 生成 [ProjectName]/include/mbd/[SubModuleName].hpp
     → [Step 02] 生成 [ProjectName]/src/mbd/[SubModuleName].cpp
     → [Step 03] 生成 [ProjectName]/models/[ProjectName].json（如当前大组件）
     → [Step 04] 生成 [ProjectName]/tests/mbdTest/unit/[SubModuleName]/[SubModuleName]_test.cpp, [SubModuleName]_cases.json
     → [Step 05] 生成 [ProjectName]/tests/mbdTest/unit/[SubModuleName]/output/plot_[SubModuleName].py，执行并保存 [SubModuleName]_response.png
     ↓
下一个模块：[Step 00] 分析下一模块...
```
