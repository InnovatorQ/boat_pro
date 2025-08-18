# MQTT接口数据接收详细指南

## 概述

本文档详细说明如何通过MQTT接口接收外界发来的数据，包括数据接收机制、回调设置、消息解析和实际应用示例。

## 🔄 数据接收机制

### 1. MQTT消息接收流程

```
外部系统发布消息 → MQTT Broker → 订阅主题 → 消息回调 → 数据解析 → 业务处理
```

### 2. 核心接收组件

#### A. 消息接收回调链
```cpp
// MQTT底层回调 → 消息处理器 → 类型解析器 → 用户回调
mosquitto_message_callback → onMessage() → handleMessage() → user_callback()
```

#### B. 主要接收方法
- `onMessage()`: MQTT底层消息接收回调
- `handleMessage()`: 消息分发和解析
- `parseBoatState()`: 船只状态消息解析
- `parseCollisionAlert()`: 碰撞告警消息解析
- `parseSystemConfig()`: 系统配置消息解析

## 📡 接收配置设置

### 3. 基本接收配置

```cpp
#include "mqtt_communicator.h"

// 创建MQTT通信器
MQTTConfig config;
config.broker_host = "localhost";
config.broker_port = 1883;
config.client_id = "data_receiver";

MQTTCommunicator mqtt(config);

// 初始化并连接
mqtt.initialize();
mqtt.connect();
```

### 4. 订阅主题设置

#### A. 订阅所有相关主题
```cpp
// 自动订阅所有预定义主题
mqtt.subscribeAllTopics();

// 等效于以下单独订阅：
mqtt.subscribe("boat_pro/boat_state/+");        // 所有船只状态
mqtt.subscribe("boat_pro/collision_alert/+");   // 所有碰撞告警
mqtt.subscribe("boat_pro/system_config");       // 系统配置
mqtt.subscribe("boat_pro/fleet_command/+");     // 舰队命令
mqtt.subscribe("boat_pro/dock_info");           // 船坞信息
mqtt.subscribe("boat_pro/route_info");          // 航线信息
mqtt.subscribe("boat_pro/heartbeat/+");         // 心跳消息
```

#### B. 选择性订阅
```cpp
// 只订阅特定船只的状态
mqtt.subscribe("boat_pro/boat_state/1", MQTTQoS::AT_LEAST_ONCE);
mqtt.subscribe("boat_pro/boat_state/2", MQTTQoS::AT_LEAST_ONCE);

// 只订阅碰撞告警
mqtt.subscribe("boat_pro/collision_alert/+", MQTTQoS::AT_LEAST_ONCE);

// 订阅系统配置更新
mqtt.subscribe("boat_pro/system_config", MQTTQoS::EXACTLY_ONCE);
```

## 🎯 回调函数设置

### 5. 设置数据接收回调

#### A. 船只状态数据接收
```cpp
mqtt.setBoatStateCallback([](const BoatState& boat) {
    std::cout << "接收到船只状态数据:" << std::endl;
    std::cout << "  船只ID: " << boat.sysid << std::endl;
    std::cout << "  位置: (" << boat.lat << ", " << boat.lng << ")" << std::endl;
    std::cout << "  速度: " << boat.speed << " m/s" << std::endl;
    std::cout << "  航向: " << boat.heading << "°" << std::endl;
    std::cout << "  状态: " << static_cast<int>(boat.status) << std::endl;
    
    // 业务处理逻辑
    processBoatStateUpdate(boat);
});
```

#### B. 碰撞告警数据接收
```cpp
mqtt.setCollisionAlertCallback([](const CollisionAlert& alert) {
    std::cout << "接收到碰撞告警:" << std::endl;
    std::cout << "  船只ID: " << alert.current_boat_id << std::endl;
    std::cout << "  告警等级: " << static_cast<int>(alert.level) << std::endl;
    std::cout << "  碰撞时间: " << alert.collision_time << "秒" << std::endl;
    
    // 根据告警等级采取行动
    switch (alert.level) {
        case AlertLevel::EMERGENCY:
            handleEmergencyAlert(alert);
            break;
        case AlertLevel::WARNING:
            handleWarningAlert(alert);
            break;
        case AlertLevel::NORMAL:
            handleNormalAlert(alert);
            break;
    }
});
```

#### C. 系统配置更新接收
```cpp
mqtt.setSystemConfigCallback([](const SystemConfig& config) {
    std::cout << "接收到系统配置更新:" << std::endl;
    std::cout << "  船只长度: " << config.boat_length << "m" << std::endl;
    std::cout << "  船只宽度: " << config.boat_width << "m" << std::endl;
    std::cout << "  紧急阈值: " << config.emergency_threshold_s << "s" << std::endl;
    
    // 应用新配置
    applySystemConfig(config);
});
```

#### D. 通用消息接收
```cpp
mqtt.setMessageCallback([](const MQTTMessage& msg) {
    std::cout << "接收到MQTT消息:" << std::endl;
    std::cout << "  主题: " << msg.topic << std::endl;
    std::cout << "  载荷长度: " << msg.payload.length() << " bytes" << std::endl;
    std::cout << "  QoS: " << static_cast<int>(msg.qos) << std::endl;
    std::cout << "  时间戳: " << msg.timestamp << std::endl;
    
    // 自定义消息处理
    if (msg.topic.find("custom/") == 0) {
        handleCustomMessage(msg);
    }
});
```

## 🔧 系统配置管理

### 17. 配置文件管理

系统配置数据统一存储在`config/system_config.json`文件中，不应在测试脚本中硬编码。

#### A. 标准配置文件位置
```
boat_pro/
├── config/
│   ├── system_config.json      # 系统配置文件
│   ├── mqtt_config.json        # MQTT配置文件
│   └── communication_config.json # 通信配置文件
```

#### B. 系统配置文件格式
```json
{
    "boat": {
        "length": 0.75,              // 船只长度(米)
        "width": 0.47                // 船只宽度(米)
    },
    "emergency_threshold_s": 5,      // 紧急阈值(秒)
    "warning_threshold_s": 30,       // 警告阈值(秒)
    "max_boats": 30,                 // 最大船只数量
    "min_route_gap_m": 10           // 最小航线间距(米)
}
```

### 18. 配置管理工具

#### A. 使用配置管理工具
```bash
# 显示当前配置
./scripts/config_manager.sh show

# 验证配置文件
./scripts/config_manager.sh validate

# 发布配置到MQTT
./scripts/config_manager.sh publish

# 清除MQTT中的保留配置
./scripts/config_manager.sh clear

# 备份当前配置
./scripts/config_manager.sh backup

# 恢复配置备份
./scripts/config_manager.sh restore
```

#### B. 配置发布流程
```bash
# 1. 修改配置文件
vim config/system_config.json

# 2. 验证配置
./scripts/config_manager.sh validate

# 3. 发布到MQTT
./scripts/config_manager.sh publish

# 4. 验证接收
mosquitto_sub -h localhost -t "boat_pro/system_config" -v
```

### 19. 外部系统集成

#### A. 从配置文件读取
外部系统应该从标准配置文件读取系统配置，而不是硬编码：

```bash
# 正确方式：从配置文件读取
cat config/system_config.json | mosquitto_pub -h localhost -t "boat_pro/system_config" -s -r

# 错误方式：硬编码配置数据
mosquitto_pub -h localhost -t "boat_pro/system_config" -m '{"boat":{"length":0.8}}'
```

#### B. 配置同步
```bash
# 发送标准配置
./scripts/send_external_data.sh system_config

# 这会自动从config/system_config.json读取数据
```

## 🚀 快速开始

### 14. 运行数据接收器

#### A. 启动数据接收器
```bash
# 进入构建目录
cd boat_pro/build

# 运行数据接收器
./mqtt_data_receiver

# 或指定MQTT代理服务器
./mqtt_data_receiver mqtt.example.com 1883
```

#### B. 使用测试脚本
```bash
# 运行完整测试
./scripts/test_mqtt_reception.sh

# 发送特定类型数据
./scripts/send_external_data.sh boat_state
./scripts/send_external_data.sh collision_alert
./scripts/send_external_data.sh all
```

### 15. 实际使用示例

#### A. 外部系统发送船只状态
```bash
# 使用mosquitto_pub发送船只状态
mosquitto_pub -h localhost -t "boat_pro/boat_state/1" -m '{
  "sysid": 1,
  "timestamp": 1722325256.530,
  "lat": 30.549832,
  "lng": 114.342922,
  "heading": 90.0,
  "speed": 2.5,
  "status": 2,
  "route_direction": 1
}'
```

#### B. 外部系统发送碰撞告警
```bash
mosquitto_pub -h localhost -t "boat_pro/collision_alert/1" -m '{
  "current_boat_id": 1,
  "level": 2,
  "collision_time": 15.5,
  "collision_position": {
    "lat": 30.549832,
    "lng": 114.342922
  },
  "front_boat_ids": [2],
  "oncoming_boat_ids": [],
  "decision_advice": "减速避让"
}'
```

#### C. 外部系统发送舰队命令
```bash
mosquitto_pub -h localhost -t "boat_pro/fleet_command/1" -m '{
  "boat_id": 1,
  "command": "slow_down",
  "target_speed": 1.0,
  "timestamp": 1722325256,
  "reason": "collision_warning"
}'
```

### 16. 监控和调试

#### A. 监控所有MQTT消息
```bash
# 监控所有boat_pro相关消息
mosquitto_sub -h localhost -t "boat_pro/#" -v

# 只监控船只状态
mosquitto_sub -h localhost -t "boat_pro/boat_state/+" -v

# 只监控告警消息
mosquitto_sub -h localhost -t "boat_pro/collision_alert/+" -v
```

#### B. 查看接收器日志
数据接收器会实时显示接收到的数据：
```
🚢 接收到船只状态数据:
  船只ID: 1
  时间戳: 1722325256.530
  位置: (30.549832, 114.342922)
  航向: 90.0°
  速度: 2.5 m/s
  状态: 正常航行
  航线方向: 顺时针
✅ 船只状态已更新到系统

🚨 接收到碰撞告警:
  船只ID: 1
  告警等级: 🟡 警告
  预计碰撞时间: 15.5 秒
  决策建议: 减速避让
⚠️ 警告告警处理 - 发送减速命令
```

## 📋 支持的数据格式

### 6. 船只状态数据格式

#### 接收的JSON格式
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

#### 对应的C++结构体
```cpp
struct BoatState {
    int sysid;                    // 船只ID
    double timestamp;             // 时间戳
    double lat;                   // 纬度
    double lng;                   // 经度
    double heading;               // 航向角(度)
    double speed;                 // 速度(m/s)
    BoatStatus status;            // 状态枚举
    RouteDirection route_direction; // 航线方向枚举
};
```

### 7. 碰撞告警数据格式

#### 接收的JSON格式
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
  "current_heading": 90.0,
  "other_heading": 270.0,
  "decision_advice": "减速避让"
}
```

### 8. 系统配置数据格式

#### 接收的JSON格式
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

## 💻 完整接收示例

### 9. 数据接收器实现

```cpp
#include "mqtt_communicator.h"
#include "fleet_manager.h"
#include <iostream>
#include <thread>
#include <chrono>

class DataReceiver {
private:
    std::unique_ptr<MQTTCommunicator> mqtt_;
    std::unique_ptr<FleetManager> fleet_manager_;
    
public:
    DataReceiver(const MQTTConfig& config) {
        mqtt_ = std::make_unique<MQTTCommunicator>(config);
        
        SystemConfig sys_config = SystemConfig::getDefault();
        fleet_manager_ = std::make_unique<FleetManager>(sys_config);
        
        setupCallbacks();
    }
    
    void setupCallbacks() {
        // 连接状态回调
        mqtt_->setConnectionCallback([this](bool connected) {
            if (connected) {
                std::cout << "✓ MQTT连接成功，开始接收数据" << std::endl;
            } else {
                std::cout << "✗ MQTT连接断开" << std::endl;
            }
        });
        
        // 船只状态数据接收
        mqtt_->setBoatStateCallback([this](const BoatState& boat) {
            std::cout << "📡 接收船只状态 - ID:" << boat.sysid 
                      << " 位置:(" << boat.lat << "," << boat.lng << ")" 
                      << " 速度:" << boat.speed << "m/s" << std::endl;
            
            // 更新到舰队管理器
            fleet_manager_->updateBoatState(boat);
        });
        
        // 碰撞告警接收
        mqtt_->setCollisionAlertCallback([this](const CollisionAlert& alert) {
            std::cout << "🚨 接收碰撞告警 - 船只:" << alert.current_boat_id 
                      << " 等级:" << static_cast<int>(alert.level) 
                      << " 建议:" << alert.decision_advice << std::endl;
            
            // 处理告警
            handleAlert(alert);
        });
        
        // 系统配置更新接收
        mqtt_->setSystemConfigCallback([this](const SystemConfig& config) {
            std::cout << "⚙️ 接收配置更新 - 船只尺寸:" << config.boat_length 
                      << "x" << config.boat_width << "m" << std::endl;
            
            // 应用新配置
            applyConfig(config);
        });
        
        // 通用消息接收
        mqtt_->setMessageCallback([this](const MQTTMessage& msg) {
            std::cout << "📨 接收消息 - 主题:" << msg.topic 
                      << " 大小:" << msg.payload.length() << "bytes" << std::endl;
        });
    }
    
    bool start() {
        if (!mqtt_->initialize()) {
            std::cerr << "MQTT初始化失败" << std::endl;
            return false;
        }
        
        if (!mqtt_->connect()) {
            std::cerr << "MQTT连接失败" << std::endl;
            return false;
        }
        
        // 订阅所有相关主题
        mqtt_->subscribeAllTopics();
        
        // 启动舰队管理器
        fleet_manager_->runSafetyMonitoring();
        
        return true;
    }
    
    void stop() {
        fleet_manager_->stopSafetyMonitoring();
        mqtt_->shutdown();
    }
    
private:
    void handleAlert(const CollisionAlert& alert) {
        // 根据告警等级采取不同行动
        switch (alert.level) {
            case AlertLevel::EMERGENCY:
                std::cout << "🔴 紧急告警 - 立即采取避让措施!" << std::endl;
                // 发送紧急停船命令
                sendEmergencyStop(alert.current_boat_id);
                break;
                
            case AlertLevel::WARNING:
                std::cout << "🟡 警告告警 - 准备避让措施" << std::endl;
                // 发送减速命令
                sendSlowDown(alert.current_boat_id);
                break;
                
            case AlertLevel::NORMAL:
                std::cout << "🟢 正常状态 - 继续监控" << std::endl;
                break;
        }
    }
    
    void applyConfig(const SystemConfig& config) {
        // 重新配置舰队管理器
        fleet_manager_->updateConfig(config);
        std::cout << "✓ 配置已更新" << std::endl;
    }
    
    void sendEmergencyStop(int boat_id) {
        // 通过MQTT发送紧急停船命令
        Json::Value command;
        command["boat_id"] = boat_id;
        command["command"] = "emergency_stop";
        command["timestamp"] = std::time(nullptr);
        
        Json::StreamWriterBuilder builder;
        std::string payload = Json::writeString(builder, command);
        
        std::string topic = "boat_pro/fleet_command/" + std::to_string(boat_id);
        mqtt_->publish(topic, payload, MQTTQoS::AT_LEAST_ONCE);
    }
    
    void sendSlowDown(int boat_id) {
        // 通过MQTT发送减速命令
        Json::Value command;
        command["boat_id"] = boat_id;
        command["command"] = "slow_down";
        command["target_speed"] = 1.0;  // 目标速度1m/s
        command["timestamp"] = std::time(nullptr);
        
        Json::StreamWriterBuilder builder;
        std::string payload = Json::writeString(builder, command);
        
        std::string topic = "boat_pro/fleet_command/" + std::to_string(boat_id);
        mqtt_->publish(topic, payload, MQTTQoS::AT_LEAST_ONCE);
    }
};

// 使用示例
int main() {
    MQTTConfig config;
    config.broker_host = "localhost";
    config.broker_port = 1883;
    config.client_id = "data_receiver";
    
    DataReceiver receiver(config);
    
    if (!receiver.start()) {
        return -1;
    }
    
    std::cout << "数据接收器已启动，按Enter键退出..." << std::endl;
    std::cin.get();
    
    receiver.stop();
    return 0;
}
```

## 🧪 测试数据接收

### 10. 使用mosquitto客户端测试

#### A. 发送船只状态数据
```bash
# 发送船只1的状态数据
mosquitto_pub -h localhost -t "boat_pro/boat_state/1" -m '{
  "sysid": 1,
  "timestamp": 1722325256.530,
  "lat": 30.549832,
  "lng": 114.342922,
  "heading": 90.0,
  "speed": 2.5,
  "status": 2,
  "route_direction": 1
}'

# 发送船只2的状态数据
mosquitto_pub -h localhost -t "boat_pro/boat_state/2" -m '{
  "sysid": 2,
  "timestamp": 1722325256.530,
  "lat": 30.549900,
  "lng": 114.343000,
  "heading": 270.0,
  "speed": 1.8,
  "status": 2,
  "route_direction": 2
}'
```

#### B. 发送碰撞告警数据
```bash
mosquitto_pub -h localhost -t "boat_pro/collision_alert/1" -m '{
  "current_boat_id": 1,
  "level": 2,
  "collision_time": 15.5,
  "collision_position": {
    "lat": 30.549832,
    "lng": 114.342922
  },
  "front_boat_ids": [2],
  "oncoming_boat_ids": [],
  "decision_advice": "减速避让"
}'
```

#### C. 发送系统配置更新
```bash
mosquitto_pub -h localhost -t "boat_pro/system_config" -m '{
  "boat": {
    "length": 0.80,
    "width": 0.50
  },
  "emergency_threshold_s": 3,
  "warning_threshold_s": 25,
  "max_boats": 25,
  "min_route_gap_m": 12
}'
```

### 11. 监控接收到的数据

```bash
# 监控所有接收到的消息
mosquitto_sub -h localhost -t "boat_pro/#" -v

# 只监控船只状态
mosquitto_sub -h localhost -t "boat_pro/boat_state/+" -v

# 只监控碰撞告警
mosquitto_sub -h localhost -t "boat_pro/collision_alert/+" -v
```

## ⚠️ 错误处理和调试

### 12. 常见问题处理

#### A. 连接问题
```cpp
mqtt.setConnectionCallback([](bool connected) {
    if (!connected) {
        std::cerr << "MQTT连接失败，检查:" << std::endl;
        std::cerr << "1. mosquitto服务是否运行" << std::endl;
        std::cerr << "2. 网络连接是否正常" << std::endl;
        std::cerr << "3. 代理服务器地址是否正确" << std::endl;
    }
});
```

#### B. 数据解析错误
```cpp
mqtt.setMessageCallback([](const MQTTMessage& msg) {
    try {
        // 尝试解析JSON
        Json::Value json;
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream stream(msg.payload);
        
        if (!Json::parseFromStream(builder, stream, &json, &errors)) {
            std::cerr << "JSON解析失败: " << errors << std::endl;
            std::cerr << "原始数据: " << msg.payload << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "消息处理异常: " << e.what() << std::endl;
    }
});
```

#### C. 调试信息输出
```cpp
// 启用详细日志
mqtt.setLogCallback([](int level, const std::string& message) {
    std::cout << "MQTT Log [" << level << "]: " << message << std::endl;
});

// 显示统计信息
auto stats = mqtt.getStatistics();
std::cout << "接收消息数: " << stats.messages_received << std::endl;
std::cout << "接收字节数: " << stats.bytes_received << std::endl;
```

## 📈 性能优化

### 13. 接收性能优化

#### A. 消息处理优化
```cpp
// 使用消息队列避免阻塞
class MessageQueue {
private:
    std::queue<MQTTMessage> queue_;
    std::mutex mutex_;
    std::condition_variable cv_;
    
public:
    void push(const MQTTMessage& msg) {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.push(msg);
        cv_.notify_one();
    }
    
    MQTTMessage pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this] { return !queue_.empty(); });
        
        MQTTMessage msg = queue_.front();
        queue_.pop();
        return msg;
    }
};
```

#### B. 批量处理
```cpp
// 批量处理船只状态更新
std::vector<BoatState> batch_boats;
std::mutex batch_mutex;

mqtt.setBoatStateCallback([&](const BoatState& boat) {
    std::lock_guard<std::mutex> lock(batch_mutex);
    batch_boats.push_back(boat);
    
    // 达到批量大小时处理
    if (batch_boats.size() >= 10) {
        fleet_manager->updateBoatStates(batch_boats);
        batch_boats.clear();
    }
});
```

通过以上详细的配置和示例，您可以完全掌握如何通过MQTT接口接收外界发来的各种数据，并进行相应的业务处理。
