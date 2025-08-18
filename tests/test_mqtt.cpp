// ==================== tests/test_mqtt.cpp ====================
#include "mqtt_communicator.h"
#include "types.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <fstream>
#include <jsoncpp/json/json.h>

using namespace boat_pro;
using namespace boat_pro::communication;

// 加载MQTT配置
MQTTConfig loadMQTTConfig(const std::string& config_file) {
    MQTTConfig config;
    
    std::ifstream file(config_file);
    if (!file.is_open()) {
        std::cout << "Using default MQTT configuration" << std::endl;
        return config;
    }
    
    Json::Value json;
    file >> json;
    
    if (json.isMember("broker")) {
        const auto& broker = json["broker"];
        config.broker_host = broker.get("host", "localhost").asString();
        config.broker_port = broker.get("port", 1883).asInt();
        config.client_id = broker.get("client_id", "boat_pro_client").asString();
        config.username = broker.get("username", "").asString();
        config.password = broker.get("password", "").asString();
        config.clean_session = broker.get("clean_session", true).asBool();
        config.keep_alive = broker.get("keep_alive", 60).asInt();
        config.connect_timeout = broker.get("connect_timeout", 30).asInt();
    }
    
    if (json.isMember("ssl")) {
        const auto& ssl = json["ssl"];
        config.enable_ssl = ssl.get("enable", false).asBool();
        config.ca_cert_file = ssl.get("ca_cert_file", "").asString();
        config.cert_file = ssl.get("cert_file", "").asString();
        config.key_file = ssl.get("key_file", "").asString();
    }
    
    if (json.isMember("topics")) {
        const auto& topics = json["topics"];
        
        if (topics.isMember("subscribe")) {
            const auto& sub = topics["subscribe"];
            config.topics.subscribe.boat_state = sub.get("boat_state", "dock/BoatState").asString();
            config.topics.subscribe.dock_info = sub.get("dock_info", "dock/DockInfo").asString();
            config.topics.subscribe.route_info = sub.get("route_info", "dock/RouteInfo").asString();
            config.topics.subscribe.system_config = sub.get("system_config", "dock/Config").asString();
        }
        
        if (topics.isMember("publish")) {
            const auto& pub = topics["publish"];
            config.topics.publish.collision_alert = pub.get("collision_alert", "dock/CollisionAlert").asString();
            config.topics.publish.safety_status = pub.get("safety_status", "dock/SafetyStatus").asString();
            config.topics.publish.fleet_command = pub.get("fleet_command", "dock/FleetCommand").asString();
            config.topics.publish.system_status = pub.get("system_status", "dock/SystemStatus").asString();
            config.topics.publish.heartbeat = pub.get("heartbeat", "dock/Heartbeat").asString();
        }
    }
    
    if (json.isMember("publishing")) {
        const auto& pub = json["publishing"];
        config.default_qos = static_cast<MQTTQoS>(pub.get("default_qos", 1).asInt());
        config.retain_messages = pub.get("retain_messages", false).asBool();
        config.publish_interval_ms = pub.get("publish_interval_ms", 1000).asInt();
    }
    
    return config;
}

// 创建测试船只状态
BoatState createTestBoatState(int id) {
    BoatState boat;
    boat.sysid = id;
    boat.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
    boat.lat = 30.549832 + (id * 0.001);  // 稍微偏移位置
    boat.lng = 114.342922 + (id * 0.001);
    boat.heading = 90.0 + (id * 10);      // 不同航向
    boat.speed = 2.5 + (id * 0.5);        // 不同速度
    boat.status = static_cast<BoatStatus>((id % 3) + 1);  // 循环状态
    boat.route_direction = static_cast<RouteDirection>((id % 2) + 1);
    return boat;
}

// 创建测试碰撞告警
CollisionAlert createTestCollisionAlert(int boat_id) {
    CollisionAlert alert;
    alert.current_boat_id = boat_id;
    alert.level = static_cast<AlertLevel>((boat_id % 3) + 1);
    alert.collision_time = 15.5;
    alert.collision_position.lat = 30.549832;
    alert.collision_position.lng = 114.342922;
    alert.front_boat_ids.push_back(boat_id + 1);
    alert.oncoming_boat_ids.push_back(boat_id + 2);
    return alert;
}

int main() {
    std::cout << "=== MQTT通信器测试程序 ===" << std::endl;
    
    // 加载配置
    MQTTConfig config = loadMQTTConfig("../config/mqtt_config.json");
    config.client_id = "boat_pro_test_client";  // 使用测试客户端ID
    
    std::cout << "MQTT配置:" << std::endl;
    std::cout << "  代理服务器: " << config.broker_host << ":" << config.broker_port << std::endl;
    std::cout << "  客户端ID: " << config.client_id << std::endl;
    std::cout << "  订阅主题:" << std::endl;
    std::cout << "    船只状态: " << config.topics.subscribe.boat_state << std::endl;
    std::cout << "    船坞信息: " << config.topics.subscribe.dock_info << std::endl;
    std::cout << "    航线信息: " << config.topics.subscribe.route_info << std::endl;
    std::cout << "    系统配置: " << config.topics.subscribe.system_config << std::endl;
    std::cout << "  发布主题:" << std::endl;
    std::cout << "    碰撞告警: " << config.topics.publish.collision_alert << std::endl;
    std::cout << "    安全状态: " << config.topics.publish.safety_status << std::endl;
    
    // 创建MQTT通信器
    MQTTCommunicator mqtt(config);
    
    // 设置回调函数
    mqtt.setConnectionCallback([](bool connected) {
        std::cout << "连接状态变化: " << (connected ? "已连接" : "已断开") << std::endl;
    });
    
    mqtt.setBoatStateCallback([](const BoatState& boat) {
        std::cout << "收到船只状态 - ID: " << boat.sysid 
                  << ", 位置: (" << boat.lat << ", " << boat.lng << ")"
                  << ", 速度: " << boat.speed << " m/s" << std::endl;
    });
    
    mqtt.setCollisionAlertCallback([](const CollisionAlert& alert) {
        std::cout << "收到碰撞告警 - 船只ID: " << alert.current_boat_id
                  << ", 等级: " << static_cast<int>(alert.level)
                  << ", 碰撞时间: " << alert.collision_time << "s" << std::endl;
    });
    
    mqtt.setMessageCallback([](const MQTTMessage& msg) {
        std::cout << "收到MQTT消息 - 主题: " << msg.topic 
                  << ", 载荷长度: " << msg.payload.length() << " bytes" << std::endl;
    });
    
    // 初始化并连接
    if (!mqtt.initialize()) {
        std::cerr << "初始化MQTT通信器失败" << std::endl;
        return 1;
    }
    
    std::cout << "正在连接到MQTT代理服务器..." << std::endl;
    if (!mqtt.connect()) {
        std::cerr << "连接MQTT代理服务器失败" << std::endl;
        return 1;
    }
    
    std::cout << "连接成功！开始测试..." << std::endl;
    
    // 启动发布线程
    mqtt.startPublishing();
    
    // 测试发布船只状态
    std::cout << "\n--- 测试发布船只状态 ---" << std::endl;
    for (int i = 1; i <= 3; i++) {
        BoatState boat = createTestBoatState(i);
        if (mqtt.publishBoatState(boat)) {
            std::cout << "发布船只 " << i << " 状态成功" << std::endl;
        } else {
            std::cout << "发布船只 " << i << " 状态失败" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // 测试发布碰撞告警
    std::cout << "\n--- 测试发布碰撞告警 ---" << std::endl;
    for (int i = 1; i <= 2; i++) {
        CollisionAlert alert = createTestCollisionAlert(i);
        if (mqtt.publishCollisionAlert(alert)) {
            std::cout << "发布船只 " << i << " 碰撞告警成功" << std::endl;
        } else {
            std::cout << "发布船只 " << i << " 碰撞告警失败" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // 测试发布系统配置
    std::cout << "\n--- 测试发布系统配置 ---" << std::endl;
    SystemConfig sys_config = SystemConfig::getDefault();
    if (mqtt.publishSystemConfig(sys_config)) {
        std::cout << "发布系统配置成功" << std::endl;
    } else {
        std::cout << "发布系统配置失败" << std::endl;
    }
    
    // 测试心跳消息
    std::cout << "\n--- 测试心跳消息 ---" << std::endl;
    for (int i = 1; i <= 3; i++) {
        if (mqtt.publishHeartbeat(i)) {
            std::cout << "发布船只 " << i << " 心跳成功" << std::endl;
        } else {
            std::cout << "发布船只 " << i << " 心跳失败" << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    // 持续运行一段时间以接收消息
    std::cout << "\n--- 持续运行30秒以接收消息 ---" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(30));
    
    // 显示统计信息
    auto stats = mqtt.getStatistics();
    std::cout << "\n=== 统计信息 ===" << std::endl;
    std::cout << "发布消息数: " << stats.messages_published << std::endl;
    std::cout << "接收消息数: " << stats.messages_received << std::endl;
    std::cout << "发布字节数: " << stats.bytes_published << std::endl;
    std::cout << "接收字节数: " << stats.bytes_received << std::endl;
    std::cout << "发布错误数: " << stats.publish_errors << std::endl;
    std::cout << "连接丢失次数: " << stats.connection_lost_count << std::endl;
    std::cout << "重连次数: " << stats.reconnect_count << std::endl;
    std::cout << "当前连接状态: " << (stats.is_connected ? "已连接" : "未连接") << std::endl;
    
    // 清理
    std::cout << "\n正在关闭MQTT通信器..." << std::endl;
    mqtt.shutdown();
    
    std::cout << "测试完成！" << std::endl;
    return 0;
}
