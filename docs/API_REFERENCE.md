# API 参考文档

## 核心数据类型

### CollisionAlert - 碰撞告警信息

碰撞告警的核心数据结构，包含完整的碰撞预测信息。

```cpp
struct CollisionAlert {
    AlertLevel level;              // 紧急程度 (NORMAL/WARNING/EMERGENCY)
    int current_boat_id;           // 当前船ID
    double collision_angle;        // 碰撞角度 (度，0-180°范围)
    CollisionType collision_type;  // 碰撞类型
    std::vector<int> front_boat_ids;    // 前向被碰撞船ID列表
    std::vector<int> oncoming_boat_ids; // 对向被碰撞船ID列表
    GeoPoint collision_position;   // 预计碰撞位置
    double collision_time;         // 预计碰撞时间(秒)
    double current_heading;        // 当前船航向
    double other_heading;          // 对方船航向
    AvoidanceDecision avoidance_decision; // 避碰决策枚举
    
    // 静态方法
    static double calculateCollisionAngle(double heading1, double heading2);
    static CollisionType determineCollisionType(double angle);
    static std::string collisionTypeToString(CollisionType type);
    
    // 实例方法
    Json::Value toJson() const;
    int getDecisionCode() const;
    void setDecisionCode(int code);
    std::string getDecisionDescription() const;
};
```

### CollisionType - 碰撞类型枚举

基于碰撞角度的碰撞类型分类。

```cpp
enum class CollisionType {
    OVERTAKING,      // 追尾碰撞 (0-30°)
    OBLIQUE,         // 斜向碰撞 (30-60°)
    CROSSING,        // 交叉碰撞 (60-120°)
    OBLIQUE_HEAD_ON, // 斜向对撞 (120-150°)
    HEAD_ON          // 正面对撞 (150-180°)
};
```

### BoatState - 船只状态信息

实时船只动态数据，适配C# mqtt_mpc_BoatState。

```cpp
struct BoatState {
    int sysid;                    // 船只系统ID
    double timestamp;             // 时间戳（UTC或本机时间，单位：秒）
    double lat;                   // 纬度（WGS84）
    double lng;                   // 经度（WGS84）
    double heading;               // 航向角（0°为正北，顺时针增加）
    double speed;                 // 船速，单位：米/秒
    BoatStatus status;            // 航行状态（1-出坞，2-正常航行，3-入坞）
    
    Json::Value toJson() const;
    static BoatState fromJson(const Json::Value& json);
    void loadFromJson(const Json::Value& json);
    GeoPoint getPosition() const;
};
```

### BoatStatus - 船只状态枚举

船只航行状态定义，影响优先级判断。

```cpp
enum class BoatStatus : int {
    UNDOCKING = 1,    // 出坞 (最低优先级)
    NORMAL_SAIL = 2,  // 正常航行 (中等优先级)
    DOCKING = 3       // 入坞 (最高优先级)
};
```

### DockInfo - 船坞信息

船坞静态位置数据，适配C# mqtt_mpc_DockInfo。

```cpp
struct DockInfo {
    int dock_id;                  // 船坞ID（与停靠boat sysid相同）
    double lat;                   // 纬度（WGS84）
    double lng;                   // 经度（WGS84）
    
    Json::Value toJson() const;
    static DockInfo fromJson(const Json::Value& json);
    GeoPoint getPosition() const;
};
```

### RouteInfo - 航线信息

船只规划航线数据，适配C# mqtt_mpc_RouteInfo。

```cpp
struct RouteInfo {
    int sysid;                    // 船只ID
    std::vector<GeoPoint> points; // 航线点列表（最多500个点）
    
    Json::Value toJson() const;
    static RouteInfo fromJson(const Json::Value& json);
};
```

### SystemConfig - 系统配置

系统配置参数，适配C# mqtt_mpc_SystemConfiguration。

```cpp
struct SystemConfig {
    BoatDimensions boat;          // 船只尺寸信息
    int emergency_threshold_s;    // 紧急判断时间阈值（秒）
    int warning_threshold_s;      // 警告判断时间阈值（秒）
    int max_boats;                // 最大船只数量
    int min_route_gap_m;          // 最小航线横向间距（米）
    
    Json::Value toJson() const;
    static SystemConfig fromJson(const Json::Value& json);
    void loadFromJson(const Json::Value& json);
    static SystemConfig getDefault();
};
```

## 核心类接口

### CollisionDetector - 碰撞检测器

负责各类型碰撞的检测和分析。

```cpp
class CollisionDetector {
public:
    CollisionDetector(const SystemConfig& config);
    
    // 主要接口
    std::vector<CollisionAlert> detectCollisions();
    void updateBoatStates(const std::map<int, BoatState>& states);
    void updateDockInfo(const std::map<int, DockInfo>& docks);
    void updateRouteInfo(const std::map<int, RouteInfo>& routes);
    
    // 专项检测
    std::vector<CollisionAlert> detectUndockingCollisions();
    std::vector<CollisionAlert> detectDockingCollisions();
    std::vector<CollisionAlert> detectFollowingCollisions();
    std::vector<CollisionAlert> detectOncomingCollisions();
    
private:
    AlertLevel calculateAlertLevel(double collision_time) const;
    AvoidanceDecision generateDecisionAdvice(const CollisionAlert& alert, 
                                           const BoatState& current_boat) const;
    bool isOncomingTraffic(const BoatState& boat1, const BoatState& boat2) const;
    double getCollisionRadius() const;
};
```

### MQTTCommunicator - MQTT通信器

处理与GCS的MQTT通信。

```cpp
class MQTTCommunicator {
public:
    MQTTCommunicator(const MQTTConfig& config);
    
    // 连接管理
    bool initialize();
    bool connect();
    void disconnect();
    bool isConnected() const;
    
    // 发布接口 (MPC → GCS)
    bool publishCollisionAlert(const CollisionAlert& alert);
    bool publishSafetyStatus(const Json::Value& status);
    bool publishSystemStatus(const Json::Value& status);
    
    // 订阅回调设置 (GCS → MPC)
    void setBoatStateCallback(std::function<void(const BoatState&)> callback);
    void setDockInfoCallback(std::function<void(const DockInfo&)> callback);
    void setRouteInfoCallback(std::function<void(const RouteInfo&)> callback);
    void setConfigCallback(std::function<void(const SystemConfig&)> callback);
    
    // 主题管理
    std::string generateCollisionAlertTopic(int boat_id) const;
    
private:
    void onMessage(const std::string& topic, const std::string& payload);
    void handleBoatStateMessage(const std::string& payload);
    void handleDockInfoMessage(const std::string& payload);
    void handleRouteInfoMessage(const std::string& payload);
    void handleConfigMessage(const std::string& payload);
};
```

### FleetManager - 舰队管理器

协调整个无人船集群的安全预测。

```cpp
class FleetManager {
public:
    FleetManager(const SystemConfig& config);
    
    // 主要接口
    void updateBoatState(const BoatState& boat);
    void updateDockInfo(const DockInfo& dock);
    void updateRouteInfo(const RouteInfo& route);
    void updateSystemConfig(const SystemConfig& config);
    
    std::vector<CollisionAlert> performSafetyCheck();
    Json::Value generateSafetyStatus() const;
    Json::Value generateSystemStatus() const;
    
    // 状态查询
    std::map<int, BoatState> getActiveBoats() const;
    int getActiveBoatCount() const;
    AlertLevel getOverallAlertLevel() const;
    
private:
    void cleanupInactiveBoats();
    bool isBoatActive(const BoatState& boat) const;
    void updateLastActivity(int boat_id);
};
```

## 工具类接口

### GeometryUtils - 地理计算工具

提供地理坐标和几何计算功能。

```cpp
namespace geometry {
    // 距离计算
    double calculateDistance(const GeoPoint& p1, const GeoPoint& p2);
    double calculateBearing(const GeoPoint& from, const GeoPoint& to);
    
    // 碰撞预测
    double calculateCollisionTime(const GeoPoint& pos1, const GeoPoint& vel1,
                                const GeoPoint& pos2, const GeoPoint& vel2,
                                double collision_radius);
    
    GeoPoint calculateDestination(const GeoPoint& start, double bearing, 
                                double distance);
    
    // 航线计算
    double calculateDistanceToRoute(const GeoPoint& point, 
                                  const std::vector<GeoPoint>& route);
    
    GeoPoint findNearestPointOnRoute(const GeoPoint& point,
                                   const std::vector<GeoPoint>& route);
}
```

### AvoidanceDecisionMapper - 避碰决策映射

避碰决策枚举与描述的转换工具。

```cpp
class AvoidanceDecisionMapper {
public:
    static std::string getChineseDescription(AvoidanceDecision decision);
    static std::string getEnglishDescription(AvoidanceDecision decision);
    static AvoidanceDecision fromCode(int code);
    static int toCode(AvoidanceDecision decision);
    
    // 决策分类
    static bool isEmergencyDecision(AvoidanceDecision decision);
    static bool isPriorityDecision(AvoidanceDecision decision);
    static bool isAngleBasedDecision(AvoidanceDecision decision);
};
```

## 使用示例

### 基本使用流程

```cpp
#include "fleet_manager.h"
#include "mqtt_communicator.h"

int main() {
    // 1. 初始化系统配置
    SystemConfig config = SystemConfig::getDefault();
    
    // 2. 创建核心组件
    FleetManager fleet_manager(config);
    
    MQTTConfig mqtt_config;
    mqtt_config.broker_host = "127.0.0.1";
    mqtt_config.broker_port = 2000;
    mqtt_config.username = "vEagles";
    mqtt_config.password = "123456";
    MQTTCommunicator mqtt_comm(mqtt_config);
    
    // 3. 设置MQTT回调
    mqtt_comm.setBoatStateCallback([&](const BoatState& boat) {
        fleet_manager.updateBoatState(boat);
    });
    
    mqtt_comm.setDockInfoCallback([&](const DockInfo& dock) {
        fleet_manager.updateDockInfo(dock);
    });
    
    // 4. 连接MQTT
    mqtt_comm.initialize();
    mqtt_comm.connect();
    
    // 5. 主循环
    while (true) {
        // 执行安全检查
        auto alerts = fleet_manager.performSafetyCheck();
        
        // 发布告警信息
        for (const auto& alert : alerts) {
            mqtt_comm.publishCollisionAlert(alert);
        }
        
        // 发布状态信息
        mqtt_comm.publishSafetyStatus(fleet_manager.generateSafetyStatus());
        mqtt_comm.publishSystemStatus(fleet_manager.generateSystemStatus());
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    return 0;
}
```

### 碰撞角度计算示例

```cpp
#include "types.h"

void demonstrateCollisionAngle() {
    // 计算两船碰撞角度
    double boat1_heading = 45.0;  // 东北方向
    double boat2_heading = 135.0; // 东南方向
    
    double angle = CollisionAlert::calculateCollisionAngle(boat1_heading, boat2_heading);
    CollisionType type = CollisionAlert::determineCollisionType(angle);
    std::string type_str = CollisionAlert::collisionTypeToString(type);
    
    std::cout << "碰撞角度: " << angle << "°" << std::endl;
    std::cout << "碰撞类型: " << type_str << std::endl;
    // 输出: 碰撞角度: 90°, 碰撞类型: crossing
}
```

### 自定义决策生成示例

```cpp
#include "avoidance_decision_types.h"

AvoidanceDecision generateCustomDecision(const CollisionAlert& alert, 
                                       const BoatState& current_boat) {
    // 基于碰撞角度选择决策
    if (alert.collision_type == CollisionType::HEAD_ON) {
        return AvoidanceDecision::HEAD_ON_TURN_RIGHT;
    } else if (alert.collision_type == CollisionType::CROSSING) {
        // 基于优先级决定
        if (current_boat.status == BoatStatus::UNDOCKING) {
            return AvoidanceDecision::UNDOCKING_MUST_YIELD;
        } else if (current_boat.status == BoatStatus::DOCKING) {
            return AvoidanceDecision::DOCKING_HAS_PRIORITY;
        }
    }
    
    return AvoidanceDecision::REDUCE_SPEED_AND_TURN_RIGHT;
}
```

## 配置参数

### MQTT配置

```json
{
    "broker": {
        "host": "127.0.0.1",
        "port": 2000,
        "username": "vEagles",
        "password": "123456",
        "client_id": "MPC_CLIENT_001"
    },
    "topics": {
        "boat_state": "mpc/BoatState",
        "dock_info": "mpc/DockInfo",
        "route_info": "mpc/RouteInfo",
        "config": "mpc/Config",
        "collision_alert": "mpc/CollisionAlert",
        "safety_status": "mpc/SafetyStatus",
        "system_status": "mpc/SystemStatus"
    }
}
```

### 系统配置

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
