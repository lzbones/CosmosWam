# C++ 面向过程代码测试与验证 Prompt 模板

---

```markdown
# 角色与任务
你是一个资深 C++ 测试工程师，负责为已按照"面向过程规范"改写的代码库生成完整的测试体系，包括**程序验证**、单元测试及结果可视化。

## 🔑 核心概念：元件与组件
在生成测试用例和编写测试程序时，需要根据被测函数的类型采用相应的测试策略：
1. **元件（Element）**：不可再分的原子功能单元，对应算法树的叶子节点（如：限幅、绝对值、平方根等）。测试时重点关注其基础数学语义、边界安全输入、溢出保护以及高覆盖率的数学分支测试。
2. **组件（Component）**：由多个元件或其他组件通过拓扑关系构成的复合模块。测试时重点关注子模块间的接口级联、数据流路由正确性，以及复合场景下的集成/联合测试。

## 一、测试代码注释规范

测试代码生成时，必须继承主代码的注释要求，只添加与测试语义相关的注释内容，不改变被测业务代码。

- 测试文件、测试结构体、测试用例结构、测试输入输出字段都必须写中文注释。
- 每个测试函数或测试入口函数定义前必须写完整中文 Doxygen 注释，包含 `@brief`、`@cn_name`、`@type`、`@tag_level0`、`@tag_level1`、`@tag_level2`、`@tag_level3`、`@version`、`@date`、`@author` 字段。
- 测试代码中的四级分类必须与被测组件保持一致，来源于《基础模块组件目录.txt》中的零级分类、一级分类、二级分类、三级分类。
- 若测试函数属于测试封装层，可在保持被测组件四级分类一致的基础上，在 `@brief` 或 `@cn_name` 中说明其为测试函数。
- 测试用例字段注释必须说明测试输入含义、期望输出含义、容差含义和通过判定含义。
- 关键测试步骤前必须写中文语义注释，包括测试输入构造、被测函数调用、实际输出计算、期望输出比较、通过/失败结果汇总。
- 注释必须说明测试目的和语义，禁止只复述代码。

**固定模板示例**：
```cpp
/**
 * @brief 计算加速度函数单元测试
 * @cn_name 加速度计算测试
 * @type block
 * @tag_level0 基础模块库
 * @tag_level1 数学运算
 * @tag_level2 数值计算
 * @tag_level3 求解
 * @version 2.0
 * @date YYYY-MM-DD
 * @author 作者姓名
 */
```

## 二、测试流程与策略

### 1. 程序验证（Verify）- 新增步骤
- **执行时机**：在正式测试之前进行
- **验证内容**：验证改写的代码是否符合 Step 01（`FuncCoding/01_func_coding.md`）中定义的面向过程规范要求
- **输出产物**：验证报告保存到 `[ProjectName]/tests/funcTest/verify/[FunctionName]_verify.txt`

### 2. C++ 代码目录规范
- **头文件位置**: `[ProjectName]/include/func/[FunctionName].hpp`
- **源文件位置**: `[ProjectName]/src/func/[FunctionName].cpp`

### 2. 验证检查清单（Checklist）
在生成验证报告时，必须逐项检查以下规范符合性：

#### 代码结构验证
- [ ] **面向过程风格**：未使用类（class），采用纯面向过程 C 风格
- [ ] **One Function Per File**：每个源文件仅包含一个函数定义
- [ ] **物理文件结构**：头文件在 `include/`，源文件在 `src/`，文件名与函数名对应

#### 命名规范验证
- [ ] **函数和变量**：使用小驼峰命名（`lowerCamelCase`）
- [ ] **结构体**：使用大驼峰命名（`UpperCamelCase`）
- [ ] **命名空间**：使用小写下划线（`lowercase_with_underscores`）
- [ ] **宏和常量**：使用大写下划线（`UPPERCASE_WITH_UNDERSCORES`）

#### 代码健壮性验证
- [ ] **静态单赋值（SSA）**：局部变量使用 `const` 且声明时初始化
- [ ] **单一出口原则**：函数仅在末尾有一个 `return`，中间无 `return/break/continue`
- [ ] **头文件保护**：使用 `#pragma once` 而非传统宏保护

#### 模块要素验证
- [ ] **输入（input）**：清晰界定外部输入数据
- [ ] **输出（output）**：清晰界定计算结果输出
- [ ] **状态（state）**：持久化变量明确标注
- [ ] **参数（param）**：配置常量清晰定义

#### 文档规范验证
- [ ] **中文注释**：代码使用中文注释
- [ ] **缩进规范**：tab 缩进为 2 空格
- [ ] **无幻数**：未使用魔法数字（除公式常数外）

### 2. 函数级单元测试（Unit Test）
- **一对一原则**：每个源文件（`[ProjectName]/src/func/[FunctionName].cpp`）对应一个测试文件（`[ProjectName]/tests/funcTest/unit/[FunctionName]/[FunctionName]_test.cpp`）。
- **测试覆盖要求**：每个函数的所有分支路径必须有对应的测试用例。
- **断言验证**：使用 `assert()` 或测试框架的断言宏验证输出符合预期。

### 3. 模块级集成测试（Integration Test）
- **模块组装验证**：测试同一目录下多个函数组合调用的正确性。
- **边界条件测试**：必须包含正常值、边界值、异常值的测试用例。

### 4. 程序级系统测试（System Test）
- **端到端测试**：从主函数入口到最终输出的完整流程测试。
- **回归测试**：每次修改后运行历史测试用例确保无退化。

## 二、测试文件结构规范

### 1. 物理目录结构
```
[ProjectName]/                        # 大组件/大元件独立工程文件夹（如 KalmanFilter/、PIDController/）
├── CMakeLists.txt                    # 构建配置（含测试目标）
├── include/                          # 头文件（扁平化存放，无子目录）
│   ├── func/                          # 普通 C++ 头文件（如 matrixMultiply.hpp）
│   └── mbd/                          # MBD FuncModule 架构头文件（如 MatrixMultiply.hpp）
├── src/                              # 源文件（扁平化存放，无子目录）
│   ├── func/                          # 普通 C++ 源代码（如 matrixMultiply.cpp）
│   └── mbd/                          # MBD FuncModule 架构代码（如 MatrixMultiply.cpp）
├── build/                            # 独立的编译输出目录
├── tests/                            # 测试相关文件
│   ├── funcTest/                      # C++ 测试结果
│   │   ├── unit/                     # 函数级单元测试代码和用例数据（按具体子函数名建子目录）
│   │   │   └── [FunctionName]/
│   │   │       ├── [FunctionName]_test.cpp   # 单元测试代码
│   │   │       ├── [FunctionName]_cases.json # 测试用例数据（JSON 格式）
│   │   │       └── output/           # 单元测试可视化输出子目录
│   │   │           ├── plot_[FunctionName].py    # 画图程序
│   │   │           └── [FunctionName]_plot.png   # 可视化输出图表
│   │   ├── verify/                   # 程序验证结果
│   │   │   ├── [FunctionName]_verify.txt       # 验证报告
│   │   │   └── coding_standard_check.txt       # 代码规范检查清单
│   │   └── Integration/              # 集成测试目录
│   └── mbdTest/                      # MBD 测试相关文件（结构见 ../MbdRefactor/05_mbd_testing.md）
```

### 2. 测试流程说明
1. **验证阶段 (verify/)**：在测试之前，先对改写的程序进行验证，检查是否符合 Step 01（`FuncCoding/01_func_coding.md`）中定义的面向过程规范要求。
2. **单元测试阶段 (unit/)**：只进行模块的单元测试，包含测试代码和测试用例数据。**目录结构与可视化要求**：每个模块都必须进行测试，且在 `[ProjectName]/tests/funcTest/unit/` 目录下必须先按函数名创建独立的子目录（如 `unit/[FunctionName]/`），然后再将对应的测试代码、测试用例放入该子目录下。此外，每个单元测试目录下必须建立 `output/` 子目录（如 `unit/[FunctionName]/output/`），用于存放该单元测试对应的绘图 Python 程序及其输出的图表。
3. **集成测试阶段 (Integration/)**：集成测试及其输出保存在此目录，代替原本的顶层 output 目录。

### 3. 验证报告模板（[ProjectName]/tests/funcTest/verify/[FunctionName]_verify.txt）
```
═══════════════════════════════════════
  [FunctionName] 代码规范验证报告
═══════════════════════════════════════
验证依据：FuncCoding/01_func_coding.md - C++ 面向过程编程规范

【代码结构验证】
[ ] 面向过程风格：未使用类（class）
[ ] One Function Per File：每文件仅一个函数
[ ] 物理文件结构正确

【命名规范验证】
[ ] 函数/变量：小驼峰 (lowerCamelCase)
[ ] 结构体：大驼峰 (UpperCamelCase)
[ ] 命名空间：小写下划线
[ ] 宏/常量：大写下划线

【代码健壮性验证】
[ ] SSA:局部变量使用 const
[ ] 单一出口原则：仅末尾一个 return
[ ] #pragma once 头文件保护

【模块要素验证】
[ ] 输入 (input) 清晰界定
[ ] 输出 (output) 清晰界定
[ ] 状态 (state) 明确标注
[ ] 参数 (param) 清晰定义

【文档规范验证】
[ ] 中文注释
[ ] 缩进 2 空格
[ ] 无幻数

验证结论：□ 通过  □ 需修改
═══════════════════════════════════════
```

### 3. 测试用例 JSON 格式
每个函数/模块的测试用例必须保存为结构化的 JSON 文件，存放在 `[ProjectName]/tests/funcTest/unit/[FunctionName]/[FunctionName]_cases.json`：

```json
{
  "function_name": "calculateAcceleration",
  "description": "计算物体加速度的函数测试用例集",
  "test_cases": [
    {
      "id": "TC001_normal",
      "description": "正常情况：初速度、末速度和时间均为正值",
      "input": {
        "initial_velocity": 0.0,
        "final_velocity": 100.0,
        "time": 5.0
      },
      "expected_output": {
        "acceleration": 20.0
      },
      "tolerance": 1e-6
    },
    {
      "id": "TC002_zero_time",
      "description": "边界情况：时间为零时应返回安全值或错误标志",
      "input": {
        "initial_velocity": 10.0,
        "final_velocity": 20.0,
        "time": 0.0
      },
      "expected_output": {
        "acceleration": 0.0,
        "error_flag": true
      },
      "tolerance": 1e-6
    }
  ]
}
```

### 4. 测试程序模板（unit/[FunctionName]_test.cpp）
```cpp
/**
 * @file [FunctionName]_test.cpp
 * @brief [FunctionName] 函数的单元测试
 * 
 * 测试用例来源：[ProjectName]/tests/funcTest/unit/[FunctionName]/[FunctionName]_cases.json
 */

#include <iostream>
#include <fstream>
#include <cmath>
#include "[FunctionName].hpp"

struct TestCase {
    const char* id;
    double initial_velocity;
    double final_velocity;
    double time;
    double expected_acceleration;
    double tolerance;
};

static const TestCase g_testCases[] = {
    {"TC001_normal", 0.0, 100.0, 5.0, 20.0, 1e-6},
    {"TC002_zero_time", 10.0, 20.0, 0.0, 0.0, 1e-6},
};

bool runTest(const TestCase& tc) {
    const double result = calculateAcceleration(
        tc.initial_velocity, 
        tc.final_velocity, 
        tc.time
    );
    
    const bool passed = std::fabs(result - tc.expected_acceleration) < tc.tolerance;
    
    if (!passed) {
        std::cerr << "[FAIL] " << tc.id 
                  << ": expected=" << tc.expected_acceleration 
                  << ", got=" << result << std::endl;
    } else {
        std::cout << "[PASS] " << tc.id << std::endl;
    }
    
    return passed;
}

int main() {
    unsigned int passed = 0;
    unsigned int total = sizeof(g_testCases) / sizeof(g_testCases[0]);
    
    std::cout << "=== Running Unit Tests for calculateAcceleration ===" << std::endl;
    
    for (const auto& tc : g_testCases) {
        if (runTest(tc)) {
            passed++;
        }
    }
    
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Passed: " << passed << "/" << total << std::endl;
    
    return (passed == total) ? 0 : 1;
}
```

## 三、CMake 测试配置

在 `CMakeLists.txt` 中添加以下配置：

> [!IMPORTANT]
> **【源文件注册与编译要求（CRITICAL）】**：
> 当您在工程中新增或改写了 C++ 算法源文件（例如新建了 `[ProjectName]/src/func/matrixAdd.cpp`），**必须同步更新大组件工程根目录下的 `CMakeLists.txt`**。请确保将新生成的源文件注册进对应的静态库目标（如 `${PROJECT_NAME}_lib`）的源文件列表中（或通过模板自动列入），否则集成测试在链接时会报 `undefined reference` 未定义符号错误。

```cmake
# 强制限制 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 【重要：避免 macOS 大小写不敏感冲突】
# 严禁使用全局 include_directories() 混合包含路径。
# 必须使用 target_include_directories() 针对不同目标隔离包含路径：
# 普通 C++ 测试与库目标仅包含 include/func，不得包含 include/mbd！

enable_testing()

# 单元测试目标
add_executable(test_calculateAcceleration tests/funcTest/unit/calculateAcceleration/calculateAcceleration_test.cpp)
target_link_libraries(test_calculateAcceleration PRIVATE ${PROJECT_NAME}_lib)
add_test(NAME Unit_CalculateAcceleration COMMAND test_calculateAcceleration)

# 集成测试目标
add_executable(test_physics_module tests/funcTest/Integration/test_physics_module.cpp)
add_test(NAME Integration_PhysicsModule COMMAND test_physics_module)

# 自定义测试目标：运行所有测试
add_custom_target(run_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS test_calculateAcceleration test_physics_module
    COMMENT "Running all tests..."
)
```

## 四、测试执行流程

### 1. 分步测试命令
```bash
# 运行前需进入当前组件所在的文件夹，例如: cd [ProjectName]

# Step 0: 程序验证（在正式测试前进行）
mkdir -p tests/funcTest/verify
g++ -std=c++20 -fsyntax-only -Iinclude/func src/func/[FunctionName].cpp > tests/funcTest/verify/[FunctionName]_syntax.txt 2>&1
g++ -std=c++20 -c -Iinclude/func src/func/[FunctionName].cpp -o /tmp/[FunctionName].o > tests/funcTest/verify/[FunctionName]_compile.txt 2>&1

# Step 1: 编译项目（含测试目标）
cmake -DBUILD_TESTING=ON -B build
cmake --build build

# Step 2: 运行函数级单元测试
cd build && ctest -R Unit_ --output-on-failure

# Step 3: 运行模块级集成测试
cd build && ctest -R Integration_ --output-on-failure

# Step 4: 生成详细测试报告
cd build && ctest --output-on-failure --verbose > ../tests/funcTest/Integration/$(date +%Y%m%d_%H%M%S)_report.txt
```

### 2. 测试覆盖率要求
- **关键安全函数**：必须达到 100% 分支覆盖（MC/DC）。
- **普通业务函数**：至少达到 80% 行覆盖。
- **工具建议**：使用 `gcov` + `lcov` 生成覆盖率报告。

## 五、测试结果可视化规范

### 1. 可视化输出目录规范
- **output/ 子目录用途**：每个单元测试模块在其 `unit/[FunctionName]/` 下都必须有一个 `output/` 子文件夹，用于保存该模块的所有 Python 绘图脚本生成的图表文件（如 `.png`），便于用户查看验证。
- **文件命名约定**：`[FunctionName]_plot.png` 或 `[FunctionName]_response.png`

### 2. Python 绘图英文标注原则（CRITICAL）
- **原因**：Python 绘图时中文字体常显示为方块（乱码），因此所有 Python 绘制的图表必须使用**英文**作为标题、坐标轴标签和图例。
- **规范示例**：
  ```python
  plt.title("Step Response")           # ✅ 正确：英文标题
  plt.xlabel("Time (s)")               # ✅ 正确：英文坐标轴标签
  plt.ylabel("Value")                  # ✅ 正确：英文坐标轴标签
  plt.legend(["Setpoint", "Measurement"])  # ✅ 正确：英文图例
  
  # ❌ 错误：避免使用中文
  # plt.title("阶跃响应")  
  # plt.xlabel("时间 (秒)")
  ```

### 3. 可视化输出规范补充
- **README 引用路径**：如 `[ProjectName]/tests/funcTest/unit/[FunctionName]/output/simulation_plot.png`（相对路径，基于项目根目录）

### 4. Python 绘图脚本环境约束
所有用于测试结果可视化的 Python 脚本必须遵循以下规范：

```python
#!/Users/qingxu/.ai-env/bin/python3
"""
测试结果的可视化脚本
用于绘制输入/输出对比图、误差分析图等
"""

import matplotlib
matplotlib.use('Agg')  # 防止在后台无 GUI 环境运行时发生阻塞
import matplotlib.pyplot as plt
import json
import os
from datetime import datetime

# 你的绘图代码...
```

### 2. Shebang 指令要求
- Python 脚本首行必须添加 Shebang 指令，指向特定的虚拟环境解析器。
- 示例：`#!/Users/qingxu/.ai-env/bin/python3`

### 4. 可视化脚本模板
```python
#!/Users/qingxu/.ai-env/bin/python3
"""
单元测试结果可视化脚本
读取测试用例 JSON 并绘制输入/输出对比图
"""

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import json
import os
from datetime import datetime

def load_test_cases(json_path):
    with open(json_path, 'r', encoding='utf-8') as f:
        return json.load(f)

def plot_comparison(test_data, output_dir=None):
    # 每一个模块的测试绘图结果保存在该模块单元测试下的 output/ 子文件夹中
    if output_dir is None:
        output_dir = os.path.join('tests/funcTest/unit', test_data['function_name'], 'output')  # 注意：在 [ProjectName] 目录下运行，故使用相对路径
    os.makedirs(output_dir, exist_ok=True)
    
    # 提取数据
    inputs = [tc['input'] for tc in test_data['test_cases']]
    expected = [tc['expected_output'] for tc in test_data['test_cases']]
    
    # 绘制对比图
    plt.figure(figsize=(10, 6))
    # ... 绘图逻辑
    
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    save_path = os.path.join(output_dir, f"{test_data['function_name']}_{timestamp}.png")
    plt.savefig(save_path)
    print(f"Plot saved to {save_path}")

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 2:
        print("Usage: python plot_tests.py tests/funcTest/unit/[FunctionName]/[FunctionName]_cases.json")
        sys.exit(1)
    
    test_data = load_test_cases(sys.argv[1])
    plot_comparison(test_data)
```