#include "mqtt_message_handler.h"
#include <iostream>
#include <regex>
#include <sstream>
#include <ctime>

MqttMessageHandler::MqttMessageHandler(std::shared_ptr<MqttInterface> mqtt_interface, 
                                      std::shared_ptr<BoatSafetySystem> safety_system)
    : m_mqtt_interface(mqtt_interface)
    , m_safety_system(safety_system)
    , m_running(false)
{
}

MqttMessageHandler::~MqttMessageHandler() {
    stop();
}

bool MqttMessageHandler::initialize() {
    if (!m_mqtt_interface || !m_safety_system) {
        std::cerr << "MQTT interface or safety system not provided" << std::endl;
        return false;
    }
    
    // 设置MQTT消息回调
    m_mqtt_interface->setMessageCallback(
        [this](const std::string& topic, const Json::Value& message) {
            onMqttMessage(topic, message);
        }
    );
    
    return true;
}

void MqttMessageHandler::start() {
    if (m_running) {
        return;
    }
    
    m_running = true;
    subscribeToTopics();
    std::cout << "MQTT message handler started" << std::endl;
}

void MqttMessageHandler::stop() {
    if (!m_running) {
        return;
    }
    
    m_running = false;
    unsubscribeFromTopics();
    std::cout << "MQTT message handler stopped" << std::endl;
}

void MqttMessageHandler::publishCollisionAlert(const Json::Value& alert_data, int boat_id) {
    std::string topic = (boat_id >= 0) ? 
        MqttTopics::getBoatAlertTopic(boat_id) : 
        MqttTopics::COLLISION_ALERT;
    
    if (!m_mqtt_interface->publish(topic, alert_data)) {
        std::cerr << "Failed to publish collision alert to topic: " << topic << std::endl;
    }
}

void MqttMessageHandler::publishAvoidanceAdvice(const Json::Value& advice_data, int boat_id) {
    std::string topic = MqttTopics::AVOIDANCE_ADVICE;
    if (boat_id >= 0) {
        topic += "/" + std::to_string(boat_id);
    }
    
    if (!m_mqtt_interface->publish(topic, advice_data)) {
        std::cerr << "Failed to publish avoidance advice to topic: " << topic << std::endl;
    }
}

void MqttMessageHandler::publishSystemStatus(const Json::Value& status_data) {
    if (!m_mqtt_interface->publish(MqttTopics::SYSTEM_STATUS, status_data)) {
        std::cerr << "Failed to publish system status" << std::endl;
    }
}

void MqttMessageHandler::publishBoatStatistics(const Json::Value& statistics_data) {
    if (!m_mqtt_interface->publish(MqttTopics::BOAT_STATISTICS, statistics_data)) {
        std::cerr << "Failed to publish boat statistics" << std::endl;
    }
}

void MqttMessageHandler::publishDockStatusResponse(int dock_id, const Json::Value& response_data) {
    std::string topic = MqttTopics::getDockManagementTopic(dock_id) + "/status_response";
    
    if (!m_mqtt_interface->publish(topic, response_data)) {
        std::cerr << "Failed to publish dock status response for dock " << dock_id << std::endl;
    }
}

void MqttMessageHandler::onMqttMessage(const std::string& topic, const Json::Value& message) {
    if (!m_running) {
        return;
    }
    
    try {
        // 根据主题路由消息
        if (topic.find(MqttTopics::BOAT_STATE) == 0) {
            handleBoatStateMessage(message);
        }
        else if (topic == MqttTopics::DOCK_INFO) {
            handleDockInfoMessage(message);
        }
        else if (topic == MqttTopics::ROUTE_INFO) {
            handleRouteInfoMessage(message);
        }
        else if (topic == MqttTopics::SYSTEM_CONFIG) {
            handleSystemConfigMessage(message);
        }
        else if (topic == MqttTopics::CONTROL_COMMAND) {
            handleControlCommandMessage(message);
        }
        else if (topic.find(MqttTopics::DOCK_LOCK_REQUEST) == 0) {
            handleDockLockRequest(message);
        }
        else if (topic.find(MqttTopics::DOCK_UNLOCK_REQUEST) == 0) {
            handleDockUnlockRequest(message);
        }
        else if (topic.find(MqttTopics::ROUTE_ASSIGNMENT) == 0) {
            handleRouteAssignmentMessage(message);
        }
        else if (topic.find(MqttTopics::ROUTE_QUERY) == 0) {
            handleRouteQueryMessage(message);
        }
        else {
            std::cout << "Unhandled topic: " << topic << std::endl;
        }
    }
    catch (const std::exception& e) {
        std::cerr << "Error processing MQTT message from topic " << topic << ": " << e.what() << std::endl;
    }
}

void MqttMessageHandler::handleBoatStateMessage(const Json::Value& message) {
    if (!validateBoatStateMessage(message)) {
        std::cerr << "Invalid boat state message format" << std::endl;
        return;
    }
    
    m_safety_system->updateBoatState(message);
    std::cout << "Updated boat state for boat ID: " << message["sysid"].asInt() << std::endl;
}

void MqttMessageHandler::handleDockInfoMessage(const Json::Value& message) {
    if (!validateDockInfoMessage(message)) {
        std::cerr << "Invalid dock info message format" << std::endl;
        return;
    }
    
    m_safety_system->updateDockInfo(message);
    std::cout << "Updated dock info for dock ID: " << message["dock_id"].asInt() << std::endl;
}

void MqttMessageHandler::handleRouteInfoMessage(const Json::Value& message) {
    if (!validateRouteInfoMessage(message)) {
        std::cerr << "Invalid route info message format" << std::endl;
        return;
    }
    
    m_safety_system->updateRouteInfo(message);
    std::cout << "Updated route info for route ID: " << message["route_id"].asInt() << std::endl;
}

void MqttMessageHandler::handleSystemConfigMessage(const Json::Value& message) {
    m_safety_system->updateSystemConfig(message);
    std::cout << "Updated system configuration" << std::endl;
}

void MqttMessageHandler::handleControlCommandMessage(const Json::Value& message) {
    m_safety_system->processControlCommand(message);
    std::cout << "Processed control command" << std::endl;
}

void MqttMessageHandler::handleDockLockRequest(const Json::Value& message) {
    Json::Value response = m_safety_system->processDockLockRequest(message);
    
    if (message.isMember("dock_id")) {
        int dock_id = message["dock_id"].asInt();
        publishDockStatusResponse(dock_id, response);
    }
}

void MqttMessageHandler::handleDockUnlockRequest(const Json::Value& message) {
    Json::Value response = m_safety_system->processDockUnlockRequest(message);
    
    if (message.isMember("dock_id")) {
        int dock_id = message["dock_id"].asInt();
        publishDockStatusResponse(dock_id, response);
    }
}

void MqttMessageHandler::handleRouteAssignmentMessage(const Json::Value& message) {
    Json::Value response = m_safety_system->processRouteAssignment(message);
    
    // 可以根据需要发布响应
    if (response.isMember("status")) {
        std::cout << "Route assignment processed: " << response["status"].asString() << std::endl;
    }
}

void MqttMessageHandler::handleRouteQueryMessage(const Json::Value& message) {
    Json::Value response = m_safety_system->processRouteQuery(message);
    
    // 发布查询响应
    std::string response_topic = MqttTopics::ROUTE_MANAGEMENT + "/query_response";
    if (!m_mqtt_interface->publish(response_topic, response)) {
        std::cerr << "Failed to publish route query response" << std::endl;
    }
}

bool MqttMessageHandler::validateBoatStateMessage(const Json::Value& message) {
    // 验证必需字段
    const std::vector<std::string> required_fields = {
        "sysid", "timestamp", "lat", "lng", "heading", "speed", "status", "route_direction"
    };
    
    for (const auto& field : required_fields) {
        if (!message.isMember(field)) {
            std::cerr << "Missing required field in boat state message: " << field << std::endl;
            return false;
        }
    }
    
    // 验证数据类型和范围
    if (!message["sysid"].isInt() || message["sysid"].asInt() <= 0) {
        std::cerr << "Invalid sysid in boat state message" << std::endl;
        return false;
    }
    
    if (!message["lat"].isDouble() || !message["lng"].isDouble()) {
        std::cerr << "Invalid coordinates in boat state message" << std::endl;
        return false;
    }
    
    if (!message["speed"].isDouble() || message["speed"].asDouble() < 0) {
        std::cerr << "Invalid speed in boat state message" << std::endl;
        return false;
    }
    
    int status = message["status"].asInt();
    if (status < 1 || status > 3) {
        std::cerr << "Invalid status in boat state message (must be 1-3)" << std::endl;
        return false;
    }
    
    int route_direction = message["route_direction"].asInt();
    if (route_direction < 1 || route_direction > 2) {
        std::cerr << "Invalid route_direction in boat state message (must be 1 or 2)" << std::endl;
        return false;
    }
    
    return true;
}

bool MqttMessageHandler::validateDockInfoMessage(const Json::Value& message) {
    const std::vector<std::string> required_fields = {"dock_id", "lat", "lng"};
    
    for (const auto& field : required_fields) {
        if (!message.isMember(field)) {
            std::cerr << "Missing required field in dock info message: " << field << std::endl;
            return false;
        }
    }
    
    if (!message["dock_id"].isInt() || message["dock_id"].asInt() <= 0) {
        std::cerr << "Invalid dock_id in dock info message" << std::endl;
        return false;
    }
    
    return true;
}

bool MqttMessageHandler::validateRouteInfoMessage(const Json::Value& message) {
    const std::vector<std::string> required_fields = {"route_id", "direction", "points"};
    
    for (const auto& field : required_fields) {
        if (!message.isMember(field)) {
            std::cerr << "Missing required field in route info message: " << field << std::endl;
            return false;
        }
    }
    
    if (!message["route_id"].isInt() || message["route_id"].asInt() <= 0) {
        std::cerr << "Invalid route_id in route info message" << std::endl;
        return false;
    }
    
    int direction = message["direction"].asInt();
    if (direction < 1 || direction > 2) {
        std::cerr << "Invalid direction in route info message (must be 1 or 2)" << std::endl;
        return false;
    }
    
    if (!message["points"].isArray() || message["points"].size() < 2) {
        std::cerr << "Invalid points array in route info message (must have at least 2 points)" << std::endl;
        return false;
    }
    
    return true;
}

int MqttMessageHandler::extractBoatIdFromTopic(const std::string& topic) {
    std::regex boat_id_regex(R"(/boat_state/(\d+))");
    std::smatch match;
    
    if (std::regex_search(topic, match, boat_id_regex)) {
        return std::stoi(match[1].str());
    }
    
    return -1;
}

int MqttMessageHandler::extractDockIdFromTopic(const std::string& topic) {
    std::regex dock_id_regex(R"(/dock_management/(\d+))");
    std::smatch match;
    
    if (std::regex_search(topic, match, dock_id_regex)) {
        return std::stoi(match[1].str());
    }
    
    return -1;
}

void MqttMessageHandler::subscribeToTopics() {
    // 订阅输入主题
    m_mqtt_interface->subscribe(MqttTopics::BOAT_STATE_WILDCARD);
    m_mqtt_interface->subscribe(MqttTopics::DOCK_INFO);
    m_mqtt_interface->subscribe(MqttTopics::ROUTE_INFO);
    m_mqtt_interface->subscribe(MqttTopics::SYSTEM_CONFIG);
    m_mqtt_interface->subscribe(MqttTopics::CONTROL_COMMAND);
    
    // 订阅船坞管理主题
    m_mqtt_interface->subscribe(MqttTopics::DOCK_LOCK_REQUEST + "/+");
    m_mqtt_interface->subscribe(MqttTopics::DOCK_UNLOCK_REQUEST + "/+");
    
    // 订阅航线管理主题
    m_mqtt_interface->subscribe(MqttTopics::ROUTE_ASSIGNMENT);
    m_mqtt_interface->subscribe(MqttTopics::ROUTE_QUERY);
    
    std::cout << "Subscribed to all MQTT topics" << std::endl;
}

void MqttMessageHandler::unsubscribeFromTopics() {
    // 取消订阅所有主题
    m_mqtt_interface->unsubscribe(MqttTopics::BOAT_STATE_WILDCARD);
    m_mqtt_interface->unsubscribe(MqttTopics::DOCK_INFO);
    m_mqtt_interface->unsubscribe(MqttTopics::ROUTE_INFO);
    m_mqtt_interface->unsubscribe(MqttTopics::SYSTEM_CONFIG);
    m_mqtt_interface->unsubscribe(MqttTopics::CONTROL_COMMAND);
    m_mqtt_interface->unsubscribe(MqttTopics::DOCK_LOCK_REQUEST + "/+");
    m_mqtt_interface->unsubscribe(MqttTopics::DOCK_UNLOCK_REQUEST + "/+");
    m_mqtt_interface->unsubscribe(MqttTopics::ROUTE_ASSIGNMENT);
    m_mqtt_interface->unsubscribe(MqttTopics::ROUTE_QUERY);
    
    std::cout << "Unsubscribed from all MQTT topics" << std::endl;
}
