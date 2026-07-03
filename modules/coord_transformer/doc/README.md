# Module 6: Coordinate Transformer (空间坐标变换模块)

The `coord_transformer` module projects 2D image coordinates to 3D camera coordinates, translates them to the local ego East-North-Up (ENU) frame, and converts offsets to absolute geodetic (WGS-84) latitude and longitude.

## 📌 Core Purpose / 核心作用
将 2D 图像像素空间的目标边界框转换投影到 3D 摄像机坐标系，再利用自车姿态转为局部三维大地坐标（ENU），并结合 GPS 基准计算出所有交通参与者的绝对经纬度位置（WGS-84）。

## ⛓️ Core Interfaces / 核心接口
```cpp
namespace cosmos_wam {

struct ReferencePoint {
    double latitude;
    double longitude;
    double altitude;
};

struct EgoPose {
    double timestamp;
    double latitude;
    double longitude;
    double altitude;
    double heading; // heading angle in degrees
    double pitch;
    double roll;
};

class CoordTransformer {
public:
    CoordTransformer();
    ~CoordTransformer() = default;

    // 解析自车 GPS/姿态轨迹数据
    bool parseEgoTrajectory(const std::string& filepath, ReferencePoint& ref, std::vector<EgoPose>& ego_path);

    // 执行三维大地坐标变换与持久化保存
    bool transformAndSave(const std::vector<TrackedObject>& tracks, 
                          const std::vector<EgoPose>& ego_path, 
                          const ReferencePoint& ref_point, 
                          const std::string& output_filepath);
};

}
```

## ⚙️ Implementation & Algorithms / 实现原理与细节
1. **逆透视投影 (Inverse Perspective Mapping, IPM)**：
   * 采用前视相机透视几何模型，读取检测框的底边中心像素点 `(u, v)`（即车轮接地接触点）。
   * 结合相机安装俯仰角、高度以及焦距标定参数，将像素反投到 3D 相机参考系 `[X_cam, Y_cam, Z_cam]`，解算目标在车体前方的径向距离与横向位移。
2. **三轴姿态三维旋转 (Rotate Vector to ENU)**：
   * 将相机系下的 3D 向量通过旋转矩阵平移转换为以自车重心为原点的局部坐标。
   * 获取当前时刻自车的 Roll（横滚）、Pitch（俯仰）、Yaw/Heading（偏航）姿态角。
   * 构造标准三维欧拉角旋转变换矩阵 $\mathbf{R} = \mathbf{R}_z(\psi)\mathbf{R}_y(\theta)\mathbf{R}_x(\phi)$，将三维向量旋转对齐至局部东北天（ENU）坐标系：
     $$\begin{bmatrix} dx \\ dy \\ dz \end{bmatrix}_{ENU} = \mathbf{R} \begin{bmatrix} X_cam \\ Z_cam \\ Y_cam \end{bmatrix}$$
3. **ENU 至绝对经纬度转换 (ENU to WGS-84)**：
   * 以自车当前 GPS 作为零偏原点（Reference Point）。
   * 基于地球椭球体曲率半径模型，计算出单位米对应的纬度与经度偏差量：
     $$Lat = Lat_{ref} + \frac{dy}{R_M}$$
     $$Lon = Lon_{ref} + \frac{dx}{R_N \cos(Lat_{ref})}$$
   * 从而解算出所有被追踪目标在每个时间戳下的高精度绝对 GPS 轨迹。
