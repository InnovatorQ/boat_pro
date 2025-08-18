/# boat_pro
无人船作业安全预测系统

## 系统概述

无人船作业安全预测系统是一个专为无人船集群设计的智能安全管理系统，提供实时碰撞预测、安全告警和智能决策支持。

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

### 4. 通信接口
- **UDP通信**: 支持Drone ID和NMEA 2000协议
- **MQTT通信**: 基于MQTT协议的实时通信接口 ⭐ **新增功能**

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

## MQTT通信接口 ⭐ **新功能**

### 客户端配置
- **IP地址**: 127.0.0.1
- **端口**: 2000 (可修改)
- **账号**: vEagles
- **密码**: 123456
- **客户端ID**: 1000

### MPC客户端主题结构
```
mpc/                   # MPC客户端发布的主题
├── BoatState          # 无人船动态数据 (MPC→GCS)
├── DockInfo           # 船坞静态数据 (MPC→GCS)
├── RouteInfo          # 船线定义数据 (MPC→GCS)
└── Config             # 系统配置文件 (MPC→GCS)

gcs/                   # GCS(boat_pro)发布的主题
├── CollisionAlert     # 碰撞告警 (GCS→MPC)
├── SafetyStatus       # 安全状态 (GCS→MPC)
├── FleetCommand       # 舰队命令 (GCS→MPC)
├── SystemStatus       # 系统状态 (GCS→MPC)
└── Heartbeat          # 心跳消息 (GCS→MPC)
```

### 快速测试
```bash
# 编译MPC客户端测试程序
cd build && make mpc_client_test

# 运行MPC客户端测试程序
./mpc_client_test

# 运行MPC测试脚本
./scripts/test_mpc_client.sh

# 监听所有MPC和GCS消息
mosquitto_sub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 -t "mpc/#" -t "gcs/#" -v

# 发布MPC测试消息
mosquitto_pub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 \
  -t "mpc/BoatState" \
  -m '{"sysid":1,"lat":30.5,"lng":114.3,"speed":2.5}'
```

## 输出数据

### 1. 碰撞告警信息
- **紧急程度**: 3级告警等级（正常/警告/紧急）
- **船只标识**: 当前船ID、前向船ID、对向船ID
- **位置信息**: 预计碰撞位置（WGS84坐标）
- **时间预测**: 预计碰撞时间（秒）
- **航向信息**: 对向碰撞时的双方航向
- **决策建议**: 避碰决策建议

### 2. 数据示例
```json
{
  "current_boat_id": 1,
  "level": 2,
  "collision_time": 15.5,
  "collision_position": {
    "lat": 30.549832,
    "lng": 114.342922
  },
  "front_boat_ids": [2],
  "oncoming_boat_ids": [3, 4],
  "decision_advice": "减速避让"
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
├── src/                    # 源代码
├── include/                # 头文件
├── config/                 # 配置文件
├── examples/               # 示例程序
├── tests/                  # 测试程序
├── docs/                   # 文档目录
├── build/                  # 构建目录
└── CMakeLists.txt         # 构建配置
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

### API使用示例
```cpp
#include "mqtt_communicator.h"
#include "fleet_manager.h"

// 创建MQTT通信器
MQTTConfig config;
config.broker_host = "localhost";
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

## 测试验证

### 单元测试
```bash
# 运行所有测试
make test

# 运行特定测试
./mqtt_test
```

### 集成测试
```bash
# MQTT功能测试
./simple_mqtt_test

# 完整系统测试
./mqtt_example
```


