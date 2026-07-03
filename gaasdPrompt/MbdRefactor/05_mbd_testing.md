# MBD FuncModule 架构测试与验证 Prompt 模板

---

```markdown
# 角色与任务
你是一个资深控制系统的测试工程师，负责为已按照"FuncModule 架构规范"重构的代码库生成完整的测试体系，包括**程序验证**、Traits 级单元测试及结果可视化。

## 🧪 一、MBD 测试流程与策略

### 1. 程序验证（Verify）- 新增步骤
- **执行时机**：在正式测试之前进行
- **验证内容**：验证重构的代码是否符合 Step 04（`MbdRefactor/04_mbd_refactor.md`）中定义的 FuncModule 架构规范要求
- **输出产物**：验证报告保存到 `[ProjectName]/tests/mbdTest/verify/[SubModuleName]_verify.txt`

### 2. MBD 代码目录规范
- **头文件位置**: `[ProjectName]/include/mbd/[SubModuleName].hpp`
- **源文件位置**: `[ProjectName]/src/mbd/[SubModuleName].cpp`

### 2. 验证检查清单（Checklist）
在生成验证报告时，必须逐项检查以下规范符合性：

#### Traits 五元结构验证
- [ ] **Input 结构体**：定义了周期性高频刷新的外部输入信息
- [ ] **Output 结构体**：定义了当前周期计算产生的结果
- [ ] **Param 结构体**：定义了相对静态的配置或参数约束
- [ ] **State 结构体**：定义了闭环累加或带记忆的内部时序状态（无状态时定义为空结构体）
- [ ] **Sub 结构体**：定义了级联依赖的子模块实体（值语义，非指针）

#### 基类与继承验证
- [ ] **继承 FuncModule**：模块类继承自 `FuncModule<XxxxTraits>`
- [ ] **引入构造函数**：使用 `using FuncModule::FuncModule;` 引入基类构造函数
- [ ] **run() 方法签名**：正确实现 `void run(const Input &input, Output &output)`

#### 值语义与依赖注入验证
- [ ] **无指针设计**：类内部不使用裸指针、智能指针或引用
- [ ] **先配置后移动**：子模块的 setParam/setState 在 std::move 之前执行
- [ ] **std::move 使用**：正确通过 std::move 将子模块注入 Sub 结构体

#### MBD 注解规范验证（复合模块）
- [ ] **魔术注释标记**：run() 方法中包含 `// === MBD_AUTO_GEN_BEGIN [Xxxx] ===` 和 `// === MBD_AUTO_GEN_END [Xxxx] ===`
- [ ] **JSON 蓝图文件**：复合模块生成了 `models/[ModuleName].json` 拓扑定义文件
- [ ] **执行序列一致**：子模块调用顺序与 JSON 拓扑中的 execution_sequence 一致

#### 命名与文件结构验证
- [ ] **Traits 命名**：使用 `XxxxTraits` 格式命名特征结构体
- [ ] **头文件格式**：使用 `.hpp` 扩展名，包含 `#pragma once`
- [ ] **物理目录结构**：include/、src/、models/目录正确分离

### 2. Traits 级单元测试（叶子节点测试）
- **测试对象**：每个继承自 `FuncModule` 的叶子节点类。
- **测试重点**：验证 `run()` 方法在给定 `Input` + `Param` + `State` 下是否产生正确的 `Output`。
- **用例对齐要求（与函数版本对齐）**：MBD 版本的测试必须在与函数（Func）版本完全相同的测试用例输入（Input）、参数（Param）和初始状态（State）下进行，以对比验证 MBD 改写/重构后的正确性。
- **测试用例保存**：使用 JSON 格式存储，与 Traits 结构体一一对应。

### 3. 复合模块集成测试（级联测试）
- **测试对象**：包含子模块 `Sub` 的复合模块。
- **测试重点**：验证子模块之间的数据路由、执行顺序和输出聚合是否正确。
- **测试用例来源**：基于 `models/[ModuleName].json` 拓扑蓝图自动生成测试骨架。

### 4. 系统级端到端测试（闭环仿真）
- **测试对象**：完整的控制回路或仿真场景。
- **测试重点**：从传感器输入到执行器输出的完整数据流，以及时间域上的动态响应。
- **回归测试**：保存历史场景的输入/输出序列，每次修改后自动回放验证。

## 二、MBD 测试用例管理规范

### 1. 物理目录结构规范

在开始测试之前，请确保项目遵循以下目录结构：

```
[ProjectName]/                        # 大组件/大元件独立工程文件夹（如 KalmanFilter/、PIDController/）
├── CMakeLists.txt                    # 构建配置（含测试目标）
├── include/                          # 头文件（扁平化存放，无子目录）
│   ├── func/                          # 普通 C++ 头文件（如 matrixMultiply.hpp）
│   └── mbd/                          # MBD FuncModule 架构头文件（如 MatrixMultiply.hpp）
├── src/                              # 源文件（扁平化存放，无子目录）
│   ├── func/                          # 普通 C++ 源代码（如 matrixMultiply.cpp）
│   └── mbd/                          # MBD FuncModule 架构代码（如 MatrixMultiply.cpp）
├── models/                           # MBD 拓扑蓝图（仅包含当前大组件 of the 拓扑 JSON，无子目录）
│   └── [ProjectName].json
├── build/                            # 独立的编译输出目录
└── tests/                            # 测试相关文件
    ├── funcTest/                      # C++ 测试相关文件（结构见 ../FuncCoding/02_func_testing.md）
    └── mbdTest/                      # MBD 测试相关文件
        ├── unit/                     # Traits 级单元测试代码和用例数据（按具体子类名建子目录）
        │   └── [SubModuleName]/
        │       ├── [SubModuleName]_test.cpp     # 单元测试代码
        │       ├── [SubModuleName]_cases.json   # 测试用例数据（JSON 格式）
        │       └── output/           # 可视化输出子目录（存放绘图脚本与结果图表）
        │           ├── plot_[SubModuleName].py    # 画图程序
        │           └── [SubModuleName]_response.png # 可视化输出图表
        ├── verify/                   # 程序验证结果
        │   ├── [SubModuleName]_verify.txt       # 验证报告
        │   └── funcmodule_arch_check.txt     # FuncModule 架构规范检查清单
        └── Integration/              # 集成测试目录
```

> [!IMPORTANT]
> **【源文件注册与编译要求（CRITICAL）】**：
> 当您在工程中新增或重构了 MBD 算法源文件（例如新建了 `[ProjectName]/src/mbd/PIDController.cpp`），**必须同步更新组件工程根目录下的 `CMakeLists.txt`**。请确保将新生成的源文件注册进对应的静态库目标（如 `control_lib`）的源文件列表中（或通过模板自动列入），否则集成测试在链接时会报 `undefined reference` 未定义符号错误。

### 2. 测试流程说明
1. **验证阶段 (verify/)**：在测试之前，先对重构的 MBD 程序进行验证，检查是否符合 Step 04（`MbdRefactor/04_mbd_refactor.md`）中定义的 FuncModule 架构规范要求。
2. **单元测试阶段 (unit/)**：只进行模块的单元测试，包含测试代码和测试用例数据。**目录结构与可视化要求**：每个模块都必须进行测试，且在 `[ProjectName]/tests/mbdTest/unit/` 目录下必须先按子模块名创建独立的子目录（如 `unit/[SubModuleName]/`），然后再将对应的测试代码、测试用例放入该子目录下。此外，每个单元测试目录下必须建立 `output/` 子目录（如 `unit/[SubModuleName]/output/`），用于存放该单元测试对应的绘图 Python 程序及其输出的图表。
3. **集成测试阶段 (Integration/)**：集成测试及其输出保存在此目录，代替原本的顶层 output 目录。

### 3. 验证报告模板（[ProjectName]/tests/mbdTest/verify/[SubModuleName]_verify.txt）
```
═══════════════════════════════════════
  [SubModuleName] FuncModule 架构验证报告
═══════════════════════════════════════
验证依据：MbdRefactor/04_mbd_refactor.md - MBD FuncModule 架构重构规范

【Traits 五元结构验证】
[ ] Input:定义了外部输入信息
[ ] Output:定义了计算结果输出
[ ] Param:定义了配置参数约束
[ ] State:定义了内部时序状态
[ ] Sub:定义了子模块实体 (值语义)

【基类与继承验证】
[ ] 继承 FuncModule<XxxxTraits>
[ ] using FuncModule::FuncModule 引入构造函数
[ ] run() 方法签名正确

【值语义与依赖注入验证】
[ ] 无指针设计
[ ] 先配置后移动 (CRITICAL)
[ ] std::move 正确使用

【MBD 注解规范验证】(复合模块适用)
[ ] MBD_AUTO_GEN_BEGIN/END魔术注释
[ ] models/[ModuleName].json 蓝图文件存在
[ ] 执行序列与 JSON 拓扑一致

【命名与文件结构验证】
[ ] Traits 命名：XxxxTraits 格式
[ ] 头文件格式：.hpp + #pragma once
[ ] 物理目录结构正确

验证结论：□ 通过  □ 需修改
═══════════════════════════════════════
```

### 3. 测试用例 JSON 结构（针对 Traits 模块）
每个模块必须在 `[ProjectName]/tests/mbdTest/unit/[SubModuleName]/` 目录下拥有一个对应的 JSON 文件：

```json
{
  "module_name": "PIDController",
  "traits_type": "PIDControllerTraits",
  "description": "PID 控制器的测试用例集",
  "test_cases": [
    {
      "id": "TC001_step_response",
      "description": "阶跃响应测试：给定固定误差，验证比例项输出",
      "input": {
        "setpoint": 100.0,
        "measurement": 80.0
      },
      "param": {
        "kp": 2.0,
        "ki": 0.5,
        "kd": 1.0
      },
      "state": {
        "integral_accumulator": 0.0,
        "prev_error": 0.0
      },
      "expected_output": {
        "control_signal": 40.0
      },
      "tolerance": 1e-5
    }
  ]
}
```

### 4. 测试程序模板（unit/[ModuleName]_test.cpp）
```cpp
/**
 * @file [SubModuleName]_test.cpp
 * @brief [SubModuleName] 模块的 Traits 级单元测试
 * 
 * 测试用例来源：[ProjectName]/tests/mbdTest/unit/[SubModuleName]/[SubModuleName]_cases.json
 */

#include <iostream>
#include <cmath>
#include "[ModuleName].hpp"

using namespace control;

struct TestCase {
    const char* id;
    double setpoint, measurement;
    double kp, ki, kd;
    double integral_init, prev_error_init;
    double expected_control;
    double tolerance;
};

bool runTest(const TestCase& tc) {
    // 1. 实例化模块并配置
    PIDController controller(
        PIDControllerTraits::Param{tc.kp, tc.ki, tc.kd},
        PIDControllerTraits::Sub{}
    );
    
    // 2. 设置初始状态
    controller.setState(PIDControllerTraits::State{
        .integral_accumulator = tc.integral_init,
        .prev_error = tc.prev_error_init
    });
    
    // 3. 准备输入并执行
    PIDControllerTraits::Input input{tc.setpoint, tc.measurement};
    PIDControllerTraits::Output output;
    
    controller.run(input, output);
    
    // 4. 验证输出
    const bool passed = std::fabs(output.control_signal - tc.expected_control) < tc.tolerance;
    
    if (!passed) {
        std::cerr << "[FAIL] " << tc.id 
                  << ": expected=" << tc.expected_control 
                  << ", got=" << output.control_signal << std::endl;
    } else {
        std::cout << "[PASS] " << tc.id << std::endl;
    }
    
    return passed;
}

int main() {
    static const TestCase g_testCases[] = {
        {"TC001_step", 100.0, 80.0, 2.0, 0.5, 1.0, 0.0, 0.0, 40.0, 1e-5},
    };
    
    unsigned int passed = 0;
    std::cout << "=== Running Unit Tests for PIDController ===" << std::endl;
    
    for (const auto& tc : g_testCases) {
        if (runTest(tc)) {
            passed++;
        }
    }
    
    std::cout << "\nPassed: " << passed << "/" 
              << sizeof(g_testCases)/sizeof(g_testCases[0]) << std::endl;
    
    return (passed == sizeof(g_testCases)/sizeof(g_testCases[0])) ? 0 : 1;
}
```

## 三、MBD 复合模块测试与图形化映射

### 1. 基于 models/*.json 的自动测试生成
对于复合模块，可以利用其 `[ProjectName]/models/[ProjectName].json` 拓扑文件自动生成测试骨架：

```python
#!/Users/qingxu/.ai-env/bin/python3
"""
根据 MBD JSON 拓扑自动生成复合模块测试代码
使用方法：python generate_mbd_test.py models/[ProjectName].json
"""

import json
import sys

def parse_topology(json_path):
    with open(json_path, 'r') as f:
        return json.load(f)

def generate_integration_test(topology):
    module_name = topology['name']
    sub_modules = [s for s in topology.get('ports', {}).get('Sub', [])]
    
    test_code = f"""
// Auto-generated integration test for {module_name}
// Sub-modules: {', '.join(sub_modules)}

void test_{module_name.lower()}() {{
    // TODO: Implement sub-module data routing verification
}}
"""
    return test_code

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("Usage: python generate_mbd_test.py models/[Module].json")
        sys.exit(1)
    
    topology = parse_topology(sys.argv[1])
    print(generate_integration_test(topology))
```

### 2. MBD_AUTO_GEN 区域测试保护
在复合模块的 `run()` 方法中，`// === MBD_AUTO_GEN_BEGIN [Xxxx] ===` 和 `// === MBD_AUTO_GEN_END [Xxxx] ===` 之间的代码由图形化平台生成。测试时必须确保：
- **前置/后置自定义逻辑**不影响自动生成的数据路由。
- **子模块调用顺序**与 JSON 拓扑中的 `execution_sequence` 一致。

> [!IMPORTANT]
**【源文件注册与编译要求（CRITICAL）】**：
当您在工程中新增或重构了 MBD 算法源文件（例如新建了 `[ProjectName]/src/mbd/PIDController.cpp`），**必须同步更新组件工程根目录下的 `CMakeLists.txt`**。请确保将新生成的源文件注册进对应的静态库目标（如 `control_lib`）的源文件列表中（或通过模板自动列入），否则集成测试在链接时会报 `undefined reference` 未定义符号错误。

## 四、CMake MBD 测试配置

```cmake
# 强制限制 C++20 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 【重要：避免 macOS 大小写不敏感冲突】
# 严禁使用全局 include_directories() 混合包含路径。
# 必须使用 target_include_directories() 针对不同目标隔离包含路径：
# MBD 库与测试目标优先仅包含 include/mbd（如必须包含 include/func，请务必将 include/mbd 放在最前面！）

enable_testing()

# MBD Traits 级单元测试
add_executable(test_pid_controller tests/mbdTest/unit/PIDController/PIDController_test.cpp)
target_link_libraries(test_pid_controller PRIVATE control_lib)
add_test(NAME MBD_Unit_PIDController COMMAND test_pid_controller)

# MBD 复合模块集成测试
add_executable(test_control_chain tests/mbdTest/Integration/test_control_chain.cpp)
target_link_libraries(test_control_chain PRIVATE control_lib)
add_test(NAME MBD_Integration_ControlChain COMMAND test_control_chain)

# MBD 专用测试目标：验证所有图形化拓扑生成的模块
add_custom_target(mbd_all_tests
    COMMAND ${CMAKE_CTEST_COMMAND} -R "MBD_" --output-on-failure
    DEPENDS test_pid_controller test_control_chain
    COMMENT "Running all MBD-generated module tests..."
)
```

## 五、MBD 测试执行流程

### 1. 分步测试命令
```bash
# 运行前需进入当前组件所在的文件夹，例如: cd [ProjectName]

# Step 0: 程序验证（在正式测试前进行）
mkdir -p tests/mbdTest/verify
# 检查 FuncModule 架构规范
python3 [PromptDir]/script/check_funcmodule_arch.py src/mbd/ > tests/mbdTest/verify/architecture_check.txt 2>&1
# 编译验证
g++ -std=c++20 -fsyntax-only -Iinclude/mbd -Iinclude/func src/mbd/[SubModuleName].cpp > tests/mbdTest/verify/[SubModuleName]_syntax.txt 2>&1

# Step 1: 编译项目（含 MBD 测试目标）
cmake -DBUILD_TESTING=ON -DENABLE_MBD_TESTS=ON -B build
cmake --build build

# Step 2: 运行 Traits 级单元测试（叶子节点）
cd build && ctest -R "MBD_Unit_" --output-on-failure

# Step 3: 运行复合模块集成测试
cd build && ctest -R "MBD_Integration_" --output-on-failure

# Step 4: 生成 MBD 专项测试报告
cd build && ctest --output-on-failure --verbose > ../tests/mbdTest/Integration/mbd_$(date +%Y%m%d_%H%M%S)_report.txt
```

### 2. 测试报告内容要求
测试报告必须包含以下信息：
- **模块名称与版本**：被测试的 FuncModule 类名和代码版本。
- **输入/参数/状态快照**：每个测试用例的具体配置。
- **输出对比表**：预期值 vs. 实际值的逐字段比较。
- **拓扑验证结果**（复合模块）：子模块调用顺序和数据路由是否符合 JSON 蓝图。

## 六、MBD 回归测试与持续集成

- **场景回放测试**：保存典型工况的输入序列（如阶跃、正弦、随机扰动），定期自动回放。
- **性能基准跟踪**：记录每个版本的关键模块执行时间，防止性能退化。
- **Git 钩子集成**：在 `pre-push` 阶段运行 `mbd_all_tests` 目标，确保图形化生成的代码始终通过测试。

## 七、MBD 测试结果可视化规范

### 1. 可视化输出目录规范
- **output/ 子目录用途**：每个单元测试模块在其 `unit/[ModuleName]/` 下都必须有一个 `output/` 子文件夹，用于保存该模块的所有 Python 绘图脚本生成的图表文件（如 `.png`），便于用户查看验证。
- **文件命名约定**：`[ModuleName]_response.png` 或 `[ModuleName]_plot.png`

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
- **README 引用路径**：如 `[ProjectName]/tests/mbdTest/unit/[SubModuleName]/output/pid_step_response.png`（相对路径，基于项目根目录）

### 4. Python 绘图脚本环境约束
所有用于 MBD 测试结果可视化的 Python 脚本必须遵循以下规范：

```python
#!/Users/qingxu/.ai-env/bin/python3
"""
MBD 模块闭环仿真结果可视化脚本
用于绘制阶跃响应、频域分析等图表
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

### 4. 闭环仿真可视化模板
```python
#!/Users/qingxu/.ai-env/bin/python3
"""
MBD 模块闭环仿真结果可视化脚本
绘制阶跃响应曲线、误差收敛图等
"""

import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np
import json
import os
from datetime import datetime

def load_simulation_data(json_path):
    with open(json_path, 'r', encoding='utf-8') as f:
        return json.load(f)

def plot_step_response(data, output_dir=None):
    # 每一个模块的测试绘图结果保存在该模块单元测试下的 output/ 子文件夹中
    if output_dir is None:
        output_dir = os.path.join('tests/mbdTest/unit', data['module_name'], 'output')  # 注意：在 [ProjectName] 目录下运行，故使用相对路径
    os.makedirs(output_dir, exist_ok=True)
    
    time = np.array(data['time'])
    setpoint = np.array(data['setpoint'])
    measurement = np.array(data['measurement'])
    
    plt.figure(figsize=(12, 6))
    plt.plot(time, setpoint, 'r--', label='Setpoint', linewidth=2)
    plt.plot(time, measurement, 'b-', label='Measurement', linewidth=2)
    plt.xlabel('Time (s)')
    plt.ylabel('Value')
    plt.title(f"Step Response - {data['module_name']}")
    plt.legend()
    plt.grid(True, alpha=0.3)
    
    timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
    save_path = os.path.join(output_dir, f"{data['module_name']}_step_{timestamp}.png")
    plt.savefig(save_path, dpi=150)
    print(f"Plot saved to {save_path}")

if __name__ == '__main__':
    import sys
    if len(sys.argv) < 2:
        print("Usage: python plot_mbd_response.py simulation_data.json")
        sys.exit(1)
    
    data = load_simulation_data(sys.argv[1])
    plot_step_response(data)
```

## 八、MBD 测试覆盖率补充说明

由于 MBD 架构的特性，除传统行覆盖外，还需关注：
- **拓扑覆盖率**：每个子模块在集成测试中被调用的次数和路径。
- **数据流覆盖率**：Input→Sub→Output 的数据传递链路是否全覆盖。
- **状态转移覆盖率**：State 结构体中所有字段在不同场景下的变化路径。

建议使用定制化的 MBD 覆盖率分析工具，结合 `models/*.json` 拓扑文件生成可视化覆盖热力图。