# MbdRefactor - MBD 架构重构与测试 Prompt

本文件夹包含 MBD FuncModule 架构重构和测试生成的 Prompt 模板。

## 📁 文件结构

```
MbdRefactor/
├── README.md                 # 本说明文件
├── 04_mbd_refactor.md        # FuncModule 架构重构规范
└── 05_mbd_testing.md         # MBD 测试生成与验证规范
```

## 📋 执行顺序

**Step 04 → Step 05** - 必须先执行架构重构，再执行测试生成

---

## 04_mbd_refactor.md - FuncModule 架构重构规范

### 核心任务
将原始 C++ 控制算法重构为符合 **C++20 FuncModule 架构** 的形态，使其能够接入模型驱动（MBD）的图形化代码生成流。

### Traits 五元结构
每个模块必须包含以下五个部分：

| 要素 | 说明 |
|------|------|
| **Input** | 周期性高频刷新的外部输入信息 |
| **Output** | 当前周期计算产生的结果 |
| **Param** | 相对静态的配置或参数约束 |
| **State** | 闭环累加或带记忆的内部时序状态 |
| **Sub** | 级联依赖的子模块实体（值语义） |

### 关键规范
- **继承 FuncModule**: 模块类继承自 `FuncModule<XxxxTraits>`
- **零样板继承**: 使用 `using FuncModule::FuncModule;` 引入基类构造函数
- **无指针设计**: 类内部不使用裸指针、智能指针或引用
- **先配置，后移动**: 子模块的 setParam/setState 在 std::move 之前执行

### MBD 注解规范（复合模块）
```cpp
void Xxxx::run(const Input &input, Output &output) {
  // === MBD_AUTO_GEN_BEGIN [Xxxx] ===
  // 这里是由图形化拓扑工具自动生成的子模块调度逻辑
  // === MBD_AUTO_GEN_END [Xxxx] ===
}
```

### 输出产物
- `[ProjectName]/include/mbd/[ModuleName].hpp` - Traits 五元结构体和类声明
- `[ProjectName]/src/mbd/[ModuleName].cpp` - run() 等算法实现
- `[ProjectName]/models/[ProjectName].json` - （仅大组件/复合模块）图形化拓扑蓝图

---

## 05_mbd_testing.md - MBD 测试生成与验证规范

### 核心任务
为已按照"FuncModule 架构规范"重构的代码库生成完整的测试体系。

### 执行流程
1. **程序验证 (verify/)**: 验证重构代码是否符合 04_mbd_refactor.md 的 FuncModule 架构规范要求
2. **单元测试 (unit/)**: Traits 级单元测试（叶子节点测试）
3. **可视化输出 (output/)**: 测试结果的画图程序及其输出

### 验证检查清单
- [ ] Traits 五元结构完整定义（Input、Output、Param、State、Sub）
- [ ] 继承 FuncModule<XxxxTraits>
- [ ] using FuncModule::FuncModule 引入构造函数
- [ ] run() 方法签名正确
- [ ] 无指针设计
- [ ] 先配置后移动 (CRITICAL)
- [ ] MBD_AUTO_GEN_BEGIN/END 魔术注释（复合模块适用）

### 输出产物
```
[ProjectName]/tests/mbdTest/
├── unit/                   # Traits 级单元测试代码和用例数据
│   ├── [Module]_test.cpp
│   └── [Module]_cases.json
├── verify/                 # 程序验证结果
│   ├── [Module]_verify.txt
│   └── funcmodule_arch_check.txt
└── output/                 # 可视化输出和画图程序
    ├── plot_[Module].py
    └── [Module]_response.png

源代码位置：
- [ProjectName]/src/mbd/[ModuleName].cpp      # 源文件
- [ProjectName]/include/mbd/[ModuleName].hpp  # 头文件
- [ProjectName]/models/[ProjectName].json      # MBD 拓扑蓝图（仅复合模块）
```

---

## 🔗 相关链接

- **主入口**: `../00_gaasdPrompt.md` - 全流程编排 Prompt（总调度入口）
- **上一步**: `../FuncDesign/03_design_doc_gen.md` - 设计文档生成
