// ==================== include/mqtt_communicator.h ====================
#ifndef BOAT_PRO_MQTT_COMMUNICATOR_H
#define BOAT_PRO_MQTT_COMMUNICATOR_H

#include "types.h"
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>
#include <map>
#include <queue>
#include <condition_variable>

namespace boat_pro {
namespace communication {

/**
 * MQTT服务质量等级
 */
enum class MQTTQoS : int {
    AT_MOST_ONCE = 0,   // 最多一次
    AT_LEAST_ONCE = 1,  // 至少一次
    EXACTLY_ONCE = 2    // 恰好一次
};

/**
 * MQTT通信器配置
 */
struct MQTTConfig {
    std::string broker_host = "localhost";      // MQTT代理服务器地址
    int broker_port = 1883;                     // MQTT代理服务器端口
    std::string client_id = "boat_pro_client";  // 客户端ID
    std::string username = "";                  // 用户名
    std::string password = "";                  // 密码
    bool clean_session = true;                  // 清理会话
    int keep_alive = 60;                        // 保活时间(秒)
    int connect_timeout = 30;                   // 连接超时(秒)
    bool enable_ssl = false;                    // 是否启用SSL/TLS
    std::string ca_cert_file = "";              // CA证书文件路径
    std::string cert_file = "";                 // 客户端证书文件路径
    std::string key_file = "";                  // 客户端私钥文件路径
    
    // 主题配置
    struct Topics {
        // 订阅主题
        struct Subscribe {
            std::string boat_state = "dock/BoatState";      // 无人船动态数据
            std::string dock_info = "dock/DockInfo";        // 船坞静态数据
            std::string route_info = "dock/RouteInfo";      // 船线定义数据
            std::string system_config = "dock/Config";      // 系统配置文件
        } subscribe;
        
        // 发布主题
        struct Publish {
            std::string collision_alert = "dock/CollisionAlert";   // 碰撞告警
            std::string safety_status = "dock/SafetyStatus";       // 安全状态
            std::string fleet_command = "dock/FleetCommand";       // 舰队命令
            std::string system_status = "dock/SystemStatus";       // 系统状态
            std::string heartbeat = "dock/Heartbeat";              // 心跳消息
        } publish;
    } topics;
    
    MQTTQoS default_qos = MQTTQoS::AT_LEAST_ONCE;  // 默认服务质量
    bool retain_messages = false;                   // 是否保留消息
    int publish_interval_ms = 1000;                 // 发布间隔(毫秒)
};

/**
 * MQTT消息结构
 */
struct MQTTMessage {
    std::string topic;
    std::string payload;
    MQTTQoS qos;
    bool retain;
    uint64_t timestamp;
    
    MQTTMessage(const std::string& t, const std::string& p, 
                MQTTQoS q = MQTTQoS::AT_LEAST_ONCE, bool r = false)
        : topic(t), payload(p), qos(q), retain(r), timestamp(0) {}
};

/**
 * 消息回调函数类型定义
 */
using MQTTMessageCallback = std::function<void(const MQTTMessage&)>;
using MQTTConnectionCallback = std::function<void(bool connected)>;
using MQTTBoatStateCallback = std::function<void(const BoatState&)>;
using MQTTCollisionAlertCallback = std::function<void(const CollisionAlert&)>;
using MQTTSystemConfigCallback = std::function<void(const SystemConfig&)>;

/**
 * MQTT通信器类
 * 负责通过MQTT协议发送和接收船只相关消息
 */
class MQTTCommunicator {
public:
    MQTTCommunicator(const MQTTConfig& config = MQTTConfig{});
    ~MQTTCommunicator();
    
    /**
     * 初始化MQTT客户端
     */
    bool initialize();
    
    /**
     * 连接到MQTT代理服务器
     */
    bool connect();
    
    /**
     * 断开连接
     */
    void disconnect();
    
    /**
     * 关闭通信器
     */
    void shutdown();
    
    /**
     * 检查连接状态
     */
    bool isConnected() const;
    
    /**
     * 启动发布线程
     */
    bool startPublishing();
    
    /**
     * 停止发布线程
     */
    void stopPublishing();
    
    /**
     * 设置回调函数
     */
    void setMessageCallback(MQTTMessageCallback callback);
    void setConnectionCallback(MQTTConnectionCallback callback);
    void setBoatStateCallback(MQTTBoatStateCallback callback);
    void setCollisionAlertCallback(MQTTCollisionAlertCallback callback);
    void setSystemConfigCallback(MQTTSystemConfigCallback callback);
    
    /**
     * 订阅主题
     */
    bool subscribe(const std::string& topic, MQTTQoS qos = MQTTQoS::AT_LEAST_ONCE);
    
    /**
     * 取消订阅主题
     */
    bool unsubscribe(const std::string& topic);
    
    /**
     * 发布消息
     */
    bool publish(const std::string& topic, const std::string& payload, 
                MQTTQoS qos = MQTTQoS::AT_LEAST_ONCE, bool retain = false);
    
    /**
     * 发布船只状态
     */
    bool publishBoatState(const BoatState& boat);
    
    /**
     * 发布碰撞告警
     */
    bool publishCollisionAlert(const CollisionAlert& alert);
    
    /**
     * 发布系统配置
     */
    bool publishSystemConfig(const SystemConfig& config);
    
    /**
     * 发布心跳消息
     */
    bool publishHeartbeat(int boat_id);
    
    /**
     * 订阅所有相关主题
     */
    bool subscribeAllTopics();
    
    /**
     * 获取统计信息
     */
    struct Statistics {
        uint64_t messages_published = 0;
        uint64_t messages_received = 0;
        uint64_t bytes_published = 0;
        uint64_t bytes_received = 0;
        uint64_t publish_errors = 0;
        uint64_t connection_lost_count = 0;
        uint64_t reconnect_count = 0;
        bool is_connected = false;
    };
    Statistics getStatistics() const;
    
    /**
     * 获取配置信息
     */
    const MQTTConfig& getConfig() const { return config_; }
    
private:
    MQTTConfig config_;
    void* mqtt_client_;  // mosquitto client指针
    std::atomic<bool> connected_;
    std::atomic<bool> publishing_;
    std::thread publish_thread_;
    mutable std::mutex stats_mutex_;
    mutable std::mutex message_queue_mutex_;
    std::condition_variable message_queue_cv_;
    Statistics stats_;
    
    // 消息队列
    std::queue<MQTTMessage> message_queue_;
    
    // 回调函数
    MQTTMessageCallback message_callback_;
    MQTTConnectionCallback connection_callback_;
    MQTTBoatStateCallback boat_state_callback_;
    MQTTCollisionAlertCallback collision_alert_callback_;
    MQTTSystemConfigCallback system_config_callback_;
    
    /**
     * 发布线程函数
     */
    void publishLoop();
    
    /**
     * 处理接收到的消息
     */
    void handleMessage(const std::string& topic, const std::string& payload);
    
    /**
     * 解析船只状态消息
     */
    bool parseBoatState(const std::string& payload, BoatState& boat);
    
    /**
     * 解析碰撞告警消息
     */
    bool parseCollisionAlert(const std::string& payload, CollisionAlert& alert);
    
    /**
     * 解析系统配置消息
     */
    bool parseSystemConfig(const std::string& payload, SystemConfig& config);
    
    /**
     * 生成主题名称
     */
    std::string generateBoatStateTopic(int boat_id) const;
    std::string generateCollisionAlertTopic(int boat_id) const;
    std::string generateFleetCommandTopic(int boat_id) const;
    std::string generateHeartbeatTopic(int boat_id) const;
    
    /**
     * MQTT回调函数（静态）
     */
    static void onConnect(void* context, int result);
    static void onDisconnect(void* context, int result);
    static void onMessage(void* context, const char* topic, const void* payload, int payload_len);
    static void onPublish(void* context, int message_id);
    static void onSubscribe(void* context, int message_id, int qos_count, const int* granted_qos);
    static void onLog(void* context, int level, const char* message);
    
    /**
     * 重连机制
     */
    bool reconnect();
    void startReconnectTimer();
    
    /**
     * 初始化mosquitto库
     */
    bool initializeMosquitto();
    
    /**
     * 清理mosquitto资源
     */
    void cleanupMosquitto();
};

} // namespace communication
} // namespace boat_pro

#endif
