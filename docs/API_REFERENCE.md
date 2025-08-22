# API接口文档

## 概述

本文档描述了无人船作业安全预测系统的核心API接口，包括C++类接口和MQTT消息接口。

## C++ API接口

### 核心类

#### CollisionDetector
碰撞检测核心类

```cpp
class CollisionDetector {
public:
    // 构造函数
    CollisionDetector(const SystemConfig& config);
    
    // 更新船只状态
    void updateBoatState(const BoatState& boat);
    
    // 检测碰撞风险
    CollisionAlert detectCollision(int boat_id);
    
    // 获取安全状态
    SafetyStatus getSafetyStatus(int boat_id);
    
    // 设置安全参数
    void setSafetyParams(const SafetyParams& params);
};
```

#### FleetManager
船队管理类

```cpp
class FleetManager {
public:
    // 构造函数
    FleetManager(const SystemConfig& config);
    
    // 添加船只
    bool addBoat(const BoatInfo& boat);
    
    // 移除船只
    bool removeBoat(int boat_id);
    
    // 获取船只列表
    std::vector<BoatInfo> getBoatList();
    
    // 更新任务配置
    void updateMissionConfig(const MissionConfig& config);
    
    // 获取船队状态
    FleetStatus getFleetStatus();
};
```

#### MQTTCommunicator
MQTT通信类

```cpp
class MQTTCommunicator {
public:
    // 构造函数
    MQTTCommunicator(const MQTTConfig& config);
    
    // 初始化连接
    bool initialize();
    
    // 连接到代理
    bool connect();
    
    // 断开连接
    void disconnect();
    
    // 发布消息
    bool publish(const std::string& topic, const std::string& message, int qos = 0);
    
    // 订阅主题
    bool subscribe(const std::string& topic, int qos = 1);
    
    // 设置消息回调
    void setMessageCallback(std::function<void(const std::string&, const std::string&)> callback);
    
    // 发布船只状态
    bool publishBoatState(const BoatState& boat);
    
    // 发布碰撞告警
    bool publishCollisionAlert(const CollisionAlert& alert);
    
    // 发布安全状态
    bool publishSafetyStatus(const SafetyStatus& status);
    
    // 发布系统状态
    bool publishSystemStatus(const SystemStatus& status);
};
```

### 数据结构

#### BoatState
船只状态结构

```cpp
struct BoatState {
    int boat_id;                    // 船只ID
    double lat;                     // 纬度
    double lng;                     // 经度
    double speed;                   // 速度 (m/s)
    double heading;                 // 航向 (度)
    int battery;                    // 电池电量 (%)
    std::string status;             // 状态 ("ACTIVE", "IDLE", "ERROR")
    uint64_t timestamp;             // 时间戳
    
    // JSON序列化
    std::string toJson() const;
    static BoatState fromJson(const std::string& json);
};
```

#### CollisionAlert
碰撞告警结构

```cpp
struct CollisionAlert {
    int boat_id;                    // 船只ID
    int alert_level;                // 告警级别 (0:正常, 1:警告, 2:紧急)
    double collision_time;          // 预计碰撞时间 (秒)
    Position collision_position;    // 预计碰撞位置
    std::vector<int> involved_boats; // 涉及的其他船只
    std::string avoidance_action;   // 避障动作
    uint64_t timestamp;             // 时间戳
    
    // JSON序列化
    std::string toJson() const;
    static CollisionAlert fromJson(const std::string& json);
};
```

#### SafetyStatus
安全状态结构

```cpp
struct SafetyStatus {
    int boat_id;                    // 船只ID
    std::string safety_level;       // 安全级别 ("SAFE", "WARNING", "DANGER")
    std::vector<std::string> risk_factors; // 风险因素
    std::string operational_status; // 运行状态
    uint64_t last_check;           // 最后检查时间
    
    // JSON序列化
    std::string toJson() const;
    static SafetyStatus fromJson(const std::string& json);
};
```

#### MissionConfig
任务配置结构

```cpp
struct MissionConfig {
    std::string mission_id;         // 任务ID
    int boat_count;                 // 船只数量
    std::string formation_type;     // 编队类型
    double safety_distance;         // 安全距离 (米)
    double max_speed;              // 最大速度 (m/s)
    OperationArea operation_area;   // 作业区域
    uint64_t timestamp;            // 时间戳
    
    // JSON序列化
    std::string toJson() const;
    static MissionConfig fromJson(const std::string& json);
};
```

#### SystemConfig
系统配置结构

```cpp
struct SystemConfig {
    struct Boat {
        double length;              // 船长 (米)
        double width;              // 船宽 (米)
    } boat;
    
    double emergency_threshold_s;   // 紧急阈值 (秒)
    double warning_threshold_s;     // 警告阈值 (秒)
    int max_boats;                 // 最大船只数
    double min_route_gap_m;        // 最小航线间距 (米)
    
    // 从文件加载
    static SystemConfig loadFromFile(const std::string& filename);
};
```

## MQTT消息接口

### MPC发布的消息

#### 船只状态消息
- **主题**: `mpc/boat_state/{boat_id}`
- **QoS**: 0
- **频率**: 1Hz
- **格式**: JSON

```json
{
  "boat_id": 1,
  "timestamp": 1640995200,
  "position": {
    "lat": 30.549832,
    "lng": 114.342922
  },
  "speed": 2.5,
  "heading": 90.0,
  "battery": 85,
  "status": "ACTIVE"
}
```

#### 碰撞告警消息
- **主题**: `mpc/collision_alert/{boat_id}`
- **QoS**: 1
- **频率**: 事件触发
- **格式**: JSON

```json
{
  "boat_id": 1,
  "alert_level": 2,
  "collision_time": 15.5,
  "collision_position": {
    "lat": 30.549832,
    "lng": 114.342922
  },
  "involved_boats": [2, 3],
  "avoidance_action": "减速避让",
  "timestamp": 1640995200
}
```

#### 安全状态消息
- **主题**: `mpc/safety_status/{boat_id}`
- **QoS**: 1
- **频率**: 0.2Hz
- **格式**: JSON

```json
{
  "boat_id": 1,
  "safety_level": "SAFE",
  "risk_factors": [],
  "operational_status": "NORMAL",
  "last_check": 1640995200
}
```

#### 系统状态消息
- **主题**: `mpc/system_status`
- **QoS**: 1
- **频率**: 0.1Hz
- **格式**: JSON

```json
{
  "system_id": "MPC_001",
  "status": "OPERATIONAL",
  "active_boats": 5,
  "cpu_usage": 45.2,
  "memory_usage": 62.1,
  "uptime": 86400,
  "timestamp": 1640995200
}
```

#### 心跳消息
- **主题**: `mpc/heartbeat/{boat_id}`
- **QoS**: 0
- **频率**: 0.1Hz
- **格式**: JSON

```json
{
  "boat_id": 1,
  "status": "ACTIVE",
  "timestamp": 1640995200
}
```

### GCS发布的消息

#### 任务配置消息
- **主题**: `gcs/mission_config`
- **QoS**: 2
- **频率**: 按需
- **格式**: JSON

```json
{
  "mission_id": "MISSION_001",
  "boat_count": 5,
  "formation_type": "LINE",
  "safety_distance": 10.0,
  "max_speed": 3.0,
  "operation_area": {
    "center": {"lat": 30.55, "lng": 114.34},
    "radius": 500
  },
  "timestamp": 1640995200
}
```

#### 航线规划消息
- **主题**: `gcs/route_plan/{route_id}`
- **QoS**: 2
- **频率**: 按需
- **格式**: JSON

```json
{
  "route_id": "ROUTE_001",
  "waypoints": [
    {"lat": 30.549832, "lng": 114.342922, "speed": 2.0},
    {"lat": 30.550832, "lng": 114.343922, "speed": 2.5}
  ],
  "route_type": "CLOCKWISE",
  "priority": 1,
  "timestamp": 1640995200
}
```

#### 安全参数消息
- **主题**: `gcs/safety_params`
- **QoS**: 2
- **频率**: 按需
- **格式**: JSON

```json
{
  "emergency_threshold_s": 5,
  "warning_threshold_s": 30,
  "min_safe_distance": 5.0,
  "max_collision_risk": 0.8,
  "auto_avoidance_enabled": true,
  "timestamp": 1640995200
}
```

#### 紧急接管消息
- **主题**: `gcs/emergency_override`
- **QoS**: 2
- **频率**: 紧急情况
- **格式**: JSON

```json
{
  "override_id": "EMERGENCY_001",
  "boat_ids": [1, 2, 3],
  "action": "STOP_ALL",
  "reason": "人工接管",
  "operator": "OPERATOR_001",
  "timestamp": 1640995200
}
```

## 使用示例

### C++ API使用示例

#### 基础碰撞检测
```cpp
#include "collision_detector.h"
#include "mqtt_communicator.h"

int main() {
    // 加载系统配置
    SystemConfig config = SystemConfig::loadFromFile("config/system_config.json");
    
    // 创建碰撞检测器
    CollisionDetector detector(config);
    
    // 创建MQTT通信器
    MQTTConfig mqtt_config;
    mqtt_config.broker_host = "127.0.0.1";
    mqtt_config.broker_port = 1883;
    MQTTCommunicator mqtt(mqtt_config);
    
    // 初始化MQTT连接
    if (!mqtt.initialize() || !mqtt.connect()) {
        std::cerr << "MQTT连接失败" << std::endl;
        return -1;
    }
    
    // 设置消息回调
    mqtt.setMessageCallback([&](const std::string& topic, const std::string& message) {
        if (topic.find("mpc/boat_state/") == 0) {
            BoatState boat = BoatState::fromJson(message);
            detector.updateBoatState(boat);
            
            // 检测碰撞
            CollisionAlert alert = detector.detectCollision(boat.boat_id);
            if (alert.alert_level > 0) {
                mqtt.publishCollisionAlert(alert);
            }
        }
    });
    
    // 订阅船只状态
    mqtt.subscribe("mpc/boat_state/+");
    
    // 主循环
    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return 0;
}
```

#### MQTT消息发布
```cpp
// 发布船只状态
BoatState boat;
boat.boat_id = 1;
boat.lat = 30.549832;
boat.lng = 114.342922;
boat.speed = 2.5;
boat.heading = 90.0;
boat.battery = 85;
boat.status = "ACTIVE";
boat.timestamp = time(nullptr);

mqtt.publishBoatState(boat);

// 发布碰撞告警
CollisionAlert alert;
alert.boat_id = 1;
alert.alert_level = 2;
alert.collision_time = 15.5;
alert.collision_position = {30.549832, 114.342922};
alert.involved_boats = {2, 3};
alert.avoidance_action = "减速避让";
alert.timestamp = time(nullptr);

mqtt.publishCollisionAlert(alert);
```

### 命令行工具使用

#### MQTT消息发布
```bash
# 发布船只状态
mosquitto_pub -h 127.0.0.1 -p 1883 \
  -t "mpc/boat_state/1" \
  -m '{"boat_id":1,"lat":30.5,"lng":114.3,"speed":2.5,"heading":90,"battery":85,"status":"ACTIVE","timestamp":1640995200}'

# 发布任务配置
mosquitto_pub -h 127.0.0.1 -p 1883 \
  -t "gcs/mission_config" \
  -m '{"mission_id":"MISSION_001","boat_count":5,"formation_type":"LINE","safety_distance":10.0,"max_speed":3.0}'
```

#### MQTT消息订阅
```bash
# 监听所有MPC消息
mosquitto_sub -h 127.0.0.1 -p 1883 -t "mpc/#" -v

# 监听特定船只状态
mosquitto_sub -h 127.0.0.1 -p 1883 -t "mpc/boat_state/1" -v

# 监听所有碰撞告警
mosquitto_sub -h 127.0.0.1 -p 1883 -t "mpc/collision_alert/+" -v
```

## 错误码定义

### 系统错误码
- **0**: 成功
- **-1**: 一般错误
- **-2**: 参数错误
- **-3**: 连接错误
- **-4**: 超时错误
- **-5**: 资源不足

### MQTT错误码
- **100**: 连接成功
- **101**: 协议版本不支持
- **102**: 客户端ID无效
- **103**: 服务器不可用
- **104**: 用户名密码错误
- **105**: 未授权

### 碰撞检测错误码
- **200**: 检测成功
- **201**: 船只不存在
- **202**: 数据不足
- **203**: 算法错误

## 性能指标

### API性能
- **碰撞检测延迟**: <10ms
- **消息发布延迟**: <5ms
- **内存使用**: <100MB
- **CPU使用**: <20%

### MQTT性能
- **消息吞吐量**: >700 msg/s
- **连接建立时间**: <1s
- **重连时间**: <5s
- **消息传输延迟**: <10ms

## 版本兼容性

### API版本
- **v1.0**: 基础API接口
- **v1.1**: 增加安全状态接口
- **v1.2**: 增加系统配置接口

### MQTT协议版本
- **支持版本**: MQTT 3.1.1, MQTT 5.0
- **推荐版本**: MQTT 3.1.1

## 注意事项

### 线程安全
- 所有API接口都是线程安全的
- MQTT回调函数在独立线程中执行
- 建议使用互斥锁保护共享数据

### 内存管理
- 所有动态分配的内存都会自动释放
- JSON解析使用智能指针管理内存
- 建议定期检查内存使用情况

### 错误处理
- 所有API都有明确的返回值
- 建议检查所有函数的返回值
- 使用日志记录错误信息


