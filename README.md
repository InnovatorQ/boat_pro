/# boat_pro
无人船作业安全预测系统

## 系统概述

无人船作业安全预测系统是一个专为无人船集群设计的智能安全预测系统，专注于提供实时碰撞预测、安全告警和避碰决策建议。系统采用先进的MQTT通信架构，MPC（模型预测控制）专门负责安全预测分析，向GCS（地面控制站）输出预测结果。

**系统职责定位**：
- **MPC**: 专注于安全预测和告警，不发送管理命令
- **输出内容**: 严格按照需求文档的拟输出数据规范

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

### 4. 碰撞角度分析
- **角度计算**: 基于两船航向线夹角，自动计算0-180°范围内的碰撞角度
- **类型分类**: 根据碰撞角度自动分类碰撞类型
  - **追尾碰撞** (0-30°): 后船追前船场景
  - **斜向碰撞** (30-60°): 两船斜向相遇
  - **交叉碰撞** (60-120°): 两船垂直或近垂直交叉
  - **斜向对撞** (120-150°): 两船大角度相向而行
  - **正面对撞** (150-180°): 两船正面相向
- **决策优化**: 基于碰撞角度和船只优先级生成精确的避让建议

### 4. 通信接口
- **UDP通信**: 支持Drone ID和NMEA 2000协议
- **MQTT通信**: 基于MQTT协议的实时通信接口，支持MPC与GCS双向通信

## 应用场景

- **船只规格**: 75cm×47cm无人船（可配置）
- **集群规模**: 支持最多30条无人船
- **航线设计**: 最多2条航线（顺时针/逆时针）
- **安全间距**: 航线间距>10米
- **水域环境**: 静水或低流速水域

## 技术架构

### 核心模块
- **CollisionDetector**: 碰撞检测算法
- **FleetManager**: 舰队管理和协调
- **UDPCommunicator**: UDP通信接口
- **MQTTCommunicator**: MQTT通信接口 ⭐ **新增**
- **GeometryUtils**: 地理计算工具

### 数据格式
- **统一格式**: JSON格式数据交换
- **统一单位**: 时间(秒)、坐标(WGS84)、速度(m/s)、角度(度)
- **可扩展性**: 所有数据结构支持字段扩展

## 快速开始

### 1. 系统要求
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install build-essential cmake
sudo apt-get install libjsoncpp-dev libmosquitto-dev mosquitto

# 启动MQTT服务
sudo systemctl start mosquitto
sudo systemctl enable mosquitto
```

### 2. 编译项目
```bash
git clone <repository>
cd boat_pro
mkdir build && cd build
cmake ..
make
```

### 3. 运行程序
```bash
# 主程序
./boat_pro

# MQTT示例程序
./mqtt_example

# 简单MQTT测试
./simple_mqtt_test
```

## MQTT通信接口

### 客户端配置
- **IP地址**: 127.0.0.1
- **端口**: 2000
- **账号**: vEagles
- **密码**: 123456
- **客户端ID**: 1000

### MQTT主题架构设计

#### MPC订阅主题 (GCS → MPC)
```
mpc/BoatState     # 船只状态数据
mpc/DockInfo      # 船坞信息数据  
mpc/RouteInfo     # 航线信息数据
mpc/Config        # 系统配置数据
```

#### MPC发布主题 (MPC → GCS)
```
mpc/CollisionAlert    # 碰撞告警信息
mpc/SafetyStatus      # 安全状态
mpc/SystemStatus      # 系统状态
```

#### GCS订阅主题 (MPC → GCS)
```
mpc/CollisionAlert    # 碰撞告警信息，包含：
                      # - alert_level: 碰撞紧急程度 (1=正常, 2=警告, 3=紧急)
                      # - avoidance_decision: 相应避碰决策建议
                      # - alert_boat_id: 告警船只ID
                      # - collision_position: 预计发生碰撞的位置(经纬度)
                      # - collision_time: 预计碰撞时间(秒)
                      # - oncoming_collision_info: 对向碰撞时两船实际航向(度,0为正北)
mpc/SafetyStatus      # 安全状态
mpc/SystemStatus      # 系统状态
```

#### GCS发布主题 (GCS → MPC)
```
mpc/BoatState     # 船只状态数据
mpc/DockInfo      # 船坞信息数据
mpc/RouteInfo     # 航线信息数据
mpc/Config        # 系统配置数据
```

**主题前缀说明**: 使用`mpc/`前缀区分MPC应用与系统中其他MQTT应用，确保主题命名空间的清晰分离。

### 碰撞告警消息格式

CollisionAlert主题的消息格式包含以下字段：

```json
{
  "alert_level": 3,                    // 碰撞紧急程度 (1=正常, 2=警告, 3=紧急)
  "avoidance_decision": "立即减速并右转避让",  // 相应避碰决策建议
  "alert_boat_id": 1,                  // 告警船只ID
  "collision_position": {              // 预计发生碰撞的位置
    "lat": 30.549832,                  // 纬度
    "lng": 114.342922                  // 经度
  },
  "collision_time": 15.5,              // 预计碰撞时间(秒)
  "oncoming_collision_info": {         // 对向碰撞时的航向信息(可选)
    "boat1_heading": 90.0,             // 船只1实际航向(度,0为正北)
    "boat2_heading": 270.0             // 船只2实际航向(度,0为正北)
  },
  "timestamp": 1692691200              // 时间戳
}
```

### 快速测试
```bash
# 使用构建脚本编译项目
./scripts/build.sh

# 运行所有测试
./scripts/run_tests.sh

# MQTT功能演示
./scripts/mqtt_demo.sh

# 快速MQTT功能检查
./scripts/mqtt_quick_check.sh

# MPC客户端测试
./scripts/test_mpc_client.sh

# 监听所有MPC发布和GCS发布的消息
mosquitto_sub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 -t "mpc/CollisionAlert" -t "mpc/SafetyStatus" -t "mpc/SystemStatus" -t "mpc/BoatState" -t "mpc/DockInfo" -t "mpc/RouteInfo" -t "mpc/Config" -v

# 发布测试消息（GCS发布船只状态）
mosquitto_pub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 \
  -t "mpc/BoatState" \
  -m '{"boat_id":1,"lat":30.55,"lng":114.34,"speed":2.5,"heading":90,"status":"ACTIVE","timestamp":'$(date +%s)'}'
```

## 输出数据

### 1. 碰撞告警信息
- **紧急程度**: 3级告警等级（正常/警告/紧急）
- **船只标识**: 当前船ID、前向船ID、对向船ID
- **碰撞角度**: 两船航向线夹角（0-180°）
- **碰撞类型**: 基于角度的碰撞分类（追尾/斜向/交叉/斜向对撞/正面对撞）
- **位置信息**: 预计碰撞位置（WGS84坐标）
- **时间预测**: 预计碰撞时间（秒）
- **航向信息**: 对向碰撞时的双方航向
- **决策建议**: 避碰决策建议（枚举代码）

### 2. 数据示例
```json
{
  "alert_level": 2,
  "avoidance_decision": 23,
  "current_boat_id": 1,
  "collision_angle": 90.0,
  "collision_type": "crossing",
  "collision_time": 15.5,
  "collision_position": {
    "lat": 30.549832,
    "lng": 114.342922
  },
  "front_collision_boat_id": null,
  "oncoming_collision_boat_ids": [2, 3],
  "oncoming_collision_info": {
    "current_boat_heading": 45.0,
    "oncoming_boats_heading": [135.0, 140.0]
  },
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

### MQTT配置 (`config/mqtt_config.json`)
```json
{
    "broker": {
        "host": "localhost",
        "port": 1883,
        "client_id": "boat_pro_client"
    },
    "topics": {
        "boat_state": "boat_pro/boat_state/",
        "collision_alert": "boat_pro/collision_alert/"
    }
}
```

## 项目结构

```
boat_pro/
├── src/                    # 核心源代码
│   ├── main.cpp           # 主程序入口
│   ├── collision_detector.cpp      # 碰撞检测算法
│   ├── fleet_manager.cpp           # 舰队管理协调
│   ├── mqtt_communicator.cpp       # MQTT通信接口
│   ├── avoidance_decision_types.cpp # 避碰决策枚举
│   ├── types.cpp                   # 数据类型定义
│   └── geometry_utils.cpp          # 地理计算工具
├── include/                # 头文件
│   ├── collision_detector.h
│   ├── fleet_manager.h
│   ├── mqtt_communicator.h
│   ├── avoidance_decision_types.h
│   ├── types.h
│   └── geometry_utils.h
├── examples/               # 功能演示程序
│   ├── mqtt_example.cpp           # MQTT通信演示
│   ├── collision_angle_test.cpp   # 碰撞角度计算测试
│   └── avoidance_decision_test.cpp # 避碰决策枚举测试
├── tests/                  # 核心测试程序
│   ├── test_collision_detector.cpp # 碰撞检测测试
│   ├── test_communication.cpp     # 通信功能测试
│   └── test_mqtt.cpp              # MQTT功能测试
├── scripts/                # 工具脚本
│   ├── build.sh                   # 构建脚本
│   ├── run_tests.sh               # 测试运行脚本
│   ├── mqtt_demo.sh               # MQTT演示脚本
│   ├── mqtt_quick_check.sh        # MQTT快速检查
│   └── test_collision_angle.sh    # 碰撞角度测试
├── config/                 # 配置文件
│   ├── mqtt_config.json          # MQTT配置
│   ├── system_config.json        # 系统配置
│   └── mosquitto_custom.conf     # MQTT服务配置
├── docs/                   # 核心文档
│   ├── SYSTEM_OVERVIEW.md         # 系统概述
│   ├── API_REFERENCE.md           # API参考
│   ├── MQTT_ARCHITECTURE.md       # MQTT架构
│   └── DEPLOYMENT_GUIDE.md        # 部署指南
├── build/                  # 构建目录（自动生成）
├── MQTT_TOPICS_DEFINITION.md      # MQTT主题定义
├── README.md                       # 项目说明
└── CMakeLists.txt                  # 构建配置
```

## 文档目录

详细文档请参考 `docs/` 目录：

- **[MQTT集成文档](docs/MQTT_INTEGRATION.md)** - MQTT接口详细技术文档
- **[MQTT使用指南](docs/MQTT_USAGE.md)** - MQTT功能使用说明
- **[MQTT集成总结](docs/MQTT_INTEGRATION_SUMMARY.md)** - MQTT集成完成总结
- **[通信协议文档](docs/COMMUNICATION.md)** - UDP通信协议说明
- **[客户端集成指南](docs/CLIENT_DATA_INTEGRATION_GUIDE.md)** - 客户端数据集成指南
- **[通信验证报告](docs/COMMUNICATION_VERIFICATION_REPORT.md)** - 通信功能验证报告
- **[实际场景测试报告](docs/REAL_SCENARIO_TEST_REPORT.md)** - 实际场景测试报告

## 开发指南

### 构建项目
```bash
# 使用构建脚本（推荐）
./scripts/build.sh

# 或手动构建
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 运行测试
```bash
# 运行所有测试
./scripts/run_tests.sh

# 运行特定测试
./build/test_collision_detector
./build/test_communication
./build/mqtt_test
```

### MQTT功能测试
```bash
# 快速检查MQTT功能
./scripts/mqtt_quick_check.sh

# 完整MQTT功能测试
./scripts/mqtt_full_test.sh

# MQTT实时通信演示
./scripts/mqtt_demo.sh
```

### API使用示例
```cpp
#include "mqtt_communicator.h"
#include "fleet_manager.h"

// 创建MQTT通信器
MQTTConfig config;
config.broker_host = "127.0.0.1";
config.broker_port = 2000;
config.username = "vEagles";
config.password = "123456";
MQTTCommunicator mqtt(config);

// 设置回调
mqtt.setBoatStateCallback([](const BoatState& boat) {
    std::cout << "收到船只状态: " << boat.sysid << std::endl;
});

// 连接并发布消息
mqtt.initialize();
mqtt.connect();
mqtt.publishBoatState(boat_state);
```

### 扩展开发
- 支持自定义消息类型
- 可集成Web监控界面
- 支持移动应用接入
- 可连接云平台服务

## 脚本工具

项目提供了一套完整的脚本工具，用于构建、测试和演示系统功能：

### 核心脚本
- **`build.sh`** - 主构建脚本，检查依赖并编译项目
- **`run_tests.sh`** - 运行所有可用的测试程序

### MQTT测试脚本
- **`mqtt_demo.sh`** - MQTT实时通信演示，展示MPC和GCS双向通信
- **`mqtt_full_test.sh`** - 完整的MQTT功能测试套件
- **`mqtt_quick_check.sh`** - 快速MQTT功能验证
- **`test_mpc_client.sh`** - MPC客户端专用测试脚本
- **`test_collision_angle.sh`** - 碰撞角度计算功能测试

### 脚本特性
- 所有脚本已配置正确的MQTT连接参数（端口2000，vEagles/123456认证）
- 使用标准的MPC/GCS主题架构
- 包含详细的状态输出和错误处理
- 支持并发测试和实时监控

### 使用建议
1. 首次使用请运行 `./scripts/build.sh` 构建项目
2. 使用 `./scripts/mqtt_quick_check.sh` 验证MQTT功能
3. 运行 `./scripts/mqtt_demo.sh` 查看完整的通信演示
4. 开发调试时使用 `./scripts/run_tests.sh` 运行单元测试

## 测试验证

### 单元测试
```bash
# 运行所有测试
./scripts/run_tests.sh

# 运行特定测试
./build/test_collision_detector
./build/test_communication
./build/mqtt_test
```

### 集成测试
```bash
# MQTT功能测试
./scripts/mqtt_quick_check.sh

# 完整系统测试
./scripts/mqtt_full_test.sh

# MPC客户端测试
./scripts/test_mpc_client.sh
```

### 实时通信演示
```bash
# MQTT实时通信演示
./scripts/mqtt_demo.sh
```


