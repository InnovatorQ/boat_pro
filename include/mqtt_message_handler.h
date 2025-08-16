#ifndef MQTT_MESSAGE_HANDLER_H
#define MQTT_MESSAGE_HANDLER_H

#include "mqtt_interface.h"
#include "mqtt_topics.h"
#include "boat_safety_system.h"
#include <json/json.h>
#include <functional>
#include <unordered_map>
#include <memory>

/**
 * MQTT消息处理器 - 处理无人船系统的MQTT消息路由和处理
 */
class MqttMessageHandler {
public:
    /**
     * 构造函数
     * @param mqtt_interface MQTT接口实例
     * @param safety_system 安全系统实例
     */
    MqttMessageHandler(std::shared_ptr<MqttInterface> mqtt_interface, 
                      std::shared_ptr<BoatSafetySystem> safety_system);
    
    /**
     * 析构函数
     */
    ~MqttMessageHandler();
    
    /**
     * 初始化消息处理器
     * @return 成功返回true，失败返回false
     */
    bool initialize();
    
    /**
     * 启动消息处理
     */
    void start();
    
    /**
     * 停止消息处理
     */
    void stop();
    
    /**
     * 发布碰撞告警
     * @param alert_data 告警数据
     * @param boat_id 船只ID（可选，用于特定船只告警）
     */
    void publishCollisionAlert(const Json::Value& alert_data, int boat_id = -1);
    
    /**
     * 发布避碰建议
     * @param advice_data 建议数据
     * @param boat_id 船只ID（可选，用于特定船只建议）
     */
    void publishAvoidanceAdvice(const Json::Value& advice_data, int boat_id = -1);
    
    /**
     * 发布系统状态
     * @param status_data 状态数据
     */
    void publishSystemStatus(const Json::Value& status_data);
    
    /**
     * 发布船只统计信息
     * @param statistics_data 统计数据
     */
    void publishBoatStatistics(const Json::Value& statistics_data);
    
    /**
     * 发布船坞状态响应
     * @param dock_id 船坞ID
     * @param response_data 响应数据
     */
    void publishDockStatusResponse(int dock_id, const Json::Value& response_data);

private:
    // MQTT接口实例
    std::shared_ptr<MqttInterface> m_mqtt_interface;
    
    // 安全系统实例
    std::shared_ptr<BoatSafetySystem> m_safety_system;
    
    // 运行状态
    bool m_running;
    
    /**
     * MQTT消息回调函数
     * @param topic 主题
     * @param message 消息内容
     */
    void onMqttMessage(const std::string& topic, const Json::Value& message);
    
    /**
     * 处理船只状态消息
     * @param message 消息内容
     */
    void handleBoatStateMessage(const Json::Value& message);
    
    /**
     * 处理船坞信息消息
     * @param message 消息内容
     */
    void handleDockInfoMessage(const Json::Value& message);
    
    /**
     * 处理航线信息消息
     * @param message 消息内容
     */
    void handleRouteInfoMessage(const Json::Value& message);
    
    /**
     * 处理系统配置消息
     * @param message 消息内容
     */
    void handleSystemConfigMessage(const Json::Value& message);
    
    /**
     * 处理控制命令消息
     * @param message 消息内容
     */
    void handleControlCommandMessage(const Json::Value& message);
    
    /**
     * 处理船坞锁定请求
     * @param message 消息内容
     */
    void handleDockLockRequest(const Json::Value& message);
    
    /**
     * 处理船坞解锁请求
     * @param message 消息内容
     */
    void handleDockUnlockRequest(const Json::Value& message);
    
    /**
     * 处理航线分配请求
     * @param message 消息内容
     */
    void handleRouteAssignmentMessage(const Json::Value& message);
    
    /**
     * 处理航线查询请求
     * @param message 消息内容
     */
    void handleRouteQueryMessage(const Json::Value& message);
    
    /**
     * 验证船只状态消息格式
     * @param message 消息内容
     * @return 有效返回true，无效返回false
     */
    bool validateBoatStateMessage(const Json::Value& message);
    
    /**
     * 验证船坞信息消息格式
     * @param message 消息内容
     * @return 有效返回true，无效返回false
     */
    bool validateDockInfoMessage(const Json::Value& message);
    
    /**
     * 验证航线信息消息格式
     * @param message 消息内容
     * @return 有效返回true，无效返回false
     */
    bool validateRouteInfoMessage(const Json::Value& message);
    
    /**
     * 从主题中提取船只ID
     * @param topic 主题字符串
     * @return 船只ID，提取失败返回-1
     */
    int extractBoatIdFromTopic(const std::string& topic);
    
    /**
     * 从主题中提取船坞ID
     * @param topic 主题字符串
     * @return 船坞ID，提取失败返回-1
     */
    int extractDockIdFromTopic(const std::string& topic);
    
    /**
     * 订阅所有必要的主题
     */
    void subscribeToTopics();
    
    /**
     * 取消订阅所有主题
     */
    void unsubscribeFromTopics();
};

#endif // MQTT_MESSAGE_HANDLER_H
