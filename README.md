/# boat_pro
无人船作业安全预测系统

## 系统概述

无人船作业安全预测系统是一个专为无人船集群设计的智能安全预测系统，专注于提供实时碰撞预测、安全告警和避碰决策建议。

**系统职责定位**：
- **MPC**: 专注于安全预测和告警，不发送管理命令
- **输出内容**: 提供碰撞告警和避碰决策建议
- **数据格式**: 支持JSON格式数据交换

## 核心功能

### 1. 碰撞预测功能
- **出坞碰撞预测**: 无人船驶出船坞时的碰撞风险评估
- **入坞碰撞预测**: 无人船驶入船坞时的碰撞安全预测
- **航线碰撞预测**: 按规划航线运行时前后船碰撞预测
- **对向碰撞预测**: 对向航行时的碰撞安全预测

### 2. 优先级管理
- **驶入船坞**: 最高优先级
- **正常航行**: 中等优先级  
- **驶出船坞**: 最低优先级

### 3. 告警系统
- **紧急级别**: 碰撞距离 < 速度×5秒，预留5秒停船时间
- **警告级别**: 碰撞距离 < 速度×30秒，预留30秒反应时间
- **正常级别**: 碰撞距离 > 速度×30秒，安全状态

### 4. 避碰决策系统
- **决策枚举**: 包含84种标准化避碰决策
- **中文描述**: 每种决策都有对应的中文说明
- **分级决策**: 根据告警等级自动选择合适的避碰策略

## 应用场景

- **船只规格**: 75cm×47cm无人船（可配置）
- **集群规模**: 支持最多30条无人船
- **安全间距**: 基于速度和时间的动态安全距离计算
- **水域环境**: 静水或低流速水域

## 技术架构

### 核心模块
- **CollisionDetector**: 碰撞检测算法
- **FleetManager**: 舰队管理和协调
- **AvoidanceDecisionTypes**: 避碰决策枚举系统
- **GeometryUtils**: 地理计算工具
- **Types**: 数据类型定义

### 数据格式
- **统一格式**: JSON格式数据交换
- **统一单位**: 时间(秒)、坐标(WGS84)、速度(m/s)、角度(度)
- **可扩展性**: 所有数据结构支持字段扩展

## 快速开始

### 1. 系统要求
```bash
# Ubuntu/Debian 系统
sudo apt-get update
sudo apt-get install build-essential cmake git

# 安装依赖库
sudo apt-get install libjsoncpp-dev
```


### 2. 编译项目
```bash
# 创建构建目录
mkdir -p build
cd build

# 配置项目
cmake ..

# 编译项目（使用多核编译加速）
make -j$(nproc)

# 编译完成后，可执行文件位于 build 目录下
ls -la boat_collision_predictor
```

### 4. 运行程序

#### 方法一：直接运行
```bash
# 在 build 目录下运行
cd build
./boat_collision_predictor
```

#### 方法二：指定配置文件运行
```bash
# 使用自定义配置运行
cd build
./boat_collision_predictor ../config/system_config.json
```

### 5. 验证运行结果
程序运行后会：
- 加载系统配置
- 创建测试船只数据
- 执行碰撞检测算法
- 输出碰撞告警信息到控制台
- 生成结果文件 collision_results.json

### 6. 常见问题解决

#### 编译错误
```bash
# 如果缺少 jsoncpp 库
sudo apt-get install libjsoncpp-dev

# 清理重新编译
cd build
rm -rf *
cmake ..
make -j$(nproc)
```

#### 运行时错误
```bash
# 检查配置文件是否存在
ls -la config/

# 检查可执行文件权限
chmod +x build/boat_collision_predictor

# 查看详细错误信息
cd build
./boat_collision_predictor 2>&1 | tee output.log
```

## 输出数据

### 1. 碰撞告警信息
- **紧急程度**: 3级告警等级（正常/警告/紧急）
- **船只标识**: 当前船ID、前向船ID、对向船ID
- **位置信息**: 预计碰撞位置（WGS84坐标）
- **时间预测**: 预计碰撞时间（秒）
- **决策建议**: 避碰决策建议（枚举代码和中文描述）

### 2. 数据示例
```json
{
  "alert_level": 2,
  "avoidance_decision": 23,
  "current_boat_id": 1,
  "collision_time": 15.5,
  "collision_position": {
    "lat": 30.549832,
    "lng": 114.342922
  },
  "front_collision_boat_id": [],
  "oncoming_collision_boat_ids": [2, 3],
  "timestamp": 1692691200
}
```

## 配置文件

### 系统配置 (`config/system_config.json`)
```json
{
    "boat": {
        "length": 0.75,
        "width": 0.47
    },
    "emergency_threshold_s": 5,
    "warning_threshold_s": 30,
    "max_boats": 30,
    "min_route_gap_m": 10
}
```

## 项目结构

```
boat_pro/
├── src/                    # 核心源代码
│   ├── main.cpp           # 主程序入口
│   ├── collision_detector.cpp      # 碰撞检测算法
│   ├── fleet_manager.cpp           # 舰队管理协调
│   ├── avoidance_decision_types.cpp # 避碰决策枚举
│   ├── types.cpp                   # 数据类型定义
│   └── geometry_utils.cpp          # 地理计算工具
├── include/                # 头文件
│   ├── collision_detector.h
│   ├── fleet_manager.h
│   ├── avoidance_decision_types.h
│   ├── types.h
│   └── geometry_utils.h
├── config/                 # 配置文件
│   └── system_config.json        # 系统配置
├── docs/                   # 核心文档
│   ├── SYSTEM_OVERVIEW.md         # 系统概述
│   ├── API_REFERENCE.md           # API参考
│   ├── MQTT_ARCHITECTURE.md       # MQTT架构
│   └── DEPLOYMENT_GUIDE.md        # 部署指南
├── build/                  # 构建目录（自动生成）
├── README.md                       # 项目说明
└── CMakeLists.txt                  # 构建配置
```

## 文档目录

详细文档请参考 `docs/` 目录：

- **[系统概述](docs/SYSTEM_OVERVIEW.md)** - 系统架构和功能概述
- **[API参考](docs/API_REFERENCE.md)** - 详细API文档
- **[MQTT架构](docs/MQTT_ARCHITECTURE.md)** - MQTT通信架构说明
- **[部署指南](docs/DEPLOYMENT_GUIDE.md)** - 系统部署和配置指南

## 开发指南

### 构建项目
```bash
# 完整构建流程
mkdir -p build
cd build
cmake ..
make -j$(nproc)

# 或者一键构建
mkdir build && cd build && cmake .. && make -j$(nproc)

# 清理重新构建
make clean
cmake ..
make -j$(nproc)
```

### 运行和测试
```bash
# 运行主程序
cd build
./boat_collision_predictor

# 运行并保存输出
./boat_collision_predictor | tee collision_output.log

# 使用自定义配置
./boat_collision_predictor ../config/system_config.json

# 检查程序是否正常退出
echo $?  # 应该输出 0
```

### 调试模式编译
```bash
# Debug 模式编译
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# 使用 gdb 调试
gdb ./boat_collision_predictor
```

### API使用示例
```cpp
#include "fleet_manager.h"
#include "types.h"

// 创建舰队管理器
SystemConfig config = SystemConfig::getDefault();
FleetManager fleet(config);

// 设置告警回调
fleet.setAlertCallback([](const CollisionAlert& alert) {
    std::cout << "碰撞告警: 船只" << alert.current_boat_id 
              << " 告警等级: " << static_cast<int>(alert.level) << std::endl;
});

// 更新船只状态
BoatState boat;
boat.sysid = 1;
boat.lat = 30.55;
boat.lng = 114.34;
boat.speed = 2.5;
boat.heading = 90;
fleet.updateBoatState(boat);
```

### 扩展开发
- 支持自定义消息类型
- 可集成Web监控界面
- 支持移动应用接入
- 可连接云平台服务

## 测试验证

### 功能测试
```bash
# 运行主程序进行功能验证
./build/boat_collision_predictor

# 查看碰撞预测结果
cat collision_results.json
```

### 系统特性
- 实时碰撞检测和告警
- 动态场景模拟
- 多船只并发处理
- JSON格式结果输出
- 可配置的安全阈值


