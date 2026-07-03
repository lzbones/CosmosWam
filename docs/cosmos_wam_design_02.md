# CosmosWAM 交通场景未来风险趋势预测系统设计文档

版本：v0.1  
日期：2026-06-29  
适用阶段：概念设计、原型开发、算法接口拆分、前后端协同实现

## 1. 文档目标

本文档用于规划一个基于 Cosmos 世界模型的交通场景未来风险趋势预测程序。程序输入交通场景图像，支持自车驾驶员视角和路侧摄像头视角，预测未来若干秒内交通风险的空间分布、时间变化趋势和交通参与者未来轨迹，并以大地坐标系下的 2D 与可操作 3D 形式展示结果。

本文档后续应作为以下工作的参考：

- 产品功能边界定义
- 算法模块拆分
- 输入输出数据结构设计
- 2D/3D 可视化交互设计
- API 与工程目录规划
- 训练、评估、部署路线制定

## 2. 背景与核心设想

交通场景风险不是单一目标检测问题，而是一个空间、时间、语义和交互共同决定的动态预测问题。系统需要理解当前图像中的道路结构、交通参与者、障碍物、交通标识标线、可行驶区域和潜在遮挡，并结合世界模型对未来若干秒进行多模态推演。

Cosmos 类型的世界基础模型适合承担“物理世界理解、未来状态推演、动作/轨迹预测、场景生成或模拟”的底层能力。根据 NVIDIA Cosmos 官方介绍，Cosmos 面向机器人、自动驾驶、智能基础设施等 Physical AI 场景，支持视觉理解、世界生成、未来预测和动作建模等能力。本文档的设计思路是：不把 Cosmos 直接当成最终产品，而是将其作为未来场景推演和语义理解的基础模型，在其上构建交通风险专用的预测头、坐标变换模块、概率校准模块和可视化系统。

## 3. 产品一句话定义

CosmosWAM 是一个面向交通场景的世界模型风险预测与可视化系统：输入一张或一段交通场景视觉数据，输出未来若干秒在大地坐标系下的风险热力图、交通参与者概率轨迹、关键风险解释，并提供 2D 鸟瞰图和可交互 3D 场景展示。

## 4. 目标用户与使用场景

### 4.1 目标用户

- 自动驾驶算法研发人员
- 交通安全评估人员
- 车路协同系统研发人员
- 智能交通运维人员
- 数据标注与场景挖掘人员
- 演示系统或评测平台使用者

### 4.2 典型使用场景

1. 自车视角风险预测  
   用户输入车辆前视摄像头图片或短视频，系统预测未来 1-8 秒内自车周边风险变化，包括前车急刹、行人横穿、非机动车切入、道路施工障碍物、车道线约束等。

2. 路侧摄像头风险预测  
   用户输入路口或道路侧方摄像头画面，系统将画面映射到大地坐标系，预测路口冲突点、行人过街风险、转弯车辆和直行车辆交互风险、遮挡区域风险等。

3. 事故隐患分析  
   对历史场景图片或视频进行离线分析，生成风险热力图序列、参与者轨迹概率、危险来源解释和导出报告。

4. 模型演示与决策辅助  
   在 2D/3D 视图中播放未来风险变化，让用户直观看到“风险在哪里、何时升高、由谁引起、概率多大、可能路径有哪些”。

## 5. 设计原则

### 5.1 风险必须落在真实空间中

所有核心输出都应落到统一的大地坐标系或局部 ENU 坐标系，而不是只停留在图片像素坐标。这样才能支持多摄像头融合、HD Map 叠加、2D/3D 展示和后续交通工程分析。

### 5.2 预测结果必须表达不确定性

未来交通行为存在多种可能。系统不应只输出一条“最可能轨迹”，而应输出多模态轨迹及概率、风险热力图概率值、空间不确定性范围和置信度。

### 5.3 风险是时空序列，不是单帧标签

系统输出应包含多个未来时间步，例如 t+0.5s、t+1.0s、t+2.0s、t+3.0s、t+5.0s。用户需要看到风险从低到高、从局部到扩散、从某个交通参与者引发到整体场景变化的过程。

### 5.4 可视化要同时服务直觉和调试

普通用户需要一眼看懂红色高风险、蓝色低风险；研发人员还需要查看概率数值、轨迹分支、坐标、模型置信度、参与者 ID、风险来源和中间层结果。

### 5.5 原型优先，工程可扩展

第一版可以先支持单图输入、手动相机标定、2D BEV 展示和离线推理；架构上应预留视频输入、多摄像头融合、在线推理、可交互 3D 和训练闭环。

## 6. 范围定义

### 6.1 v0.1 应包含

- 输入一张交通场景图片
- 支持自车视角和路侧视角两种模式
- 支持用户提供或手动配置相机内参、外参、地面参考点
- 将图片中的道路区域、交通参与者、障碍物和标识标线投影到大地坐标系或局部 ENU 坐标系
- 输出未来若干秒的风险热力图序列
- 输出交通参与者未来轨迹的多模态预测
- 在 2D BEV 视图中展示风险热力图、轨迹、参与者、道路元素
- 预留 3D 视图的数据结构和接口
- 导出 JSON、PNG、MP4 或交互式结果包

### 6.2 v0.1 不强制包含

- 真正安全认证或车规级功能安全闭环
- 实时在线部署
- 高精地图自动接入
- 多摄像头自动融合
- 端到端完全自动标定
- 直接控制车辆或发送控制指令

### 6.3 未来版本应扩展

- 视频输入和连续帧跟踪
- 多摄像头融合
- 激光雷达、毫米波雷达、V2X 数据接入
- HD Map 和交通灯相位接入
- 可操作 3D 结果查看器
- 场景检索、批量分析和风险报告生成
- 模型训练、主动学习和数据闭环

## 7. 核心概念定义

### 7.1 大地坐标系与局部坐标系

系统内部建议采用两层坐标表达：

1. 全球地理坐标  
   使用 WGS84 经纬度和高度，字段为 latitude、longitude、altitude。该坐标适合数据交换、地图定位和跨区域管理。

2. 局部 ENU 坐标  
   以场景中心点或相机安装点为原点，使用 East、North、Up 坐标，单位为米。该坐标适合模型输出、网格热力图、轨迹计算和 3D 渲染。

建议内部计算主要使用 ENU，导入导出时提供 WGS84 与 ENU 的互转元数据。

### 7.2 风险

本文档中的风险定义为：在未来某一时间步、某一空间位置发生碰撞、冲突、违法/异常行为、不可通行、遮挡导致不可确认危险或交通参与者行为不确定性导致的潜在危险的概率或强度。

风险值记为：

```text
R(x, y, t) in [0, 1]
```

其中：

- x, y 表示 ENU 平面坐标
- t 表示未来时间
- 0 表示低风险
- 1 表示高风险

### 7.3 风险热力图

风险热力图是对未来时间 t 的空间风险场离散化表示：

```text
risk_grid[t_index][row][col] = probability
```

每个网格单元代表真实世界中固定大小的区域，例如 0.2m x 0.2m、0.5m x 0.5m 或 1.0m x 1.0m。

### 7.4 交通参与者轨迹

交通参与者包括但不限于：

- 机动车
- 非机动车
- 行人
- 动物或其他移动目标
- 自车
- 特殊车辆

每个参与者未来轨迹应支持多模态表达：

```text
actor_i:
  mode_1: probability=0.55, trajectory=[(x, y, yaw, v, t), ...]
  mode_2: probability=0.30, trajectory=[...]
  mode_3: probability=0.15, trajectory=[...]
```

### 7.5 障碍物与道路元素

静态或低速风险来源包括：

- 障碍物
- 施工区域
- 路缘
- 隔离栏
- 锥桶
- 停车车辆
- 交通标志牌
- 交通信号灯
- 车道线
- 人行横道
- 停止线
- 导流线
- 禁停区
- 可行驶区域边界

这些元素不一定都有轨迹，但会影响风险场和交通参与者轨迹概率。

## 8. 输入设计

### 8.1 最小输入

```json
{
  "scenario_id": "demo_001",
  "input_type": "image",
  "view_type": "ego_front",
  "image_path": "data/input/demo_001.jpg",
  "timestamp": "2026-06-29T10:00:00+08:00"
}
```

### 8.2 推荐输入

```json
{
  "scenario_id": "demo_001",
  "input_type": "image",
  "view_type": "roadside",
  "image": {
    "path": "data/input/demo_001.jpg",
    "width": 1920,
    "height": 1080,
    "timestamp": "2026-06-29T10:00:00+08:00"
  },
  "camera": {
    "camera_id": "cam_crossing_01",
    "intrinsics": {
      "fx": 1400.0,
      "fy": 1400.0,
      "cx": 960.0,
      "cy": 540.0,
      "distortion": [0.0, 0.0, 0.0, 0.0, 0.0]
    },
    "extrinsics": {
      "position_wgs84": {
        "latitude": 31.2304,
        "longitude": 121.4737,
        "altitude": 12.0
      },
      "rotation": {
        "roll": 0.0,
        "pitch": -12.0,
        "yaw": 83.0,
        "unit": "degree"
      }
    }
  },
  "world": {
    "origin_wgs84": {
      "latitude": 31.2304,
      "longitude": 121.4737,
      "altitude": 0.0
    },
    "coordinate_system": "ENU",
    "ground_plane": {
      "type": "flat",
      "z": 0.0
    }
  },
  "prediction": {
    "horizon_seconds": 5.0,
    "time_step_seconds": 0.5,
    "grid_resolution_m": 0.5,
    "x_range_m": [-50.0, 80.0],
    "y_range_m": [-40.0, 40.0],
    "trajectory_modes": 6
  }
}
```

### 8.3 可选输入

- 连续视频或多帧图片
- 自车速度、转角、加速度、导航路径
- 路侧摄像头安装高度、朝向和畸变参数
- 地面控制点 GCP
- HD Map 或简化道路矢量图
- 交通信号灯状态
- 天气、光照、道路湿滑程度
- 历史检测框或外部感知结果
- 人工标注的危险区域或关注区域

## 9. 输出设计

### 9.1 总体输出结构

```json
{
  "scenario_id": "demo_001",
  "model_version": "cosmoswam-risk-v0.1",
  "created_at": "2026-06-29T10:00:05+08:00",
  "coordinate_system": {
    "type": "ENU",
    "origin_wgs84": {
      "latitude": 31.2304,
      "longitude": 121.4737,
      "altitude": 0.0
    },
    "unit": "meter"
  },
  "time_horizon": {
    "start": 0.0,
    "end": 5.0,
    "step": 0.5
  },
  "risk_maps": [],
  "actors": [],
  "static_elements": [],
  "scene_summary": {},
  "exports": {}
}
```

### 9.2 风险热力图输出

```json
{
  "time_s": 2.0,
  "grid": {
    "resolution_m": 0.5,
    "origin_xy_m": [-50.0, -40.0],
    "width": 260,
    "height": 160,
    "values_encoding": "float32_row_major",
    "values_uri": "outputs/demo_001/risk_t_2.0.bin"
  },
  "statistics": {
    "max_risk": 0.91,
    "mean_risk": 0.14,
    "p95_risk": 0.62
  },
  "hotspots": [
    {
      "id": "hotspot_001",
      "center_xy_m": [18.5, -2.0],
      "risk": 0.91,
      "main_causes": ["pedestrian_crossing", "vehicle_turning_conflict"],
      "related_actor_ids": ["actor_003", "actor_007"]
    }
  ]
}
```

### 9.3 交通参与者输出

```json
{
  "actor_id": "actor_003",
  "category": "pedestrian",
  "current_state": {
    "position_xy_m": [14.2, -3.5],
    "z_m": 0.0,
    "yaw_rad": 1.57,
    "speed_mps": 1.2,
    "bbox_2d": [810, 420, 860, 560],
    "bbox_3d": {
      "center_xyz_m": [14.2, -3.5, 0.9],
      "size_lwh_m": [0.6, 0.6, 1.8],
      "yaw_rad": 1.57
    },
    "confidence": 0.94
  },
  "future_trajectories": [
    {
      "mode_id": "m1",
      "probability": 0.62,
      "points": [
        {"t_s": 0.5, "x_m": 14.4, "y_m": -2.9, "yaw_rad": 1.57, "speed_mps": 1.2},
        {"t_s": 1.0, "x_m": 14.5, "y_m": -2.3, "yaw_rad": 1.57, "speed_mps": 1.2}
      ],
      "uncertainty": [
        {"t_s": 0.5, "sigma_x_m": 0.25, "sigma_y_m": 0.35, "rho": 0.1},
        {"t_s": 1.0, "sigma_x_m": 0.35, "sigma_y_m": 0.45, "rho": 0.1}
      ],
      "risk_contribution": 0.78,
      "semantic_intent": "crossing_road"
    }
  ]
}
```

### 9.4 场景摘要输出

```json
{
  "scene_summary": {
    "overall_risk_level": "high",
    "overall_risk_score": 0.86,
    "critical_time_s": 2.5,
    "critical_location_xy_m": [18.5, -2.0],
    "main_causes": [
      "左转车辆与横穿行人存在冲突",
      "人行横道附近存在遮挡区域",
      "目标 actor_003 的横穿概率较高"
    ],
    "recommended_attention": [
      "关注 t+2.0s 至 t+3.0s 的路口中心区域",
      "关注 actor_007 左转轨迹与 actor_003 行人轨迹交叉点"
    ]
  }
}
```

## 10. 坐标转换设计

### 10.1 坐标链路

系统需要支持以下坐标链路：

```text
Image pixel (u, v)
  -> Camera coordinate (Xc, Yc, Zc)
  -> Ego / roadside sensor coordinate
  -> Local ENU world coordinate (x, y, z)
  -> WGS84 coordinate (lat, lon, alt)
```

### 10.2 自车视角

自车视角通常已知相机安装位置和车辆坐标系。推荐使用：

- 相机内参 K
- 相机到车体坐标外参 T_camera_to_ego
- 自车定位姿态 T_ego_to_world
- 地面平面或深度估计

单图情况下，远处目标深度误差会明显影响世界坐标，因此输出应包含坐标置信度。

### 10.3 路侧视角

路侧摄像头需要解决图像到地面的映射。推荐分三档实现：

1. 手动四点标定  
   用户在图像中点选至少 4 个地面控制点，并输入对应 ENU 或经纬度坐标。系统估计平面单应性矩阵。

2. 相机参数标定  
   用户提供内参、外参、安装高度和地面平面，系统通过投影模型转换坐标。

3. 地图辅助自动标定  
   系统根据车道线、停止线、人行横道、路缘等与地图矢量自动对齐，优化外参。

### 10.4 地面假设与非平面修正

v0.1 可先假设地面局部平坦。后续应支持：

- 坡道和高架
- 路缘高度
- 人行天桥或地下通道
- 多层道路
- 数字高程模型 DEM

### 10.5 坐标误差表达

每个从图像映射到世界的对象应包含：

```json
{
  "position_xy_m": [12.3, -4.5],
  "position_covariance": [[0.25, 0.02], [0.02, 0.36]],
  "projection_confidence": 0.82
}
```

这有助于在轨迹和风险热力图中传播空间不确定性。

## 11. 算法总体架构

### 11.1 模块总览

```text
Input Image / Video
  -> Input Loader
  -> Camera Calibration & Coordinate Transform
  -> Scene Perception
      -> Actors
      -> Static Obstacles
      -> Road Markings
      -> Traffic Signs / Signals
      -> Drivable Area
      -> Occlusion Regions
  -> Cosmos-based World Prediction
      -> Future Scene Rollout
      -> Actor Intent Prediction
      -> Interaction Reasoning
  -> Trajectory Prediction Head
  -> Risk Field Prediction Head
  -> Probability Calibration
  -> 2D / 3D Visualization
  -> Export & Report
```

### 11.2 Cosmos 的角色

Cosmos 在系统中建议承担三类能力：

1. 场景理解  
   理解图片或视频中的道路结构、参与者关系、交通语义和物理可行性。

2. 未来推演  
   对未来若干秒可能出现的状态进行生成式或隐空间推演，得到多种可能的世界演化。

3. 动作和意图建模  
   推断交通参与者的可能意图，例如直行、左转、右转、变道、横穿、减速、停车、避让。

### 11.3 任务专用头

不建议只依靠大模型文本输出完成全部预测。生产系统应在 Cosmos 基础能力之上增加任务专用模块：

- Actor Detection Head
- Actor Tracking Head
- BEV Feature Head
- Trajectory Prediction Head
- Occupancy Prediction Head
- Risk Heatmap Head
- Uncertainty Head
- Explanation Head

原型阶段可以用提示词或外部检测模型快速生成结构化结果，但正式系统应转向可训练、可评估、可校准的数值输出。

### 11.4 两条实现路径

#### 路径 A：快速原型路径

适合早期演示。

- 使用现成检测/分割模型识别交通参与者和道路元素
- 使用手动标定将结果映射到 BEV
- 使用 Cosmos 或 VLM 对场景做语义理解和未来意图推断
- 使用规则模型加轨迹预测模型生成未来轨迹
- 使用 TTC、PET、距离、遮挡和语义风险生成热力图
- 前端展示 2D 结果

优点：实现快、便于演示。  
缺点：精度和一致性受规则影响，难以形成端到端评估。

#### 路径 B：生产模型路径

适合长期研发。

- 使用 Cosmos 作为世界模型 backbone 或教师模型
- 训练交通场景专用 BEV 表征
- 使用多任务学习同时输出检测、轨迹、占用和风险
- 使用真实数据、仿真数据和人工标注风险数据联合训练
- 使用概率校准和 OOD 检测保证结果可信度

优点：结果一致、可评估、可迭代。  
缺点：需要数据、训练资源和较长研发周期。

## 12. 感知模块设计

### 12.1 目标检测

检测对象类别：

- car
- truck
- bus
- motorcycle
- bicycle
- pedestrian
- traffic_cone
- barrier
- construction_object
- unknown_obstacle

输出：

- 2D bbox
- 类别
- 置信度
- 可选实例分割 mask
- 可选 3D bbox
- 可选深度

### 12.2 道路结构识别

需要识别：

- 车道线
- 停止线
- 人行横道
- 路缘
- 可行驶区域
- 路口区域
- 导流区
- 禁停区
- 非机动车道
- 人行道

输出应尽量转为世界坐标下的矢量线或多边形，而不是只保留像素 mask。

### 12.3 交通标志标线

识别：

- 限速
- 禁止通行
- 禁止停车
- 转向箭头
- 让行标志
- 信号灯
- 斑马线
- 公交车道

这些元素应进入风险解释模块。例如：车辆轨迹穿过停止线、行人接近人行横道、车辆可能违反转向约束。

### 12.4 遮挡区域

遮挡本身是风险来源。系统应识别：

- 大车遮挡
- 建筑物遮挡
- 路边停放车辆遮挡
- 画面边缘不可见区域
- 弯道或坡道不可见区域

遮挡区域在风险图中不应简单视为低风险，而应根据可能出现交通参与者的概率产生“未知风险”。

## 13. 轨迹预测设计

### 13.1 预测对象

轨迹预测针对所有动态参与者：

- 自车
- 周边车辆
- 行人
- 非机动车
- 特殊移动目标

### 13.2 预测时间

建议默认：

```text
horizon = 5.0s
step = 0.5s
time_steps = [0.5, 1.0, 1.5, ..., 5.0]
```

可配置：

- 快速预警：0-3s
- 城市场景：0-5s
- 路口推演：0-8s
- 离线分析：0-10s

### 13.3 多模态轨迹

每个参与者输出 K 条候选轨迹：

```text
K = 3, 6 or 12
sum(probabilities) = 1
```

示例：

- 车辆：直行 0.55，减速停车 0.25，变道 0.20
- 行人：继续等待 0.40，横穿 0.45，后退 0.15
- 非机动车：直行 0.60，绕行 0.25，突然左偏 0.15

### 13.4 轨迹概率表达

轨迹概率建议同时通过三种方式展示：

1. 线宽  
   概率越高，轨迹线越宽。

2. 透明度  
   概率越高，透明度越低，视觉越实。

3. 端点标签  
   在轨迹终点或用户悬停时显示概率，例如 62%。

### 13.5 轨迹不确定性表达

每个未来时间点可输出协方差椭圆：

```text
mean = (x, y)
covariance = [[sigma_x^2, rho], [rho, sigma_y^2]]
```

2D 展示中：

- 在每个关键时间点画半透明椭圆
- 椭圆越大表示空间不确定性越高
- 椭圆颜色跟随该轨迹模式

3D 展示中：

- 用半透明椭球或轨迹管道包络表达不确定性
- 也可以将时间轴向上抬升，形成“时间-空间轨迹带”

### 13.6 轨迹与风险的关系

轨迹本身不是风险，风险来自轨迹之间、轨迹与道路约束之间、轨迹与障碍物之间的冲突。系统应计算：

- 参与者之间的最小距离
- 预计碰撞时间 TTC
- 预计冲突后时间 PET
- 轨迹穿越不可行驶区域的概率
- 轨迹与停止线/人行横道/信号灯规则的冲突
- 遮挡区域中潜在参与者与当前参与者的冲突

## 14. 风险热力图设计

### 14.1 风险来源分解

建议将总风险拆成多个通道：

```text
R_total = combine(
  R_collision,
  R_near_miss,
  R_occupancy,
  R_static_obstacle,
  R_road_boundary,
  R_rule_violation,
  R_occlusion,
  R_uncertainty,
  R_vulnerable_road_user
)
```

各通道含义：

- R_collision：未来碰撞风险
- R_near_miss：近失风险
- R_occupancy：未来被交通参与者占用的概率
- R_static_obstacle：静态障碍物风险
- R_road_boundary：路缘、护栏、不可通行区域风险
- R_rule_violation：违反交通规则或道路约束带来的风险
- R_occlusion：遮挡导致的未知风险
- R_uncertainty：模型不确定性风险
- R_vulnerable_road_user：行人、非机动车等弱势交通参与者相关风险

### 14.2 风险融合方式

初期可采用加权融合：

```text
R_total = clip(sum(w_i * R_i), 0, 1)
```

更推荐采用概率并集融合：

```text
R_total = 1 - product(1 - w_i * R_i)
```

这样多个中等风险叠加时会自然升高，同时不会超过 1。

### 14.3 空间平滑

热力图需要根据概率平滑过渡。建议：

- 对每个参与者轨迹点生成高斯核风险分布
- 核宽度随预测时间增加而增大
- 沿运动方向使用各向异性高斯核
- 对道路边界、障碍物等使用距离场生成连续风险
- 对所有风险通道进行边缘保持平滑，避免完全模糊掉车道边界

示例：

```text
sigma_longitudinal(t) = base_sigma + k1 * t * speed
sigma_lateral(t) = base_sigma + k2 * t
```

### 14.4 时间平滑

风险热力图应避免相邻时间步闪烁。建议：

- 使用 temporal EMA 平滑
- 对关键风险热点保留峰值
- 对出现和消失的风险区域使用淡入淡出

示例：

```text
R_smooth(t) = alpha * R_raw(t) + (1 - alpha) * R_smooth(t - step)
```

### 14.5 颜色设计

用户要求红色高风险、蓝色低风险。建议采用如下连续色带：

```text
0.00 - 0.20: 深蓝 / 蓝
0.20 - 0.40: 青色
0.40 - 0.60: 绿色 / 黄绿
0.60 - 0.80: 黄色 / 橙色
0.80 - 1.00: 红色 / 深红
```

设计要求：

- 色带必须固定，不随当前场景最大值自动重映射，否则不同场景之间不可比较
- 图例必须显示 0、0.25、0.5、0.75、1.0
- 允许用户切换线性或分段增强显示
- 高风险红色区域不应完全遮挡轨迹和道路元素
- 默认热力图透明度建议 45%-65%

### 14.6 风险等级

建议将连续风险值映射为工程等级：

```text
0.00 - 0.20: very_low
0.20 - 0.40: low
0.40 - 0.60: medium
0.60 - 0.80: high
0.80 - 1.00: critical
```

等级用于 UI 标签、报告、筛选和告警，但模型内部仍保留连续概率。

## 15. 2D 可视化设计

### 15.1 2D 主视图

2D 主视图采用 BEV 鸟瞰图，以局部 ENU 坐标为平面坐标。中心可以是自车、路口中心或摄像头投影点。

图层从底到顶建议为：

1. 底图  
   空白网格、简化道路图、HD Map、卫星图或用户导入地图。

2. 道路元素层  
   车道线、停止线、人行横道、路缘、可行驶区域。

3. 静态障碍物层  
   锥桶、护栏、施工区域、停车车辆。

4. 动态参与者当前状态层  
   当前 bbox、朝向箭头、速度文本、actor_id。

5. 轨迹预测层  
   多模态轨迹、概率线宽、协方差椭圆、终点概率标签。

6. 风险热力图层  
   未来某时间步的风险概率热力图。

7. 风险热点层  
   高风险中心点、冲突点、相关参与者连线。

8. 标注和悬停信息层  
   tooltip、数值、解释。

### 15.2 轨迹展示样式

车辆轨迹：

- 当前框使用矩形或 3D box 投影
- 轨迹为带箭头的曲线
- 高概率模式使用实线
- 低概率模式使用虚线或更透明的线
- 轨迹颜色可以按类别或风险贡献决定

行人轨迹：

- 当前状态使用圆点或人体图标
- 轨迹使用更柔和的曲线
- 不确定性椭圆更明显，因为行人运动随机性更大

非机动车轨迹：

- 使用较细但高亮的曲线
- 对横向摆动和突然变向保留概率分支

### 15.3 轨迹概率视觉编码

推荐规则：

```text
line_width_px = 1.5 + 6.0 * probability
opacity = 0.20 + 0.75 * probability
dash = probability < 0.20
```

示例：

- 0.70 概率：粗实线，高透明度低
- 0.30 概率：中等线宽，半透明
- 0.08 概率：细虚线，仅在展开低概率模式时显示

### 15.4 时间控制

底部时间轴：

- 播放 / 暂停
- 单步前进 / 后退
- 拖动时间滑块
- 时间刻度：0s、1s、2s、3s、5s
- 支持固定某一时刻查看，也支持动画播放

在动画播放时：

- 风险热力图随时间更新
- 轨迹上当前时间点高亮
- 参与者 ghost box 移动到预测位置
- 冲突点在临近时刻闪烁或增强

### 15.5 右侧检查面板

当用户选中一个参与者时，显示：

- actor_id
- 类别
- 当前速度
- 当前朝向
- 检测置信度
- 投影置信度
- Top-K 轨迹模式
- 每条轨迹概率
- 风险贡献
- 主要交互对象
- 关键冲突时间
- 关键冲突位置
- 语义意图

当用户选中风险热点时，显示：

- 风险值
- 风险等级
- 出现时间
- 持续时间
- 位置坐标
- 相关参与者
- 风险来源分解
- 建议关注点

### 15.6 视图控件

应提供：

- 图层开关
- 风险阈值滑块
- 热力图透明度滑块
- 轨迹模式数量选择
- 坐标网格开关
- 距离测量工具
- 截图导出
- 视频导出
- JSON 导出
- 重置视角

## 16. 3D 可视化设计

### 16.1 3D 目标

3D 视图不是简单把 2D 图抬起来，而是用于理解真实空间关系、摄像头视角、遮挡、目标高度、未来轨迹和风险随时间变化。

3D 视图应支持：

- 地面平面或道路模型
- 相机位置和视锥显示
- 当前交通参与者 3D box
- 未来轨迹 3D 曲线
- 风险热力图贴地显示
- 风险区域抬升显示
- 轨迹不确定性体显示
- 交互旋转、平移、缩放、选中

### 16.2 风险热力图 3D 表达

建议提供两种模式：

1. 贴地热力图  
   将 2D 风险图作为半透明纹理贴在地面上。适合直观看空间风险。

2. 风险高度场  
   将风险值映射为 z 方向高度：

```text
z = risk * max_height
```

高风险区域形成红色隆起，低风险区域贴近地面。适合展示风险强度变化。

### 16.3 轨迹 3D 表达

推荐三种轨迹显示方式：

1. 贴地轨迹带  
   轨迹在地面上显示为有宽度的 ribbon，宽度和透明度表示概率。

2. 抬升轨迹管  
   轨迹稍微抬高 0.1-0.3m，避免与热力图视觉冲突。

3. 时间高度轨迹  
   将未来时间映射到高度：

```text
z = t * time_height_scale
```

这样每条轨迹形成时空曲线，用户可以同时看到空间位置和未来时间。

### 16.4 不确定性 3D 表达

可以使用：

- 半透明椭球
- 轨迹包络管
- 概率云
- 未来占用体素

默认建议：

- v1.0 使用椭圆柱或半透明椭球
- v2.0 使用概率体素或 occupancy volume

### 16.5 3D 交互

必须支持：

- Orbit 旋转
- Pan 平移
- Zoom 缩放
- 点选 actor / hotspot
- 时间轴播放
- 切换俯视、相机视角、自由视角
- 显示或隐藏相机视锥
- 导出 GLB / glTF

建议支持：

- 第一人称重放
- 多视角分屏
- 2D 与 3D 联动选中
- 点击风险区域回放原图位置

## 17. 原图视角叠加设计

除了 BEV 和 3D，系统应保留原图视角叠加，用于验证模型是否理解正确。

原图视角展示：

- 检测框
- 分割 mask
- 道路元素
- 当前风险来源
- 参与者 ID
- 未来轨迹回投影
- 风险热点回投影

注意：原图回投影只能作为辅助显示，最终风险标准输出仍应以世界坐标为准。

## 18. 风险解释设计

系统不应只输出红色热力图，还应解释风险原因。

### 18.1 风险解释粒度

1. 场景级  
   当前场景整体风险高低，最危险时间和位置。

2. 热点级  
   某个高风险区域为何危险。

3. 参与者级  
   某个 actor 对风险的贡献和可能意图。

4. 规则级  
   是否涉及车道线、停止线、人行横道、信号灯、限速等约束。

### 18.2 解释示例

```json
{
  "risk_explanation": {
    "hotspot_id": "hotspot_001",
    "summary": "t+2.5s 时路口中心存在高风险冲突",
    "causes": [
      {
        "type": "trajectory_conflict",
        "actors": ["actor_003", "actor_007"],
        "probability": 0.74,
        "description": "左转车辆与横穿行人的预测轨迹在路口中心交叉"
      },
      {
        "type": "occlusion",
        "probability": 0.38,
        "description": "路边停放车辆遮挡了人行横道入口"
      }
    ]
  }
}
```

## 19. API 设计

### 19.1 创建场景

```http
POST /api/scenarios
Content-Type: multipart/form-data
```

输入：

- image 或 video
- metadata.json

输出：

```json
{
  "scenario_id": "demo_001",
  "status": "created"
}
```

### 19.2 发起预测

```http
POST /api/scenarios/{scenario_id}/predict
Content-Type: application/json
```

请求：

```json
{
  "horizon_seconds": 5.0,
  "time_step_seconds": 0.5,
  "grid_resolution_m": 0.5,
  "trajectory_modes": 6,
  "enable_3d": true
}
```

响应：

```json
{
  "prediction_id": "pred_001",
  "status": "running"
}
```

### 19.3 查询预测结果

```http
GET /api/predictions/{prediction_id}
```

响应：

```json
{
  "prediction_id": "pred_001",
  "status": "completed",
  "result_uri": "/api/predictions/pred_001/result"
}
```

### 19.4 获取某时间步风险图

```http
GET /api/predictions/{prediction_id}/risk-map?time_s=2.0
```

支持返回：

- PNG 可视化
- float32 binary
- NumPy npy
- GeoTIFF
- JSON 小图

### 19.5 导出结果

```http
POST /api/predictions/{prediction_id}/exports
```

请求：

```json
{
  "formats": ["json", "png_sequence", "mp4", "glb", "report_md"]
}
```

## 20. 数据文件结构建议

```text
CosmosWam/
  docs/
    cosmoswam_design.md
  data/
    input/
      images/
      videos/
      metadata/
    processed/
      calibration/
      bev/
      perception/
    outputs/
      predictions/
      exports/
  src/
    cosmoswam/
      api/
      config/
      data/
      geometry/
      models/
      perception/
      prediction/
      risk/
      visualization/
      export/
      utils/
  web/
    src/
      components/
      views/
      layers/
      stores/
      api/
  tests/
    unit/
    integration/
    fixtures/
  scripts/
    run_demo.py
    calibrate_camera.py
    export_report.py
```

## 21. 前端界面规划

### 21.1 总体布局

建议采用专业工具型布局：

```text
Top Bar:
  Project / Scenario selector / Run prediction / Export

Left Panel:
  Input
  Calibration
  Model settings
  Layer controls

Center:
  2D BEV view or 3D view

Right Panel:
  Selected actor / hotspot inspector
  Risk explanation
  Numeric details

Bottom:
  Timeline
  Playback controls
  Event markers
```

### 21.2 页面

1. Scenario Import  
   上传图片/视频，填写视角类型、相机参数、坐标原点。

2. Calibration  
   点选地面控制点，预览图像到 BEV 的映射。

3. Prediction Workspace  
   主工作区，查看 2D/3D 风险预测。

4. Report Export  
   导出图片、视频、JSON、报告。

5. Model Debug  
   查看中间输出、性能、日志和错误。

### 21.3 前端技术建议

2D：

- MapLibre GL 或 deck.gl
- Canvas/WebGL 热力图
- SVG/Canvas 叠加轨迹和标签

3D：

- Three.js
- deck.gl 3D layers
- CesiumJS，如果需要真实地理地图和地形

UI：

- React
- Zustand 或 Redux
- Tailwind 或本地设计系统

## 22. 后端技术建议

### 22.1 服务框架

推荐：

- Python
- FastAPI
- Pydantic
- PyTorch
- OpenCV
- NumPy
- SciPy
- pyproj
- Shapely
- rasterio 或 xarray

### 22.2 模型服务

可选方式：

1. 同进程加载模型  
   简单，但资源管理不灵活。

2. 独立模型服务  
   FastAPI 后端调用 Cosmos 模型服务、检测模型服务、轨迹模型服务。

3. OpenAI-compatible / vLLM 风格接口  
   适合接入支持视觉输入的模型服务，便于替换底层模型。

### 22.3 存储

原型阶段：

- 本地文件系统
- SQLite
- JSON metadata

生产阶段：

- 对象存储
- PostgreSQL/PostGIS
- Redis 队列
- MLflow 或 Weights & Biases 记录模型版本

## 23. 模型训练设计

### 23.1 数据来源

候选数据：

- 自采车载摄像头数据
- 路侧摄像头数据
- nuScenes
- Waymo Open Dataset
- Argoverse
- INTERACTION Dataset
- inD / rounD / highD
- CARLA 或其他仿真数据
- Cosmos 生成或增强的合成数据

### 23.2 标注内容

必要标注：

- 交通参与者类别和位置
- 轨迹历史和未来轨迹
- 可行驶区域
- 车道线和道路边界
- 信号灯和标志
- 静态障碍物
- 坐标标定参数

风险标注：

- 碰撞或近失事件
- TTC / PET 阈值事件
- 人工标注高风险区域
- 规则冲突区域
- 遮挡风险区域
- 场景级风险等级

### 23.3 损失函数

轨迹：

```text
L_traj = NLL_GMM + ADE + FDE + mode_classification_loss
```

占用：

```text
L_occ = BCE_or_focal_loss + dice_loss
```

风险：

```text
L_risk = focal_loss + calibration_loss + temporal_consistency_loss
```

解释：

```text
L_exp = classification_loss + relation_loss
```

总体：

```text
L_total = a*L_traj + b*L_occ + c*L_risk + d*L_exp + e*L_consistency
```

### 23.4 概率校准

系统输出是概率，不只是分数，因此需要校准：

- Temperature Scaling
- Isotonic Regression
- Platt Scaling
- Reliability Diagram
- Expected Calibration Error
- Brier Score

轨迹模式概率、风险热力图概率和整体风险等级都应做校准。

## 24. 评估指标

### 24.1 感知指标

- mAP
- IoU
- Lane F1
- Drivable Area IoU
- Depth error
- Projection error in meters

### 24.2 轨迹指标

- minADE
- minFDE
- Miss Rate
- NLL
- Mode probability calibration
- Collision consistency

### 24.3 风险指标

- AUROC
- AUPRC
- Precision / Recall at high-risk threshold
- Risk hotspot localization error
- Critical time error
- Brier Score
- Expected Calibration Error
- Top-K risk recall

### 24.4 可视化与产品指标

- 单场景加载时间
- 单次预测耗时
- 2D 渲染帧率
- 3D 渲染帧率
- 导出耗时
- 用户是否能在 10 秒内定位最高风险点

## 25. 性能目标

### 25.1 原型目标

- 单张图片离线预测：30-120 秒
- BEV 风险网格：0.5m 分辨率
- 预测范围：前后左右 50-100m
- 预测时间：5 秒
- 时间步长：0.5 秒
- 轨迹模式数：3-6
- 2D 交互帧率：30 FPS

### 25.2 研发目标

- 单场景预测：5-15 秒
- 支持短视频输入
- 支持 0.2m-0.5m 风险网格
- 3D 交互帧率：30 FPS
- 支持批量离线处理

### 25.3 未来实时目标

- 端到端延迟小于 500ms-2s
- 支持流式视频
- 支持连续风险场更新
- 支持边缘设备或车载 GPU 部署

## 26. 安全与可信设计

### 26.1 非安全认证声明

系统早期版本只能用于研发、分析和演示，不应直接用于自动驾驶控制或生命安全相关决策。

### 26.2 置信度与不确定性

每个关键输出都应有置信度：

- 检测置信度
- 坐标投影置信度
- 轨迹概率
- 风险概率
- 场景 OOD 分数

当置信度低时，UI 应明确显示“低置信度”或“需要人工确认”。

### 26.3 OOD 检测

需要识别：

- 极端天气
- 夜间强眩光
- 摄像头遮挡
- 严重模糊
- 不支持道路类型
- 标定缺失或错误
- 模型未见过的交通参与者

### 26.4 审计日志

每次预测应记录：

- 输入文件
- 参数
- 模型版本
- 标定版本
- 输出文件
- 推理时间
- 错误日志
- 用户修改记录

## 27. 迭代路线图

### 27.1 M0：设计与静态 Demo

目标：

- 完成设计文档
- 定义 JSON schema
- 准备 1-3 个示例场景
- 手工构造风险热力图和轨迹数据
- 完成 2D 静态展示原型

交付：

- 文档
- 示例输入输出
- 静态可视化页面

### 27.2 M1：2D 原型

目标：

- 支持图片上传
- 支持手动标定
- 支持 BEV 坐标转换
- 支持检测结果导入或自动检测
- 支持规则生成风险热力图
- 支持轨迹多模态展示

交付：

- 可运行 Web Demo
- 示例预测结果
- JSON 导出

### 27.3 M2：Cosmos 接入

目标：

- 接入 Cosmos 或 Cosmos 风格模型服务
- 使用模型进行场景理解和未来意图推断
- 将模型输出转为结构化 actor、trajectory、risk hint
- 与规则风险引擎融合

交付：

- 模型推理 pipeline
- prompt / adapter 设计
- 模型输出解析器

### 27.4 M3：任务模型训练

目标：

- 构建训练数据集
- 训练轨迹预测头
- 训练风险热力图头
- 评估和校准概率

交付：

- 训练脚本
- 验证指标
- 模型权重
- 误差分析报告

### 27.5 M4：3D 交互视图

目标：

- 支持 3D 地面、道路元素、相机视锥
- 支持 3D 风险热力图和轨迹
- 支持 2D/3D 联动
- 支持 GLB / MP4 导出

交付：

- 3D 可操作界面
- 3D 数据导出格式

### 27.6 M5：实时与多源融合

目标：

- 支持视频流
- 支持多摄像头
- 支持地图和交通灯接入
- 支持在线更新风险热力图

交付：

- 在线推理系统
- 多源融合模块
- 性能优化报告

## 28. 关键开放问题

1. 输入到底是单图优先，还是短视频优先？  
   单图可以演示，但未来轨迹预测不确定性较大；短视频能显著提升速度和意图估计。

2. 是否有相机标定参数？  
   没有标定时，只能做近似 BEV，世界坐标可信度会下降。

3. 大地坐标精度要求是多少？  
   如果只是演示，0.5m-2m 可接受；如果做工程评估，可能需要 0.1m-0.3m 级别。

4. 风险概率是否需要严格等价于事故概率？  
   如果需要，训练数据和标注要求会大幅提高。早期建议定义为“归一化风险强度 + 校准概率”。

5. 2D 和 3D 哪个是第一优先级？  
   建议先做 2D BEV，因为算法输出天然是 2D 时空风险场；3D 在数据结构稳定后再做。

6. 是否需要高精地图？  
   没有高精地图时可用感知出的道路元素；有高精地图时风险解释和规则约束会更可靠。

7. Cosmos 是直接推理，还是作为教师模型/基础模型？  
   原型阶段可直接推理；长期建议训练任务专用头。

## 29. 第一版最小可行产品建议

第一版建议不要试图一次做完整自动驾驶级系统，而是做一个强可视化、可解释、可扩展的离线原型。

### 29.1 MVP 输入

- 单张图片
- 视角类型：ego_front 或 roadside
- 手动标定点
- 预测参数

### 29.2 MVP 输出

- 2D BEV 风险热力图
- Top-3 参与者轨迹
- 每条轨迹概率
- 风险热点
- 场景风险摘要
- JSON 导出
- PNG/MP4 导出

### 29.3 MVP 技术路线

1. 图片上传
2. 手动标定
3. 外部检测模型或人工示例数据
4. 简单轨迹生成器
5. 规则风险场生成
6. Cosmos 场景解释接入
7. 2D 可视化
8. 导出结果

### 29.4 MVP 成功标准

- 用户能导入一张交通图片
- 用户能完成地面标定
- 系统能显示 BEV 地面坐标
- 系统能显示未来 5 秒风险变化
- 系统能显示每个参与者的多条概率轨迹
- 用户能点击高风险区域看到原因
- 用户能导出 JSON 和图片序列

## 30. 推荐实现顺序

1. 定义数据 schema  
   先稳定输入、输出、坐标、risk_grid、actor、trajectory 的结构。

2. 构造一份 mock 预测结果  
   不依赖模型，先做可视化。

3. 实现 2D BEV 渲染  
   画底图、热力图、actor、轨迹、时间轴。

4. 实现手动标定  
   让真实图片能映射到 BEV。

5. 接入检测模型  
   自动识别目标和道路元素。

6. 实现规则风险引擎  
   根据轨迹、距离、TTC、道路约束生成风险图。

7. 接入 Cosmos 场景理解  
   用于意图、风险解释、复杂交互推断。

8. 训练或微调任务模型  
   将规则和大模型能力沉淀为稳定数值模型。

9. 做 3D 展示  
   使用同一份世界坐标输出渲染 3D。

10. 做批量评估和报告  
   支持真实研发闭环。

## 31. 参考资料

- NVIDIA Cosmos 官方页面：https://www.nvidia.com/en-us/ai/cosmos/
- NVIDIA Cosmos GitHub 仓库：https://github.com/NVIDIA/Cosmos
- NVIDIA Cosmos 论文入口：https://arxiv.org/abs/2501.03575

## 32. 结论

CosmosWAM 的核心不应只是“给图片上色”，而应是一个以世界坐标为核心的未来风险预测系统。它需要把图像理解、坐标转换、世界模型推演、多模态轨迹预测、概率风险场、可解释风险分析和 2D/3D 可视化统一起来。

最稳妥的开发策略是：先用 mock 数据和规则引擎把数据结构、2D BEV、时间轴、轨迹概率和风险热力图跑通；再接入 Cosmos 做语义理解和未来推演；最后逐步训练任务专用模型，让结果从演示型系统走向可评估、可迭代的研发平台。
