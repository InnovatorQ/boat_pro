# 无人船MQTT接口使用指南

## 概述

本项目已成功集成MQTT通信接口，为无人船作业安全预测系统提供了基于MQTT协议的实时通信能力。

## 新增功能

### 1. MQTT通信器 (`MQTTCommunicator`)
- 支持船只状态实时发布和订阅
- 支持碰撞告警分发
- 支持系统配置管理
- 支持心跳监控
- 支持SSL/TLS加密连接
- 自动重连机制

### 2. 主题设计
```
boat_pro/
├── boat_state/{boat_id}        # 船只状态
├── collision_alert/{boat_id}   # 碰撞告警
├── system_config               # 系统配置
├── fleet_command/{boat_id}     # 舰队命令
├── dock_info                   # 船坞信息
├── route_info                  # 航线信息
└── heartbeat/{boat_id}         # 心跳消息
```

### 3. 新增文件
- `include/mqtt_communicator.h` - MQTT通信器头文件
- `src/mqtt_communicator.cpp` - MQTT通信器实现
- `config/mqtt_config.json` - MQTT配置文件
- `examples/mqtt_example.cpp` - 完整MQTT示例程序
- `examples/simple_mqtt_test.cpp` - 简单MQTT测试程序
- `tests/test_mqtt.cpp` - MQTT功能测试程序
- `docs/MQTT_INTEGRATION.md` - 详细集成文档

## 快速开始

### 1. 安装依赖

确保已安装mosquitto库：
```bash
# Ubuntu/Debian
sudo apt-get install libmosquitto-dev mosquitto mosquitto-clients

# 启动mosquitto服务
sudo systemctl start mosquitto
sudo systemctl enable mosquitto
```

### 2. 编译项目

```bash
cd boat_pro
mkdir build && cd build
cmake ..
make
```

编译成功后会生成以下可执行文件：
- `boat_pro` - 主程序
- `mqtt_example` - 完整MQTT示例
- `simple_mqtt_test` - 简单MQTT测试

### 3. 运行测试

#### 简单测试
```bash
# 在一个终端运行测试程序
./simple_mqtt_test

# 在另一个终端监听消息
mosquitto_sub -h localhost -t "boat_pro/+"
```

#### 完整示例
```bash
# 运行完整MQTT示例
./mqtt_example
```

## 配置说明

### MQTT配置文件 (`config/mqtt_config.json`)

```json
{
    "broker": {
        "host": "localhost",
        "port": 1883,
        "client_id": "boat_pro_client",
        "username": "",
        "password": "",
        "clean_session": true,
        "keep_alive": 60,
        "connect_timeout": 30
    },
    "ssl": {
        "enable": false,
        "ca_cert_file": "",
        "cert_file": "",
        "key_file": ""
    },
    "topics": {
        "boat_state": "boat_pro/boat_state/",
        "collision_alert": "boat_pro/collision_alert/",
        "system_config": "boat_pro/system_config",
        "fleet_command": "boat_pro/fleet_command/",
        "dock_info": "boat_pro/dock_info",
        "route_info": "boat_pro/route_info",
        "heartbeat": "boat_pro/heartbeat/"
    },
    "publishing": {
        "default_qos": 1,
        "retain_messages": false,
        "publish_interval_ms": 1000
    }
}
```

## API使用示例

### 基本使用

```cpp
#include "mqtt_communicator.h"

// 创建配置
MQTTConfig config;
config.broker_host = "localhost";
config.client_id = "my_boat_client";

// 创建通信器
MQTTCommunicator mqtt(config);

// 设置回调
mqtt.setBoatStateCallback([](const BoatState& boat) {
    std::cout << "收到船只状态: " << boat.sysid << std::endl;
});

// 连接并启动
mqtt.initialize();
mqtt.connect();
mqtt.startPublishing();

// 发布船只状态
BoatState boat;
boat.sysid = 1;
boat.lat = 30.549832;
boat.lng = 114.342922;
mqtt.publishBoatState(boat);
```

## 测试验证

### 1. 使用mosquitto客户端工具

```bash
# 订阅所有船只状态
mosquitto_sub -h localhost -t "boat_pro/boat_state/+"

# 订阅碰撞告警
mosquitto_sub -h localhost -t "boat_pro/collision_alert/+"

# 发布测试消息
mosquitto_pub -h localhost -t "boat_pro/boat_state/1" \
  -m '{"sysid":1,"lat":30.5,"lng":114.3,"speed":2.5}'
```

### 2. 监控所有消息

```bash
# 监控所有boat_pro相关消息
mosquitto_sub -h localhost -t "boat_pro/#" -v
```

## 消息格式

### 船只状态消息
```json
{
  "sysid": 1,
  "timestamp": 1722325256.530,
  "lat": 30.549832,
  "lng": 114.342922,
  "heading": 90.0,
  "speed": 2.5,
  "status": 2,
  "route_direction": 1
}
```

### 碰撞告警消息
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
  "oncoming_boat_ids": [3, 4]
}
```

### 心跳消息
```json
{
  "boat_id": 1,
  "timestamp": 1722325256,
  "status": "alive"
}
```

## 故障排除

### 常见问题

1. **连接失败**
   ```bash
   # 检查mosquitto服务状态
   sudo systemctl status mosquitto
   
   # 启动mosquitto服务
   sudo systemctl start mosquitto
   ```

2. **编译错误**
   ```bash
   # 确保安装了开发库
   sudo apt-get install libmosquitto-dev
   ```

3. **权限问题**
   ```bash
   # 检查mosquitto配置文件
   sudo nano /etc/mosquitto/mosquitto.conf
   
   # 添加以下行允许匿名连接（仅用于测试）
   allow_anonymous true
   listener 1883 localhost
   ```

## 性能特性

- **高效通信**: 基于TCP的可靠消息传输
- **QoS支持**: 支持三种服务质量等级
- **自动重连**: 网络断开时自动重连
- **消息队列**: 内置队列确保消息不丢失
- **统计监控**: 提供详细的通信统计

## 扩展开发

### 自定义消息类型

```cpp
// 发布自定义消息
Json::Value custom_data;
custom_data["type"] = "custom";
custom_data["data"] = "test";

Json::StreamWriterBuilder builder;
std::string payload = Json::writeString(builder, custom_data);
mqtt.publish("boat_pro/custom/1", payload);
```

### 集成其他系统

MQTT接口可以方便地与以下系统集成：
- Web监控界面
- 移动应用
- 第三方监控系统
- 数据分析平台
- 云服务

## 总结

MQTT接口的成功集成为无人船作业安全预测系统提供了：

1. **标准化通信**: 基于MQTT标准协议
2. **实时性**: 低延迟的消息传输
3. **可靠性**: 多种QoS保证消息可靠传输
4. **可扩展性**: 易于集成其他系统和服务
5. **易用性**: 简单的API接口和配置

通过MQTT接口，系统可以实现：
- 实时船只状态监控
- 即时碰撞告警分发
- 集中化系统配置管理
- 多客户端协同工作
- 与云平台和第三方系统集成

这为无人船集群管理提供了强大的通信基础设施。
