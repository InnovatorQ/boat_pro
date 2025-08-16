#include "mqtt_interface.h"
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>

MqttInterface::MqttInterface(const std::string& broker_host, int broker_port, const std::string& client_id)
    : m_client(nullptr)
    , m_broker_host(broker_host)
    , m_broker_port(broker_port)
    , m_client_id(client_id)
    , m_connected(false)
    , m_loop_running(false)
    , m_message_callback(nullptr)
    , m_loop_thread(nullptr)
{
}

MqttInterface::~MqttInterface() {
    stopLoop();
    disconnect();
    
    if (m_client) {
        mosquitto_destroy(m_client);
        m_client = nullptr;
    }
    
    mosquitto_lib_cleanup();
}

bool MqttInterface::initialize() {
    // 初始化mosquitto库
    int result = mosquitto_lib_init();
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to initialize mosquitto library: " << mosquitto_strerror(result) << std::endl;
        return false;
    }
    
    // 创建MQTT客户端
    m_client = mosquitto_new(m_client_id.c_str(), true, this);
    if (!m_client) {
        std::cerr << "Failed to create mosquitto client" << std::endl;
        mosquitto_lib_cleanup();
        return false;
    }
    
    // 设置回调函数
    mosquitto_connect_callback_set(m_client, on_connect);
    mosquitto_disconnect_callback_set(m_client, on_disconnect);
    mosquitto_message_callback_set(m_client, on_message);
    mosquitto_subscribe_callback_set(m_client, on_subscribe);
    mosquitto_unsubscribe_callback_set(m_client, on_unsubscribe);
    mosquitto_publish_callback_set(m_client, on_publish);
    
    return true;
}

bool MqttInterface::connect() {
    if (!m_client) {
        std::cerr << "MQTT client not initialized" << std::endl;
        return false;
    }
    
    int result = mosquitto_connect(m_client, m_broker_host.c_str(), m_broker_port, 60);
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to MQTT broker: " << mosquitto_strerror(result) << std::endl;
        return false;
    }
    
    // 等待连接完成
    int timeout = 5000; // 5秒超时
    while (!m_connected && timeout > 0) {
        mosquitto_loop(m_client, 10, 1);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        timeout -= 10;
    }
    
    return m_connected;
}

void MqttInterface::disconnect() {
    if (m_client && m_connected) {
        mosquitto_disconnect(m_client);
        m_connected = false;
    }
}

bool MqttInterface::subscribe(const std::string& topic, int qos) {
    if (!m_client || !m_connected) {
        std::cerr << "MQTT client not connected" << std::endl;
        return false;
    }
    
    int result = mosquitto_subscribe(m_client, nullptr, topic.c_str(), qos);
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to subscribe to topic " << topic << ": " << mosquitto_strerror(result) << std::endl;
        return false;
    }
    
    std::cout << "Subscribed to topic: " << topic << std::endl;
    return true;
}

bool MqttInterface::unsubscribe(const std::string& topic) {
    if (!m_client || !m_connected) {
        std::cerr << "MQTT client not connected" << std::endl;
        return false;
    }
    
    int result = mosquitto_unsubscribe(m_client, nullptr, topic.c_str());
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to unsubscribe from topic " << topic << ": " << mosquitto_strerror(result) << std::endl;
        return false;
    }
    
    std::cout << "Unsubscribed from topic: " << topic << std::endl;
    return true;
}

bool MqttInterface::publish(const std::string& topic, const Json::Value& message, int qos, bool retain) {
    Json::StreamWriterBuilder builder;
    builder["indentation"] = "";
    std::string json_string = Json::writeString(builder, message);
    
    return publishString(topic, json_string, qos, retain);
}

bool MqttInterface::publishString(const std::string& topic, const std::string& message, int qos, bool retain) {
    if (!m_client || !m_connected) {
        std::cerr << "MQTT client not connected" << std::endl;
        return false;
    }
    
    int result = mosquitto_publish(m_client, nullptr, topic.c_str(), 
                                  message.length(), message.c_str(), qos, retain);
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to publish message to topic " << topic << ": " << mosquitto_strerror(result) << std::endl;
        return false;
    }
    
    return true;
}

void MqttInterface::setMessageCallback(MessageCallback callback) {
    m_message_callback = callback;
}

void MqttInterface::startLoop() {
    if (m_loop_running) {
        return;
    }
    
    m_loop_running = true;
    m_loop_thread = std::make_unique<std::thread>(&MqttInterface::messageLoop, this);
}

void MqttInterface::stopLoop() {
    if (m_loop_running) {
        m_loop_running = false;
        if (m_loop_thread && m_loop_thread->joinable()) {
            m_loop_thread->join();
        }
        m_loop_thread.reset();
    }
}

bool MqttInterface::isConnected() const {
    return m_connected;
}

void MqttInterface::messageLoop() {
    while (m_loop_running && m_client) {
        int result = mosquitto_loop(m_client, 100, 1);
        if (result != MOSQ_ERR_SUCCESS) {
            if (result == MOSQ_ERR_CONN_LOST) {
                std::cerr << "MQTT connection lost, attempting to reconnect..." << std::endl;
                m_connected = false;
                
                // 尝试重连
                std::this_thread::sleep_for(std::chrono::seconds(1));
                if (mosquitto_reconnect(m_client) == MOSQ_ERR_SUCCESS) {
                    std::cout << "MQTT reconnected successfully" << std::endl;
                }
            } else {
                std::cerr << "MQTT loop error: " << mosquitto_strerror(result) << std::endl;
                break;
            }
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

// 静态回调函数实现
void MqttInterface::on_connect(struct mosquitto* mosq, void* userdata, int result) {
    MqttInterface* interface = static_cast<MqttInterface*>(userdata);
    
    if (result == 0) {
        std::cout << "MQTT connected successfully" << std::endl;
        interface->m_connected = true;
    } else {
        std::cerr << "MQTT connection failed: " << mosquitto_connack_string(result) << std::endl;
        interface->m_connected = false;
    }
}

void MqttInterface::on_disconnect(struct mosquitto* mosq, void* userdata, int result) {
    MqttInterface* interface = static_cast<MqttInterface*>(userdata);
    interface->m_connected = false;
    
    if (result == 0) {
        std::cout << "MQTT disconnected successfully" << std::endl;
    } else {
        std::cerr << "MQTT unexpected disconnection: " << mosquitto_strerror(result) << std::endl;
    }
}

void MqttInterface::on_message(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message) {
    MqttInterface* interface = static_cast<MqttInterface*>(userdata);
    
    if (interface->m_message_callback && message->payload) {
        std::string topic(message->topic);
        std::string payload(static_cast<char*>(message->payload), message->payloadlen);
        
        // 尝试解析JSON
        Json::Value json_message;
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream payload_stream(payload);
        
        if (Json::parseFromStream(builder, payload_stream, &json_message, &errors)) {
            interface->m_message_callback(topic, json_message);
        } else {
            // 如果不是有效的JSON，创建一个包含原始字符串的JSON对象
            json_message["raw_message"] = payload;
            interface->m_message_callback(topic, json_message);
        }
    }
}

void MqttInterface::on_subscribe(struct mosquitto* mosq, void* userdata, int mid, int qos_count, const int* granted_qos) {
    std::cout << "MQTT subscription confirmed (mid: " << mid << ")" << std::endl;
}

void MqttInterface::on_unsubscribe(struct mosquitto* mosq, void* userdata, int mid) {
    std::cout << "MQTT unsubscription confirmed (mid: " << mid << ")" << std::endl;
}

void MqttInterface::on_publish(struct mosquitto* mosq, void* userdata, int mid) {
    // 可选：记录发布确认
    // std::cout << "MQTT message published (mid: " << mid << ")" << std::endl;
}
