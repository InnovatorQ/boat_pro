// ==================== src/mqtt_communicator.cpp ====================
#include "mqtt_communicator.h"
#include <mosquitto.h>
#include <jsoncpp/json/json.h>
#include <iostream>
#include <chrono>
#include <sstream>
#include <iomanip>

namespace boat_pro {
namespace communication {

MQTTCommunicator::MQTTCommunicator(const MQTTConfig& config)
    : config_(config), mqtt_client_(nullptr), connected_(false), publishing_(false) {
    initializeMosquitto();
}

MQTTCommunicator::~MQTTCommunicator() {
    shutdown();
    cleanupMosquitto();
}

bool MQTTCommunicator::initialize() {
    if (mqtt_client_) {
        mosquitto_destroy(static_cast<mosquitto*>(mqtt_client_));
    }
    
    // 创建MQTT客户端
    mqtt_client_ = mosquitto_new(config_.client_id.c_str(), config_.clean_session, this);
    if (!mqtt_client_) {
        std::cerr << "Failed to create MQTT client" << std::endl;
        return false;
    }
    
    auto* client = static_cast<mosquitto*>(mqtt_client_);
    
    // 设置回调函数
    mosquitto_connect_callback_set(client, [](struct mosquitto *mosq, void *obj, int result) {
        onConnect(obj, result);
    });
    mosquitto_disconnect_callback_set(client, [](struct mosquitto *mosq, void *obj, int result) {
        onDisconnect(obj, result);
    });
    mosquitto_message_callback_set(client, [](struct mosquitto *mosq, void *obj, const struct mosquitto_message *message) {
        onMessage(obj, message->topic, message->payload, message->payloadlen);
    });
    mosquitto_publish_callback_set(client, [](struct mosquitto *mosq, void *obj, int mid) {
        onPublish(obj, mid);
    });
    mosquitto_subscribe_callback_set(client, [](struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos) {
        onSubscribe(obj, mid, qos_count, granted_qos);
    });
    mosquitto_log_callback_set(client, [](struct mosquitto *mosq, void *obj, int level, const char *str) {
        onLog(obj, level, str);
    });
    
    // 设置用户名和密码
    if (!config_.username.empty()) {
        mosquitto_username_pw_set(client, config_.username.c_str(), 
                                 config_.password.empty() ? nullptr : config_.password.c_str());
    }
    
    // 设置SSL/TLS
    if (config_.enable_ssl) {
        int result = mosquitto_tls_set(client, 
                                      config_.ca_cert_file.empty() ? nullptr : config_.ca_cert_file.c_str(),
                                      nullptr,  // capath
                                      config_.cert_file.empty() ? nullptr : config_.cert_file.c_str(),
                                      config_.key_file.empty() ? nullptr : config_.key_file.c_str(),
                                      nullptr); // pw_callback
        if (result != MOSQ_ERR_SUCCESS) {
            std::cerr << "Failed to set TLS options: " << mosquitto_strerror(result) << std::endl;
            return false;
        }
    }
    
    return true;
}

bool MQTTCommunicator::connect() {
    if (!mqtt_client_) {
        if (!initialize()) {
            return false;
        }
    }
    
    auto* client = static_cast<mosquitto*>(mqtt_client_);
    
    int result = mosquitto_connect(client, config_.broker_host.c_str(), 
                                  config_.broker_port, config_.keep_alive);
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to connect to MQTT broker: " << mosquitto_strerror(result) << std::endl;
        return false;
    }
    
    // 启动网络循环
    result = mosquitto_loop_start(client);
    if (result != MOSQ_ERR_SUCCESS) {
        std::cerr << "Failed to start MQTT loop: " << mosquitto_strerror(result) << std::endl;
        return false;
    }
    
    // 等待连接建立
    auto start_time = std::chrono::steady_clock::now();
    while (!connected_ && 
           std::chrono::duration_cast<std::chrono::seconds>(
               std::chrono::steady_clock::now() - start_time).count() < config_.connect_timeout) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    return connected_;
}

void MQTTCommunicator::disconnect() {
    if (mqtt_client_ && connected_) {
        auto* client = static_cast<mosquitto*>(mqtt_client_);
        mosquitto_disconnect(client);
        mosquitto_loop_stop(client, true);
        connected_ = false;
    }
}

void MQTTCommunicator::shutdown() {
    stopPublishing();
    disconnect();
}

bool MQTTCommunicator::isConnected() const {
    return connected_;
}

bool MQTTCommunicator::startPublishing() {
    if (publishing_) {
        return true;
    }
    
    publishing_ = true;
    publish_thread_ = std::thread(&MQTTCommunicator::publishLoop, this);
    return true;
}

void MQTTCommunicator::stopPublishing() {
    if (publishing_) {
        publishing_ = false;
        message_queue_cv_.notify_all();
        if (publish_thread_.joinable()) {
            publish_thread_.join();
        }
    }
}

void MQTTCommunicator::setMessageCallback(MQTTMessageCallback callback) {
    message_callback_ = callback;
}

void MQTTCommunicator::setConnectionCallback(MQTTConnectionCallback callback) {
    connection_callback_ = callback;
}

void MQTTCommunicator::setBoatStateCallback(MQTTBoatStateCallback callback) {
    boat_state_callback_ = callback;
}

void MQTTCommunicator::setCollisionAlertCallback(MQTTCollisionAlertCallback callback) {
    collision_alert_callback_ = callback;
}

void MQTTCommunicator::setSystemConfigCallback(MQTTSystemConfigCallback callback) {
    system_config_callback_ = callback;
}

bool MQTTCommunicator::subscribe(const std::string& topic, MQTTQoS qos) {
    if (!mqtt_client_ || !connected_) {
        return false;
    }
    
    auto* client = static_cast<mosquitto*>(mqtt_client_);
    int result = mosquitto_subscribe(client, nullptr, topic.c_str(), static_cast<int>(qos));
    
    if (result == MOSQ_ERR_SUCCESS) {
        std::cout << "Subscribed to topic: " << topic << std::endl;
        return true;
    } else {
        std::cerr << "Failed to subscribe to topic " << topic << ": " << mosquitto_strerror(result) << std::endl;
        return false;
    }
}

bool MQTTCommunicator::unsubscribe(const std::string& topic) {
    if (!mqtt_client_ || !connected_) {
        return false;
    }
    
    auto* client = static_cast<mosquitto*>(mqtt_client_);
    int result = mosquitto_unsubscribe(client, nullptr, topic.c_str());
    
    if (result == MOSQ_ERR_SUCCESS) {
        std::cout << "Unsubscribed from topic: " << topic << std::endl;
        return true;
    } else {
        std::cerr << "Failed to unsubscribe from topic " << topic << ": " << mosquitto_strerror(result) << std::endl;
        return false;
    }
}

bool MQTTCommunicator::publish(const std::string& topic, const std::string& payload, 
                              MQTTQoS qos, bool retain) {
    if (!mqtt_client_ || !connected_) {
        return false;
    }
    
    auto* client = static_cast<mosquitto*>(mqtt_client_);
    int result = mosquitto_publish(client, nullptr, topic.c_str(), 
                                  payload.length(), payload.c_str(), 
                                  static_cast<int>(qos), retain);
    
    if (result == MOSQ_ERR_SUCCESS) {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.messages_published++;
        stats_.bytes_published += payload.length();
        return true;
    } else {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.publish_errors++;
        std::cerr << "Failed to publish to topic " << topic << ": " << mosquitto_strerror(result) << std::endl;
        return false;
    }
}

bool MQTTCommunicator::publishBoatState(const BoatState& boat) {
    Json::Value json = boat.toJson();
    Json::StreamWriterBuilder builder;
    std::string payload = Json::writeString(builder, json);
    
    std::string topic = generateBoatStateTopic(boat.sysid);
    return publish(topic, payload, config_.default_qos, config_.retain_messages);
}

bool MQTTCommunicator::publishCollisionAlert(const CollisionAlert& alert) {
    Json::Value json = alert.toJson();
    Json::StreamWriterBuilder builder;
    std::string payload = Json::writeString(builder, json);
    
    std::string topic = generateCollisionAlertTopic(alert.current_boat_id);
    return publish(topic, payload, MQTTQoS::AT_LEAST_ONCE, false); // 告警消息不保留
}

bool MQTTCommunicator::publishSystemConfig(const SystemConfig& config) {
    Json::Value json = config.toJson();
    Json::StreamWriterBuilder builder;
    std::string payload = Json::writeString(builder, json);
    
    return publish(config_.topics.subscribe.system_config, payload, MQTTQoS::AT_LEAST_ONCE, true); // 配置消息保留
}

bool MQTTCommunicator::publishHeartbeat(int boat_id) {
    Json::Value json;
    json["boat_id"] = boat_id;
    json["timestamp"] = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    json["status"] = "alive";
    
    Json::StreamWriterBuilder builder;
    std::string payload = Json::writeString(builder, json);
    
    std::string topic = generateHeartbeatTopic(boat_id);
    return publish(topic, payload, MQTTQoS::AT_MOST_ONCE, false);
}

bool MQTTCommunicator::subscribeAllTopics() {
    if (!connected_) {
        return false;
    }
    
    bool success = true;
    
    // 订阅无人船动态数据
    success &= subscribe(config_.topics.subscribe.boat_state, config_.default_qos);
    
    // 订阅船坞静态数据
    success &= subscribe(config_.topics.subscribe.dock_info, config_.default_qos);
    
    // 订阅船线定义数据
    success &= subscribe(config_.topics.subscribe.route_info, config_.default_qos);
    
    // 订阅系统配置文件
    success &= subscribe(config_.topics.subscribe.system_config, config_.default_qos);
    
    return success;
}

MQTTCommunicator::Statistics MQTTCommunicator::getStatistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    Statistics stats = stats_;
    stats.is_connected = connected_;
    return stats;
}

void MQTTCommunicator::publishLoop() {
    while (publishing_) {
        std::unique_lock<std::mutex> lock(message_queue_mutex_);
        
        // 等待消息或超时
        message_queue_cv_.wait_for(lock, std::chrono::milliseconds(config_.publish_interval_ms),
                                  [this] { return !message_queue_.empty() || !publishing_; });
        
        // 处理队列中的消息
        while (!message_queue_.empty() && publishing_) {
            MQTTMessage msg = message_queue_.front();
            message_queue_.pop();
            lock.unlock();
            
            publish(msg.topic, msg.payload, msg.qos, msg.retain);
            
            lock.lock();
        }
    }
}

void MQTTCommunicator::handleMessage(const std::string& topic, const std::string& payload) {
    // 创建MQTT消息对象
    MQTTMessage msg(topic, payload);
    msg.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
    
    // 调用通用消息回调
    if (message_callback_) {
        message_callback_(msg);
    }
    
    // 根据主题类型调用特定回调
    if (topic == config_.topics.subscribe.boat_state) {
        BoatState boat;
        if (parseBoatState(payload, boat) && boat_state_callback_) {
            boat_state_callback_(boat);
        }
    } else if (topic == config_.topics.subscribe.dock_info) {
        // 处理船坞信息 - 可以添加相应的回调和解析
        std::cout << "收到船坞信息: " << payload << std::endl;
    } else if (topic == config_.topics.subscribe.route_info) {
        // 处理航线信息 - 可以添加相应的回调和解析
        std::cout << "收到航线信息: " << payload << std::endl;
    } else if (topic == config_.topics.subscribe.system_config) {
        SystemConfig config;
        if (parseSystemConfig(payload, config) && system_config_callback_) {
            system_config_callback_(config);
        }
    }
}

bool MQTTCommunicator::parseBoatState(const std::string& payload, BoatState& boat) {
    try {
        Json::Value json;
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream stream(payload);
        
        if (Json::parseFromStream(builder, stream, &json, &errors)) {
            boat.loadFromJson(json);  // 使用新的方法名
            return true;
        } else {
            std::cerr << "Failed to parse JSON: " << errors << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse boat state: " << e.what() << std::endl;
    }
    return false;
}

bool MQTTCommunicator::parseCollisionAlert(const std::string& payload, CollisionAlert& alert) {
    try {
        Json::Value json;
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream stream(payload);
        
        if (Json::parseFromStream(builder, stream, &json, &errors)) {
            // 手动解析CollisionAlert（假设有相应的fromJson方法）
            alert.current_boat_id = json["current_boat_id"].asInt();
            alert.level = static_cast<AlertLevel>(json["level"].asInt());
            alert.collision_time = json["collision_time"].asDouble();
            
            if (json.isMember("collision_position")) {
                alert.collision_position.lat = json["collision_position"]["lat"].asDouble();
                alert.collision_position.lng = json["collision_position"]["lng"].asDouble();
            }
            
            if (json.isMember("front_boat_ids") && json["front_boat_ids"].isArray()) {
                for (const auto& id : json["front_boat_ids"]) {
                    alert.front_boat_ids.push_back(id.asInt());
                }
            }
            
            if (json.isMember("oncoming_boat_ids") && json["oncoming_boat_ids"].isArray()) {
                for (const auto& id : json["oncoming_boat_ids"]) {
                    alert.oncoming_boat_ids.push_back(id.asInt());
                }
            }
            
            return true;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse collision alert: " << e.what() << std::endl;
    }
    return false;
}

bool MQTTCommunicator::parseSystemConfig(const std::string& payload, SystemConfig& config) {
    try {
        Json::Value json;
        Json::CharReaderBuilder builder;
        std::string errors;
        std::istringstream stream(payload);
        
        if (Json::parseFromStream(builder, stream, &json, &errors)) {
            config.loadFromJson(json);  // 使用新的方法名
            return true;
        } else {
            std::cerr << "Failed to parse JSON: " << errors << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Failed to parse system config: " << e.what() << std::endl;
    }
    return false;
}

std::string MQTTCommunicator::generateBoatStateTopic(int boat_id) const {
    return config_.topics.subscribe.boat_state;  // 订阅主题不需要boat_id后缀
}

std::string MQTTCommunicator::generateCollisionAlertTopic(int boat_id) const {
    return config_.topics.publish.collision_alert;
}

std::string MQTTCommunicator::generateFleetCommandTopic(int boat_id) const {
    return config_.topics.publish.fleet_command;
}

std::string MQTTCommunicator::generateHeartbeatTopic(int boat_id) const {
    return config_.topics.publish.heartbeat;
}

// 静态回调函数实现
void MQTTCommunicator::onConnect(void* context, int result) {
    auto* comm = static_cast<MQTTCommunicator*>(context);
    comm->connected_ = (result == 0);
    
    if (comm->connected_) {
        std::cout << "Connected to MQTT broker" << std::endl;
        comm->subscribeAllTopics();
    } else {
        std::cerr << "Failed to connect to MQTT broker: " << mosquitto_connack_string(result) << std::endl;
    }
    
    if (comm->connection_callback_) {
        comm->connection_callback_(comm->connected_);
    }
}

void MQTTCommunicator::onDisconnect(void* context, int result) {
    auto* comm = static_cast<MQTTCommunicator*>(context);
    comm->connected_ = false;
    
    std::lock_guard<std::mutex> lock(comm->stats_mutex_);
    comm->stats_.connection_lost_count++;
    
    std::cout << "Disconnected from MQTT broker" << std::endl;
    
    if (comm->connection_callback_) {
        comm->connection_callback_(false);
    }
}

void MQTTCommunicator::onMessage(void* context, const char* topic, const void* payload, int payload_len) {
    auto* comm = static_cast<MQTTCommunicator*>(context);
    
    std::string topic_str(topic);
    std::string payload_str(static_cast<const char*>(payload), payload_len);
    
    std::lock_guard<std::mutex> lock(comm->stats_mutex_);
    comm->stats_.messages_received++;
    comm->stats_.bytes_received += payload_len;
    
    // 在新线程中处理消息以避免阻塞
    std::thread([comm, topic_str, payload_str]() {
        comm->handleMessage(topic_str, payload_str);
    }).detach();
}

void MQTTCommunicator::onPublish(void* context, int message_id) {
    // 消息发布确认
}

void MQTTCommunicator::onSubscribe(void* context, int message_id, int qos_count, const int* granted_qos) {
    std::cout << "Subscription confirmed with QoS: ";
    for (int i = 0; i < qos_count; i++) {
        std::cout << granted_qos[i] << " ";
    }
    std::cout << std::endl;
}

void MQTTCommunicator::onLog(void* context, int level, const char* message) {
    // 可以根据需要实现日志记录
    if (level <= MOSQ_LOG_WARNING) {
        std::cout << "MQTT Log [" << level << "]: " << message << std::endl;
    }
}

bool MQTTCommunicator::initializeMosquitto() {
    static bool initialized = false;
    if (!initialized) {
        int result = mosquitto_lib_init();
        if (result != MOSQ_ERR_SUCCESS) {
            std::cerr << "Failed to initialize mosquitto library: " << mosquitto_strerror(result) << std::endl;
            return false;
        }
        initialized = true;
    }
    return true;
}

void MQTTCommunicator::cleanupMosquitto() {
    if (mqtt_client_) {
        mosquitto_destroy(static_cast<mosquitto*>(mqtt_client_));
        mqtt_client_ = nullptr;
    }
    mosquitto_lib_cleanup();
}

} // namespace communication
} // namespace boat_pro
