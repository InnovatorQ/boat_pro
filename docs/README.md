# 无人船作业安全预测系统

## 项目概述

本项目实现了一个完整的无人船作业安全预测系统，能够实时监测船队中的碰撞风险，并提供相应的避碰决策建议。

## 功能特性

1. **多类型碰撞检测**
   - 出坞碰撞检测
   - 入坞碰撞检测  
   - 前后船碰撞检测
   - 对向航行碰撞检测

2. **优先级管理**
   - 入坞船只：最高优先级
   - 正常航行：中等优先级
   - 出坞船只：最低优先级

3. **实时告警系统**
   - 三级告警等级（正常/警告/紧急）
   - 智能决策建议生成
   - 碰撞时间和位置预测

## 系统架构

```
boat_pro/
├── include/           # 头文件
│   ├── types.h       # 数据类型定义
│   ├── geometry_utils.h  # 几何计算工具
│   ├── collision_detector.h  # 碰撞检测器
│   └── fleet_manager.h     # 船队管理器
├── src/              # 源代码实现
├── apps/             # 应用程序入口
├── config/           # 配置文件
├── tests/            # 测试代码
└── scripts/          # 构建和运行脚本
```

## 编译和运行

### 依赖项

- CMake 3.16+
- C++17 编译器 (GCC 7+ 或 Clang 6+)
- JsonCpp 库

### 安装依赖 (Ubuntu/Debian)

```bash
sudo apt-get update
sudo apt-get install cmake build-essential libjsoncpp-dev pkg-config
```

### 编译

```bash
# 克隆项目
git clone <项目地址>
cd boat_pro

# 使用构建脚本
chmod +x scripts/build.sh
./scripts/build.sh

# 或手动构建
mkdir build && cd build
cmake ..
make -j4
```

### 运行

```bash
# 运行主程序
./build/boat_pro

# 运行测试
./scripts/run_tests.sh
```

## 配置说明

系统配置文件位于 `config/system_config.json`：

```json
{
    "boat": {
        "length": 0.75,    // 船只长度(米)
        "width": 0.47      // 船只宽度(米)
    },
    "emergency_threshold_s": 5,    // 紧急告警阈值(秒)
    "warning_threshold_s": 30,     // 警告告警阈值(秒)
    "max_boats": 30,              // 最大船只数量
    "min_route_gap_m": 10         // 航线最小间距(米)
}
```

## API 使用示例

### 基本使用

```cpp
#include "fleet_manager.h"

// 创建船队管理器
boat_pro::SystemConfig config = boat_pro::SystemConfig::getDefault();
boat_pro::FleetManager fleet_manager(config);

// 设置告警回调
fleet_manager.setAlertCallback([](const boat_pro::CollisionAlert& alert) {
    std::cout << "告警: 船只 " << alert.current_boat_id 
              << " 碰撞风险等级 " << static_cast<int>(alert.level) << std::endl;
});

// 初始化船坞和航线
std::vector<boat_pro::DockInfo> docks = { /* ... */ };
std::vector<boat_pro::RouteInfo> routes = { /* ... */ };

fleet_manager.initializeDocks(docks);
fleet_manager.initializeRoutes(routes);

// 启动安全监控
fleet_manager.runSafetyMonitoring();

// 更新船只状态
boat_pro::BoatState boat;
boat.sysid = 1;
boat.lat = 30.549832;
boat.lng = 114.342922;
boat.heading = 90.0;
boat.speed = 2.5;
boat.status = boat_pro::BoatStatus::NORMAL_SAIL;
boat.route_direction = boat_pro::RouteDirection::CLOCKWISE;

fleet_manager.updateBoatState(boat);
```

### 船只状态更新

```cpp
// 创建船只状态数据
boat_pro::BoatState boat;
boat.sysid = 1;                                    // 船只ID
boat.timestamp = 1722325256.530;                   // 时间戳
boat.lat = 30.549832;                             // 纬度
boat.lng = 114.342922;                            // 经度
boat.heading = 90.0;                              // 航向(度)
boat.speed = 2.5;                                 // 速度(m/s)
boat.status = boat_pro::BoatStatus::NORMAL_SAIL;  // 状态
boat.route_direction = boat_pro::RouteDirection::CLOCKWISE; // 航线方向

// 更新到系统
fleet_manager.updateBoatState(boat);
```

### 出入坞请求处理

```cpp
// 出坞请求
bool can_undock = fleet_manager.requestUndocking(boat_id, dock_id);
if (can_undock) {
    std::cout << "船只获准出坞" << std::endl;
} else {
    std::cout << "出坞请求被拒绝，存在安全风险" << std::endl;
}

// 入坞请求
bool can_dock = fleet_manager.requestDocking(boat_id);
if (can_dock) {
    std::cout << "船只获准入坞" << std::endl;
} else {
    std::cout << "入坞请求被拒绝" << std::endl;
}
```

## 告警级别说明

系统提供三级碰撞告警：

1. **正常 (NORMAL)**
   - 无碰撞风险
   - 距离安全

2. **警告 (WARNING)**
   - 预计30秒内可能发生碰撞
   - 建议减速或调整航向

3. **紧急 (EMERGENCY)**
   - 预计5秒内发生碰撞
   - 立即停船或紧急避让

## 核心算法

### 碰撞检测算法

系统使用基于运动学的碰撞检测算法：

1. **相对运动计算**: 计算两船的相对位置和相对速度
2. **最近接近点预测**: 预测两船最近接近的时间和位置  
3. **碰撞半径判断**: 根据船只尺寸确定安全距离
4. **时间阈值分级**: 根据碰撞时间确定告警等级

### 优先级调度

系统按以下优先级处理船只：

```
入坞船只 > 正常航行船只 > 出坞船只
```

- 入坞船只具有路权优先级
- 出坞船只需要等待其他船只通过
- 同优先级船只按先到先服务原则

## 测试

### 运行单元测试

```bash
# 构建并运行测试
./scripts/run_tests.sh

# 或手动运行
./build/boat_pro_test
```

### 测试覆盖

- 几何计算工具测试
- 碰撞检测算法测试  
- 船队管理功能测试
- 优先级调度测试

## 扩展开发

### 添加新的碰撞检测类型

1. 在 `CollisionDetector` 类中添加新的检测方法
2. 在 `detectCollisions()` 中调用新方法
3. 更新告警生成逻辑

```cpp
// 添加新的检测方法
std::vector<CollisionAlert> CollisionDetector::detectNewCollisionType() {
    std::vector<CollisionAlert> alerts;
    // 实现检测逻辑
    return alerts;
}
```

### 自定义告警处理

```cpp
// 实现自定义告警回调
void customAlertHandler(const boat_pro::CollisionAlert& alert) {
    // 发送到外部系统
    // 记录日志
    // 触发自动避障
}

fleet_manager.setAlertCallback(customAlertHandler);
```

## 性能优化

- 使用空间索引优化碰撞检测
- 实现多线程并行处理
- 缓存几何计算结果
- 按需更新船只状态

## 故障排除

### 常见问题

1. **编译错误**: 检查依赖库是否正确安装
2. **告警过多**: 调整阈值参数或碰撞半径
3. **性能问题**: 减少更新频率或优化算法

### 调试选项

在编译时添加调试信息：

```bash
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```