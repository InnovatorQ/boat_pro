#include "mqtt_interface.h"
#include "mqtt_message_handler.h"
#include "mqtt_topics.h"
#include "boat_safety_system.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <json/json.h>
#include <memory>
#include <ctime>

// 简单的安全系统实现示例
class SimpleSafetySystem : public BoatSafetySystem {
public:
    void updateBoatState(const Json::Value& boat_state) override {
        std::cout << "Received boat state update for boat " << boat_state["sysid"].asInt() << std::endl;
        // 这里可以添加碰撞检测逻辑
        
        // 模拟碰撞检测结果
        if (boat_state["speed"].asDouble() > 3.0) {
            Json::Value alert;
            alert["alert_level"] = "warning";
            alert["boat_id"] = boat_state["sysid"];
            alert["message"] = "High speed detected";
            alert["timestamp"] = boat_state["timestamp"];
            
            // 通过消息处理器发布告警
            if (message_handler) {
                message_handler->publishCollisionAlert(alert, boat_state["sysid"].asInt());
            }
        }
    }
    
    void updateDockInfo(const Json::Value& dock_info) override {
        std::cout << "Received dock info update for dock " << dock_info["dock_id"].asInt() << std::endl;
    }
    
    void updateRouteInfo(const Json::Value& route_info) override {
        std::cout << "Received route info update for route " << route_info["route_id"].asInt() << std::endl;
    }
    
    void updateSystemConfig(const Json::Value& config) override {
        std::cout << "Received system config update" << std::endl;
    }
    
    void processControlCommand(const Json::Value& command) override {
        std::cout << "Received control command: " << command.get("command", "unknown").asString() << std::endl;
    }
    
    Json::Value processDockLockRequest(const Json::Value& request) override {
        Json::Value response;
        response["dock_id"] = request["dock_id"];
        response["status"] = "locked";
        response["timestamp"] = std::time(nullptr);
        std::cout << "Processed dock lock request for dock " << request["dock_id"].asInt() << std::endl;
        return response;
    }
    
    Json::Value processDockUnlockRequest(const Json::Value& request) override {
        Json::Value response;
        response["dock_id"] = request["dock_id"];
        response["status"] = "unlocked";
        response["timestamp"] = std::time(nullptr);
        std::cout << "Processed dock unlock request for dock " << request["dock_id"].asInt() << std::endl;
        return response;
    }
    
    Json::Value processRouteAssignment(const Json::Value& assignment) override {
        Json::Value response;
        response["status"] = "assigned";
        response["boat_id"] = assignment["boat_id"];
        response["route_id"] = assignment["route_id"];
        std::cout << "Processed route assignment" << std::endl;
        return response;
    }
    
    Json::Value processRouteQuery(const Json::Value& query) override {
        Json::Value response;
        response["available_routes"] = Json::Value(Json::arrayValue);
        response["available_routes"].append(1);
        response["available_routes"].append(2);
        std::cout << "Processed route query" << std::endl;
        return response;
    }
    
    void setMessageHandler(std::shared_ptr<MqttMessageHandler> handler) {
        message_handler = handler;
    }

private:
    std::shared_ptr<MqttMessageHandler> message_handler;
};

int main() {
    std::cout << "Starting MQTT Boat Safety System Example..." << std::endl;
    
    // 创建MQTT接口
    auto mqtt_interface = std::make_shared<MqttInterface>("localhost", 1883, "boat_safety_example");
    
    // 创建安全系统
    auto safety_system = std::make_shared<SimpleSafetySystem>();
    
    // 创建消息处理器
    auto message_handler = std::make_shared<MqttMessageHandler>(mqtt_interface, safety_system);
    safety_system->setMessageHandler(message_handler);
    
    // 初始化系统
    if (!mqtt_interface->initialize()) {
        std::cerr << "Failed to initialize MQTT interface" << std::endl;
        return 1;
    }
    
    if (!message_handler->initialize()) {
        std::cerr << "Failed to initialize message handler" << std::endl;
        return 1;
    }
    
    // 连接到MQTT代理
    if (!mqtt_interface->connect()) {
        std::cerr << "Failed to connect to MQTT broker" << std::endl;
        return 1;
    }
    
    // 启动消息循环和处理器
    mqtt_interface->startLoop();
    message_handler->start();
    
    // 发布系统状态
    Json::Value system_status;
    system_status["status"] = "running";
    system_status["timestamp"] = std::time(nullptr);
    system_status["version"] = "1.0.0";
    message_handler->publishSystemStatus(system_status);
    
    // 模拟发送一些测试数据
    std::thread test_thread([&]() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // 发送船只状态数据
        Json::Value boat_state;
        boat_state["sysid"] = 1;
        boat_state["timestamp"] = std::time(nullptr);
        boat_state["lat"] = 30.549832;
        boat_state["lng"] = 114.342922;
        boat_state["heading"] = 90.0;
        boat_state["speed"] = 3.5; // 高速，会触发告警
        boat_state["status"] = 2;
        boat_state["route_direction"] = 1;
        
        mqtt_interface->publish(MqttTopics::BOAT_STATE, boat_state);
        std::cout << "Published test boat state data" << std::endl;
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // 发送船坞信息
        Json::Value dock_info;
        dock_info["dock_id"] = 1;
        dock_info["lat"] = 30.549100;
        dock_info["lng"] = 114.343000;
        
        mqtt_interface->publish(MqttTopics::DOCK_INFO, dock_info);
        std::cout << "Published test dock info data" << std::endl;
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // 发送航线信息
        Json::Value route_info;
        route_info["route_id"] = 1;
        route_info["direction"] = 1;
        route_info["points"] = Json::Value(Json::arrayValue);
        
        Json::Value point1;
        point1["lat"] = 30.549500;
        point1["lng"] = 114.342800;
        route_info["points"].append(point1);
        
        Json::Value point2;
        point2["lat"] = 30.549800;
        point2["lng"] = 114.343300;
        route_info["points"].append(point2);
        
        mqtt_interface->publish(MqttTopics::ROUTE_INFO, route_info);
        std::cout << "Published test route info data" << std::endl;
    });
    
    // 运行10秒后退出
    std::cout << "System running... Press Ctrl+C to exit or wait 10 seconds" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(10));
    
    // 清理
    test_thread.join();
    message_handler->stop();
    mqtt_interface->stopLoop();
    mqtt_interface->disconnect();
    
    std::cout << "MQTT Boat Safety System Example finished" << std::endl;
    return 0;
}
