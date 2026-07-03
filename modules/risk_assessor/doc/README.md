# Module 7: Risk Assessor (行车安全场评估模块)

The `risk_assessor` module computes the bird's-eye-view (BEV) driving risk grid by reconstructing velocities/accelerations and modeling safety fields (potential and kinetic energy).

## 📌 Core Purpose / 核心作用
在局部坐标（ENU）网格中，求解车辆的安全行车潜力场与行驶动能场叠加图，用以描述行车过程中的瞬态危险度空间分布，并输出 BEV 栅格风险分布矩阵。

## ⛓️ Core Interfaces / 核心接口
```cpp
namespace cosmos_wam {

struct EgoState {
    double timestamp;
    double x;
    double y;
    double vx, vy;
    double speed;
    double ax, ay;
    double accel_mag;
};

class RiskAssessor {
public:
    RiskAssessor();
    ~RiskAssessor() = default;

    // 根据输入轨迹重构与计算障碍物和自车的速度、加速度运动学参数
    void reconstructKinematics(std::vector<EgoState>& ego_states, 
                               std::vector<EnvVehicleTrajectory>& env_trajectories);

    // 运行安全场核心算法，求解行车危险度栅格矩阵并落盘保存
    bool assessRiskAndSave(const std::string& input_trajectories_filepath, 
                           const std::string& output_filepath);
};

}
```

## ⚙️ Implementation & Algorithms / 实现原理与细节
1. **运动学重构与轨迹高频插值 (Path Interpolation)**：
   * 目标检测的原始时间步为 `0.2s`（低频），这会导致空间危险场网格中出现断层或跳变。
   * Risk Assessor 核心计算前执行 **`0.05s` 高频线性轨迹插值**，确保车辆路径的平滑与连续。
   * 对插值后的点坐标实施数值一阶、二阶中心差分，逆向重构出速度 $(v_x, v_y)$、加速度 $(a_x, a_y)$。
2. **统一动力学安全场模型 (Unified Dynamic Safety Field)**：
   * 空间的风险网格（$X \in [-20, 20]$ 米，$Y \in [-5, 80]$ 米）中任意一点 $(x_g, y_g)$ 的危险强度由车辆势能场与动能场复合决定：
   * **静态势能场（Potential Field）**：受库伦斥力启发，与距离平方反比，表征障碍物自身固有的侵入性边界风险。
   * **动态动能场（Kinetic Field）**：车辆具有速度和航向，其场强在航向正方向呈非对称拉伸（扇形伸展），场强与车辆动能 $\frac{1}{2}m v^2$ 成正比例放大，代表车辆前方潜在失控或制动避让所能覆盖的危险区域。
3. **指数衰减尺度缩放 (Progress-based Exponential Decay)**：
   * 越往未来预测的时间点，不确定性越高，场强影响力应逐步退化。
   * 系统沿着路径执行时长进展对当前场强系数做指数尺度衰减（以 `norm_t = t / max_t` 为自变量，场强从起点的 `0.9` 随时间指数退化至终点的 `0.2`）。
4. **栅格落盘与临时更名**：
   * 采用 `.tmp` 写入，完成后 `std::rename` 更名为 `frame_result.json`，确保其他模块或绘图工具调用时的安全性。
