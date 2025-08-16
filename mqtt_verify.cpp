#include <iostream>
#include <mosquitto.h>
#include <json/json.h>
#include <thread>
#include <chrono>
#include <ctime>

class MqttVerifier {
private:
    struct mosquitto* client;
    bool connected;
    int message_count;

public:
    MqttVerifier() : client(nullptr), connected(false), message_count(0) {}
    
    ~MqttVerifier() {
        if (client) {
            mosquitto_destroy(client);
        }
        mosquitto_lib_cleanup();
    }
    
    bool initialize() {
        mosquitto_lib_init();
        client = mosquitto_new("mqtt_verifier", true, this);
        if (!client) {
            std::cerr << "Failed to create mosquitto client" << std::endl;
            return false;
        }
        
        mosquitto_connect_callback_set(client, on_connect);
        mosquitto_message_callback_set(client, on_message);
        
        return true;
    }
    
    bool connect() {
        int result = mosquitto_connect(client, "localhost", 1883, 60);
        if (result != MOSQ_ERR_SUCCESS) {
            std::cerr << "Failed to connect: " << mosquitto_strerror(result) << std::endl;
            return false;
        }
        
        // 等待连接
        int timeout = 5000;
        while (!connected && timeout > 0) {
            mosquitto_loop(client, 10, 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            timeout -= 10;
        }
        
        return connected;
    }
    
    void subscribe(const std::string& topic) {
        mosquitto_subscribe(client, nullptr, topic.c_str(), 1);
        std::cout << "Subscribed to: " << topic << std::endl;
    }
    
    void publish(const std::string& topic, const std::string& message) {
        int result = mosquitto_publish(client, nullptr, topic.c_str(), 
                                     message.length(), message.c_str(), 1, false);
        if (result == MOSQ_ERR_SUCCESS) {
            std::cout << "Published to " << topic << ": " << message << std::endl;
        } else {
            std::cerr << "Failed to publish: " << mosquitto_strerror(result) << std::endl;
        }
    }
    
    void loop(int seconds) {
        auto start = std::chrono::steady_clock::now();
        while (std::chrono::steady_clock::now() - start < std::chrono::seconds(seconds)) {
            mosquitto_loop(client, 100, 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    
    int getMessageCount() const { return message_count; }
    
    static void on_connect(struct mosquitto* mosq, void* userdata, int result) {
        MqttVerifier* verifier = static_cast<MqttVerifier*>(userdata);
        if (result == 0) {
            std::cout << "✓ MQTT connected successfully" << std::endl;
            verifier->connected = true;
        } else {
            std::cerr << "✗ MQTT connection failed: " << mosquitto_connack_string(result) << std::endl;
        }
    }
    
    static void on_message(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message) {
        MqttVerifier* verifier = static_cast<MqttVerifier*>(userdata);
        verifier->message_count++;
        
        std::string topic(message->topic);
        std::string payload(static_cast<char*>(message->payload), message->payloadlen);
        
        std::cout << "✓ Received message on " << topic << ": " << payload << std::endl;
    }
};

int main() {
    std::cout << "=== MQTT接口功能验证程序 ===" << std::endl;
    
    MqttVerifier verifier;
    
    // 初始化
    if (!verifier.initialize()) {
        std::cerr << "✗ 初始化失败" << std::endl;
        return 1;
    }
    std::cout << "✓ MQTT库初始化成功" << std::endl;
    
    // 连接
    if (!verifier.connect()) {
        std::cerr << "✗ 连接失败" << std::endl;
        return 1;
    }
    
    // 订阅主题
    verifier.subscribe("boat_safety/output/+");
    verifier.subscribe("boat_safety/test");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 发布测试消息
    std::cout << "\n=== 发布测试消息 ===" << std::endl;
    
    // 1. 发布简单测试消息
    verifier.publish("boat_safety/test", "Hello MQTT Interface!");
    
    // 2. 发布JSON格式的船只状态
    Json::Value boat_state;
    boat_state["sysid"] = 1;
    boat_state["timestamp"] = std::time(nullptr);
    boat_state["lat"] = 30.549832;
    boat_state["lng"] = 114.342922;
    boat_state["heading"] = 90.0;
    boat_state["speed"] = 2.5;
    boat_state["status"] = 2;
    boat_state["route_direction"] = 1;
    
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string boat_json = Json::writeString(builder, boat_state);
    verifier.publish("boat_safety/input/boat_state/1", boat_json);
    
    // 3. 发布系统状态
    Json::Value system_status;
    system_status["status"] = "running";
    system_status["timestamp"] = std::time(nullptr);
    system_status["active_boats"] = 1;
    
    std::string status_json = Json::writeString(builder, system_status);
    verifier.publish("boat_safety/output/system_status", status_json);
    
    // 4. 发布碰撞告警
    Json::Value alert;
    alert["alert_level"] = "warning";
    alert["boat_id"] = 1;
    alert["message"] = "Test collision alert";
    alert["timestamp"] = std::time(nullptr);
    
    std::string alert_json = Json::writeString(builder, alert);
    verifier.publish("boat_safety/output/collision_alert/1", alert_json);
    
    std::cout << "\n=== 等待消息接收 ===" << std::endl;
    verifier.loop(3);
    
    std::cout << "\n=== 验证结果 ===" << std::endl;
    std::cout << "接收到消息数量: " << verifier.getMessageCount() << std::endl;
    
    if (verifier.getMessageCount() > 0) {
        std::cout << "🎉 MQTT接口功能验证成功！" << std::endl;
        return 0;
    } else {
        std::cout << "⚠️  未接收到消息，请检查配置" << std::endl;
        return 1;
    }
}
