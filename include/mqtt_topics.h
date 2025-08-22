#ifndef MQTT_TOPICS_H
#define MQTT_TOPICS_H

#include <string>

/**
 * MQTT主题定义 - 无人船作业安全预测系统
 * 
 * MPC订阅主题: BoatState, DockInfo, RouteInfo, Config
 * MPC发布主题: CollisionAlert, SafetyStatus, FleetCommand, SystemStatus, Heartbeat
 * 
 * GCS订阅主题: CollisionAlert, SafetyStatus, FleetCommand, SystemStatus, Heartbeat  
 * GCS发布主题: BoatState, DockInfo, RouteInfo, Config
 */
namespace MqttTopics {
    
    // ========== MPC 订阅主题 (GCS发布) ==========
    const std::string BOAT_STATE = "BoatState";           // 船只状态数据
    const std::string DOCK_INFO = "DockInfo";             // 船坞信息数据
    const std::string ROUTE_INFO = "RouteInfo";           // 航线信息数据
    const std::string CONFIG = "Config";                  // 系统配置数据
    
    // ========== MPC 发布主题 (GCS订阅) ==========
    const std::string COLLISION_ALERT = "CollisionAlert"; // 碰撞告警
    const std::string SAFETY_STATUS = "SafetyStatus";     // 安全状态
    const std::string FLEET_COMMAND = "FleetCommand";     // 舰队命令
    const std::string SYSTEM_STATUS = "SystemStatus";     // 系统状态
    const std::string HEARTBEAT = "Heartbeat";            // 心跳信息
    
    // ========== 碰撞告警消息字段定义 ==========
    namespace CollisionAlertFields {
        const std::string ALERT_LEVEL = "alert_level";                    // 紧急程度 (1=正常, 2=警告, 3=紧急)
        const std::string AVOIDANCE_DECISION = "avoidance_decision";      // 避碰决策建议
        const std::string ALERT_BOAT_ID = "alert_boat_id";                // 告警船只ID
        const std::string COLLISION_POSITION = "collision_position";       // 碰撞位置
        const std::string COLLISION_LAT = "lat";                          // 碰撞位置纬度
        const std::string COLLISION_LNG = "lng";                          // 碰撞位置经度
        const std::string COLLISION_TIME = "collision_time";              // 预计碰撞时间(秒)
        const std::string ONCOMING_INFO = "oncoming_collision_info";      // 对向碰撞信息
        const std::string BOAT1_HEADING = "boat1_heading";                // 船只1航向(度,0为正北)
        const std::string BOAT2_HEADING = "boat2_heading";                // 船只2航向(度,0为正北)
        const std::string TIMESTAMP = "timestamp";                        // 时间戳
    }
    
    // ========== 告警等级定义 ==========
    enum class AlertLevel {
        NORMAL = 1,     // 正常状态
        WARNING = 2,    // 警告状态
        EMERGENCY = 3   // 紧急状态
    };
    
    // ========== 辅助函数 ==========
    
    /**
     * 获取告警等级描述
     * @param level 告警等级
     * @return 等级描述字符串
     */
    inline std::string getAlertLevelDescription(AlertLevel level) {
        switch (level) {
            case AlertLevel::NORMAL: return "正常";
            case AlertLevel::WARNING: return "警告";
            case AlertLevel::EMERGENCY: return "紧急";
            default: return "未知";
        }
    }
    
    /**
     * 验证主题名称是否为MPC订阅主题
     * @param topic 主题名称
     * @return 是否为MPC订阅主题
     */
    inline bool isMPCSubscribeTopic(const std::string& topic) {
        return topic == BOAT_STATE || topic == DOCK_INFO || 
               topic == ROUTE_INFO || topic == CONFIG;
    }
    
    /**
     * 验证主题名称是否为MPC发布主题
     * @param topic 主题名称
     * @return 是否为MPC发布主题
     */
    inline bool isMPCPublishTopic(const std::string& topic) {
        return topic == COLLISION_ALERT || topic == SAFETY_STATUS || 
               topic == FLEET_COMMAND || topic == SYSTEM_STATUS || 
               topic == HEARTBEAT;
    }
    
    /**
     * 验证主题名称是否为GCS订阅主题
     * @param topic 主题名称
     * @return 是否为GCS订阅主题
     */
    inline bool isGCSSubscribeTopic(const std::string& topic) {
        return isMPCPublishTopic(topic); // GCS订阅MPC发布的主题
    }
    
    /**
     * 验证主题名称是否为GCS发布主题
     * @param topic 主题名称
     * @return 是否为GCS发布主题
     */
    inline bool isGCSPublishTopic(const std::string& topic) {
        return isMPCSubscribeTopic(topic); // GCS发布MPC订阅的主题
    }
}

#endif // MQTT_TOPICS_H
