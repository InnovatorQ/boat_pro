# MQTT接口集成文档

## 概述

本文档描述了无人船作业安全预测系统的MQTT接口实现，提供了基于MQTT协议的实时通信能力，支持船只状态发布、碰撞告警分发、系统配置管理等功能。

## 功能特性

### 核心功能
- **实时船只状态发布**: 支持船只位置、速度、航向等状态信息的实时发布
- **碰撞告警分发**: 自动发布碰撞告警信息到相关主题
- **系统配置管理**: 支持系统配置的发布和订阅
- **心跳监控**: 提供船只在线状态监控
- **舰队命令传输**: 支持舰队管理命令的下发

### 技术特性
- **多QoS支持**: 支持MQTT的三种服务质量等级
- **SSL/TLS加密**: 支持安全连接
- **自动重连**: 网络断开时自动重连
- **消息队列**: 内置消息队列确保消息可靠传输
- **统计监控**: 提供详细的通信统计信息

## 系统架构

```
┌─────────────────┐    MQTT     ┌─────────────────┐
│   船只客户端1    │◄──────────►│   MQTT Broker   │
└─────────────────┘             │   (mosquitto)   │
┌─────────────────┐             │                 │
│   船只客户端2    │◄──────────►│                 │
└─────────────────┘             └─────────────────┘
┌─────────────────┐                       ▲
│   船只客户端N    │◄──────────────────────┘
└─────────────────┘
┌─────────────────┐
│  监控中心客户端  │◄──────────────────────┘
└─────────────────┘
```

## 主题设计

### 主题层次结构
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

### 主题说明

#### 1. 船只状态主题 (`boat_pro/boat_state/{boat_id}`)
- **用途**: 发布船只实时状态信息
- **QoS**: 1 (至少一次)
- **保留**: 可配置
- **消息格式**: JSON格式的BoatState对象

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

#### 2. 碰撞告警主题 (`boat_pro/collision_alert/{boat_id}`)
- **用途**: 发布碰撞告警信息
- **QoS**: 1 (至少一次)
- **保留**: false (不保留)
- **消息格式**: JSON格式的CollisionAlert对象

```json
{
  "current_boat_id": 1,
  "level": 2,
  "collision_time": 15.5,
  "collision_position": {
    "lat": 30.549832,
    "lng": 114.342922
  },
  "forward_boat_ids": [2],
  "oncoming_boat_ids": [3, 4]
}
```

#### 3. 系统配置主题 (`boat_pro/system_config`)
- **用途**: 发布和订阅系统配置
- **QoS**: 1 (至少一次)
- **保留**: true (保留最新配置)
- **消息格式**: JSON格式的SystemConfig对象

#### 4. 心跳主题 (`boat_pro/heartbeat/{boat_id}`)
- **用途**: 船只在线状态监控
- **QoS**: 0 (最多一次)
- **保留**: false
- **消息格式**: 简单的心跳信息

```json
{
  "boat_id": 1,
  "timestamp": 1722325256,
  "status": "alive"
}
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

### 配置参数说明

#### Broker配置
- `host`: MQTT代理服务器地址
- `port`: MQTT代理服务器端口 (默认1883)
- `client_id`: 客户端唯一标识符
- `username/password`: 认证凭据
- `clean_session`: 是否清理会话
- `keep_alive`: 保活时间间隔(秒)
- `connect_timeout`: 连接超时时间(秒)

#### SSL配置
- `enable`: 是否启用SSL/TLS加密
- `ca_cert_file`: CA证书文件路径
- `cert_file`: 客户端证书文件路径
- `key_file`: 客户端私钥文件路径

## API使用说明

### 基本使用流程

```cpp
#include "mqtt_communicator.h"

// 1. 创建配置
MQTTConfig config;
config.broker_host = "localhost";
config.broker_port = 1883;
config.client_id = "my_boat_client";

// 2. 创建通信器
MQTTCommunicator mqtt(config);

// 3. 设置回调函数
mqtt.setBoatStateCallback([](const BoatState& boat) {
    std::cout << "收到船只状态: " << boat.sysid << std::endl;
});

mqtt.setCollisionAlertCallback([](const CollisionAlert& alert) {
    std::cout << "收到碰撞告警: " << alert.current_boat_id << std::endl;
});

// 4. 初始化并连接
if (!mqtt.initialize() || !mqtt.connect()) {
    std::cerr << "MQTT连接失败" << std::endl;
    return -1;
}

// 5. 启动发布线程
mqtt.startPublishing();

// 6. 发布消息
BoatState boat;
boat.sysid = 1;
boat.lat = 30.549832;
boat.lng = 114.342922;
mqtt.publishBoatState(boat);

// 7. 运行主循环
while (running) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// 8. 清理
mqtt.shutdown();
```

### 主要API接口

#### 连接管理
```cpp
bool initialize();                    // 初始化MQTT客户端
bool connect();                       // 连接到代理服务器
void disconnect();                    // 断开连接
void shutdown();                      // 关闭通信器
bool isConnected() const;             // 检查连接状态
```

#### 消息发布
```cpp
bool publishBoatState(const BoatState& boat);
bool publishCollisionAlert(const CollisionAlert& alert);
bool publishSystemConfig(const SystemConfig& config);
bool publishHeartbeat(int boat_id);
bool publish(const std::string& topic, const std::string& payload, 
            MQTTQoS qos = MQTTQoS::AT_LEAST_ONCE, bool retain = false);
```

#### 消息订阅
```cpp
bool subscribe(const std::string& topic, MQTTQoS qos = MQTTQoS::AT_LEAST_ONCE);
bool unsubscribe(const std::string& topic);
bool subscribeAllTopics();            // 订阅所有相关主题
```

#### 回调设置
```cpp
void setMessageCallback(MQTTMessageCallback callback);
void setConnectionCallback(MQTTConnectionCallback callback);
void setBoatStateCallback(MQTTBoatStateCallback callback);
void setCollisionAlertCallback(MQTTCollisionAlertCallback callback);
void setSystemConfigCallback(MQTTSystemConfigCallback callback);
```

## 部署指南

### 1. 安装依赖

#### Ubuntu/Debian
```bash
sudo apt-get update
sudo apt-get install libmosquitto-dev mosquitto mosquitto-clients
```

#### CentOS/RHEL
```bash
sudo yum install mosquitto-devel mosquitto
```

### 2. 启动MQTT代理服务器

```bash
# 启动mosquitto服务
sudo systemctl start mosquitto
sudo systemctl enable mosquitto

# 或者直接运行
mosquitto -v
```

### 3. 编译项目

```bash
cd boat_pro
mkdir build && cd build
cmake ..
make
```

### 4. 运行示例程序

```bash
# 运行MQTT示例程序
./mqtt_example

# 运行MQTT测试程序
./mqtt_test
```

## 测试验证

### 1. 使用mosquitto客户端工具测试

```bash
# 订阅所有船只状态
mosquitto_sub -h localhost -t "boat_pro/boat_state/+"

# 订阅碰撞告警
mosquitto_sub -h localhost -t "boat_pro/collision_alert/+"

# 发布测试消息
mosquitto_pub -h localhost -t "boat_pro/boat_state/1" -m '{"sysid":1,"lat":30.5,"lng":114.3}'
```

### 2. 监控工具

可以使用MQTT客户端工具如MQTT Explorer、MQTT.fx等进行可视化监控和调试。

## 性能优化

### 1. 消息频率控制
- 根据实际需求调整发布间隔
- 对于高频数据可以考虑批量发送

### 2. QoS选择
- 状态信息使用QoS 1确保可靠传输
- 心跳消息使用QoS 0减少开销
- 重要告警使用QoS 2确保恰好一次传输

### 3. 主题设计优化
- 使用层次化主题结构便于订阅管理
- 避免过深的主题层次
- 合理使用通配符订阅

## 故障排除

### 常见问题

1. **连接失败**
   - 检查MQTT代理服务器是否运行
   - 验证网络连接和防火墙设置
   - 确认用户名密码正确

2. **消息丢失**
   - 检查QoS设置
   - 验证网络稳定性
   - 查看客户端日志

3. **性能问题**
   - 调整发布频率
   - 优化消息大小
   - 检查网络带宽

### 日志分析

程序提供详细的日志输出，可以通过日志分析问题：

```cpp
// 启用MQTT日志
mqtt.setLogCallback([](int level, const std::string& message) {
    std::cout << "MQTT Log [" << level << "]: " << message << std::endl;
});
```

## 扩展开发

### 自定义消息类型

可以通过扩展现有接口支持自定义消息类型：

```cpp
// 自定义消息发布
bool publishCustomMessage(const std::string& topic, const Json::Value& data) {
    Json::StreamWriterBuilder builder;
    std::string payload = Json::writeString(builder, data);
    return publish(topic, payload);
}
```

### 集成其他系统

MQTT接口可以方便地与其他系统集成：
- Web监控界面
- 移动应用
- 第三方监控系统
- 数据分析平台

## 总结

MQTT接口为无人船作业安全预测系统提供了标准化、可扩展的通信能力。通过合理的主题设计和配置，可以实现高效、可靠的实时通信，满足无人船集群管理的各种需求。
