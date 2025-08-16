#ifndef MQTT_TOPICS_H
#define MQTT_TOPICS_H

#include <string>

/**
 * MQTT主题定义 - 无人船作业安全预测系统
 */
namespace MqttTopics {
    
    // 基础主题前缀
    const std::string BASE_TOPIC = "boat_safety";
    
    // ========== 输入主题 (系统订阅) ==========
    
    // 无人船动态数据主题
    const std::string BOAT_STATE = BASE_TOPIC + "/input/boat_state";
    const std::string BOAT_STATE_WILDCARD = BASE_TOPIC + "/input/boat_state/+"; // +为船只ID
    
    // 船坞静态数据主题
    const std::string DOCK_INFO = BASE_TOPIC + "/input/dock_info";
    
    // 航线定义数据主题
    const std::string ROUTE_INFO = BASE_TOPIC + "/input/route_info";
    
    // 系统配置主题
    const std::string SYSTEM_CONFIG = BASE_TOPIC + "/input/config";
    
    // 控制命令主题
    const std::string CONTROL_COMMAND = BASE_TOPIC + "/input/control";
    
    // ========== 输出主题 (系统发布) ==========
    
    // 碰撞告警主题
    const std::string COLLISION_ALERT = BASE_TOPIC + "/output/collision_alert";
    
    // 避碰决策建议主题
    const std::string AVOIDANCE_ADVICE = BASE_TOPIC + "/output/avoidance_advice";
    
    // 系统状态主题
    const std::string SYSTEM_STATUS = BASE_TOPIC + "/output/system_status";
    
    // 船只状态统计主题
    const std::string BOAT_STATISTICS = BASE_TOPIC + "/output/statistics";
    
    // ========== 双向通信主题 ==========
    
    // 船坞管理主题
    const std::string DOCK_MANAGEMENT = BASE_TOPIC + "/dock_management";
    const std::string DOCK_LOCK_REQUEST = DOCK_MANAGEMENT + "/lock_request";
    const std::string DOCK_UNLOCK_REQUEST = DOCK_MANAGEMENT + "/unlock_request";
    const std::string DOCK_STATUS_RESPONSE = DOCK_MANAGEMENT + "/status_response";
    
    // 航线管理主题
    const std::string ROUTE_MANAGEMENT = BASE_TOPIC + "/route_management";
    const std::string ROUTE_ASSIGNMENT = ROUTE_MANAGEMENT + "/assignment";
    const std::string ROUTE_QUERY = ROUTE_MANAGEMENT + "/query";
    
    // ========== 辅助函数 ==========
    
    /**
     * 生成特定船只的状态主题
     * @param boat_id 船只ID
     * @return 船只状态主题
     */
    inline std::string getBoatStateTopic(int boat_id) {
        return BOAT_STATE + "/" + std::to_string(boat_id);
    }
    
    /**
     * 生成特定船只的告警主题
     * @param boat_id 船只ID
     * @return 船只告警主题
     */
    inline std::string getBoatAlertTopic(int boat_id) {
        return COLLISION_ALERT + "/" + std::to_string(boat_id);
    }
    
    /**
     * 生成特定船坞的管理主题
     * @param dock_id 船坞ID
     * @return 船坞管理主题
     */
    inline std::string getDockManagementTopic(int dock_id) {
        return DOCK_MANAGEMENT + "/" + std::to_string(dock_id);
    }
    
    /**
     * 生成特定航线的管理主题
     * @param route_id 航线ID
     * @return 航线管理主题
     */
    inline std::string getRouteManagementTopic(int route_id) {
        return ROUTE_MANAGEMENT + "/" + std::to_string(route_id);
    }
}

#endif // MQTT_TOPICS_H
