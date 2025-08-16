# 无人船作业安全预测系统 MQTT 接口文档

## 概述

本文档描述了无人船作业安全预测系统的MQTT接口实现，包括主题定义、消息格式、使用方法等。

## 系统架构

```
┌─────────────────┐    MQTT    ┌──────────────────┐    MQTT    ┌─────────────────┐
│   无人船设备      │ ◄────────► │   MQTT Broker    │ ◄────────► │  安全预测系统     │
└─────────────────┘            └──────────────────┘            └─────────────────┘
│                                                                │
│  - 发布船只状态                                                  │  - 订阅船只数据
│  - 订阅控制命令                                                  │  - 发布告警信息
│  - 订阅告警信息                                                  │  - 发布避碰建议
└────────────────────────────────────────────────────────────────┘
```

## MQTT 主题结构

### 基础主题前缀
```
boat_safety/
```

### 输入主题（系统订阅）

| 主题 | 描述 | QoS | 消息格式 |
|------|------|-----|----------|
| `boat_safety/input/boat_state/+` | 船只动态数据 | 1 | BoatState |
| `boat_safety/input/dock_info` | 船坞静态数据 | 1 | DockInfo |
| `boat_safety/input/route_info` | 航线定义数据 | 1 | RouteInfo |
| `boat_safety/input/config` | 系统配置 | 2 | SystemConfig |
| `boat_safety/input/control` | 控制命令 | 1 | ControlCommand |

### 输出主题（系统发布）

| 主题 | 描述 | QoS | 消息格式 |
|------|------|-----|----------|
| `boat_safety/output/collision_alert` | 碰撞告警 | 2 | CollisionAlert |
| `boat_safety/output/collision_alert/{boat_id}` | 特定船只告警 | 2 | CollisionAlert |
| `boat_safety/output/avoidance_advice` | 避碰建议 | 1 | AvoidanceAdvice |
| `boat_safety/output/system_status` | 系统状态 | 1 | SystemStatus |
| `boat_safety/output/statistics` | 船只统计 | 0 | BoatStatistics |

### 双向通信主题

| 主题 | 描述 | QoS | 消息格式 |
|------|------|-----|----------|
| `boat_safety/dock_management/{dock_id}/lock_request` | 船坞锁定请求 | 1 | DockRequest |
| `boat_safety/dock_management/{dock_id}/unlock_request` | 船坞解锁请求 | 1 | DockRequest |
| `boat_safety/dock_management/{dock_id}/status_response` | 船坞状态响应 | 1 | DockResponse |
| `boat_safety/route_management/assignment` | 航线分配 | 1 | RouteAssignment |
| `boat_safety/route_management/query` | 航线查询 | 1 | RouteQuery |
| `boat_safety/route_management/query_response` | 航线查询响应 | 1 | RouteQueryResponse |

## 消息格式定义

### 1. 船只动态数据 (BoatState)
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

### 2. 碰撞告警 (CollisionAlert)
```json
{
  "alert_level": "emergency",
  "current_boat_id": 1,
  "target_boat_ids": [2, 3],
  "collision_position": {
    "lat": 30.549832,
    "lng": 114.342922
  },
  "collision_time_s": 15.5,
  "headings": {
    "current_boat": 90.0,
    "target_boats": [270.0, 180.0]
  },
  "timestamp": 1722325256.530,
  "message": "紧急碰撞风险"
}
```

### 3. 避碰建议 (AvoidanceAdvice)
```json
{
  "boat_id": 1,
  "advice_type": "speed_reduction",
  "recommended_speed": 1.0,
  "recommended_heading": 85.0,
  "priority": "high",
  "duration_s": 30,
  "reason": "避让入坞船只",
  "timestamp": 1722325256.530
}
```

### 4. 系统状态 (SystemStatus)
```json
{
  "status": "running",
  "active_boats": 5,
  "active_alerts": 2,
  "system_load": 0.65,
  "uptime_s": 3600,
  "version": "1.0.0",
  "timestamp": 1722325256.530
}
```

### 5. 船坞请求/响应 (DockRequest/DockResponse)
```json
// 请求
{
  "dock_id": 1,
  "boat_id": 3,
  "operation": "lock",
  "timestamp": 1722325256.530
}

// 响应
{
  "dock_id": 1,
  "boat_id": 3,
  "status": "locked",
  "success": true,
  "message": "船坞锁定成功",
  "timestamp": 1722325256.530
}
```

## 使用示例

### 1. 基本初始化

```cpp
#include "mqtt_interface.h"
#include "mqtt_message_handler.h"

// 创建MQTT接口
auto mqtt_interface = std::make_shared<MqttInterface>("localhost", 1883, "boat_system_1");

// 创建安全系统实例
auto safety_system = std::make_shared<YourSafetySystem>();

// 创建消息处理器
auto message_handler = std::make_shared<MqttMessageHandler>(mqtt_interface, safety_system);

// 初始化
mqtt_interface->initialize();
message_handler->initialize();

// 连接并启动
mqtt_interface->connect();
mqtt_interface->startLoop();
message_handler->start();
```

### 2. 发布船只状态

```cpp
Json::Value boat_state;
boat_state["sysid"] = 1;
boat_state["timestamp"] = std::time(nullptr);
boat_state["lat"] = 30.549832;
boat_state["lng"] = 114.342922;
boat_state["heading"] = 90.0;
boat_state["speed"] = 2.5;
boat_state["status"] = 2;
boat_state["route_direction"] = 1;

mqtt_interface->publish(MqttTopics::getBoatStateTopic(1), boat_state);
```

### 3. 发布碰撞告警

```cpp
Json::Value alert;
alert["alert_level"] = "emergency";
alert["current_boat_id"] = 1;
alert["target_boat_ids"] = Json::Value(Json::arrayValue);
alert["target_boat_ids"].append(2);
alert["collision_time_s"] = 15.5;
alert["timestamp"] = std::time(nullptr);

message_handler->publishCollisionAlert(alert, 1);
```

## 配置文件

系统使用JSON配置文件 `config/mqtt_config.json`：

```json
{
  "mqtt": {
    "broker": {
      "host": "localhost",
      "port": 1883,
      "client_id": "boat_safety_system"
    },
    "qos": {
      "default_publish": 1,
      "critical_messages": 2
    }
  }
}
```

## 编译和运行

### 依赖项
- libmosquitto-dev
- libjsoncpp-dev
- cmake (>= 3.10)
- gcc/g++ (支持C++17)

### 编译步骤

```bash
# 安装依赖
sudo apt-get install libmosquitto-dev libjsoncpp-dev cmake build-essential

# 编译项目
mkdir build && cd build
cmake ..
make

# 运行示例
./mqtt_example
```

### Docker 部署

```dockerfile
FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    libmosquitto-dev \
    libjsoncpp-dev \
    mosquitto \
    mosquitto-clients

COPY . /app
WORKDIR /app/build
RUN cmake .. && make

CMD ["./mqtt_example"]
```

## 错误处理

### 常见错误码

| 错误码 | 描述 | 解决方案 |
|--------|------|----------|
| MOSQ_ERR_CONN_LOST | 连接丢失 | 自动重连机制 |
| MOSQ_ERR_NO_CONN | 未连接 | 检查网络和代理状态 |
| MOSQ_ERR_PROTOCOL | 协议错误 | 检查MQTT版本兼容性 |
| MOSQ_ERR_INVAL | 参数无效 | 验证输入参数 |

### 日志级别

- ERROR: 系统错误和连接失败
- WARN: 消息格式错误和超时
- INFO: 正常操作和状态变化
- DEBUG: 详细的消息跟踪

## 性能优化

### 消息批处理
对于高频数据，可以实现消息批处理：

```cpp
// 批量发送船只状态
Json::Value batch_states(Json::arrayValue);
for (const auto& state : boat_states) {
    batch_states.append(state);
}
mqtt_interface->publish("boat_safety/input/boat_state/batch", batch_states);
```

### QoS 选择建议

- QoS 0: 统计数据、日志信息
- QoS 1: 一般状态数据、控制命令
- QoS 2: 关键告警、安全相关消息

## 安全考虑

### 认证和授权
```cpp
// 设置用户名和密码
mosquitto_username_pw_set(client, "username", "password");

// 使用TLS加密
mosquitto_tls_set(client, "ca.crt", nullptr, "client.crt", "client.key", nullptr);
```

### 主题访问控制
建议在MQTT代理中配置ACL规则，限制客户端的主题访问权限。

## 监控和调试

### MQTT 客户端工具
```bash
# 订阅所有告警消息
mosquitto_sub -h localhost -t "boat_safety/output/collision_alert/+"

# 发布测试船只状态
mosquitto_pub -h localhost -t "boat_safety/input/boat_state/1" -m '{"sysid":1,"timestamp":1722325256.530,"lat":30.549832,"lng":114.342922,"heading":90.0,"speed":2.5,"status":2,"route_direction":1}'
```

### 系统监控
系统会定期发布状态信息到 `boat_safety/output/system_status` 主题，可用于监控系统健康状态。
