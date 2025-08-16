#ifndef MQTT_INTERFACE_H
#define MQTT_INTERFACE_H

#include <string>
#include <functional>
#include <memory>
#include <thread>
#include <atomic>
#include <json/json.h>
#include <mosquitto.h>

/**
 * MQTT接口类 - 处理无人船系统的MQTT通信
 */
class MqttInterface {
public:
    // 消息回调函数类型
    using MessageCallback = std::function<void(const std::string& topic, const Json::Value& message)>;
    
    /**
     * 构造函数
     * @param broker_host MQTT代理服务器地址
     * @param broker_port MQTT代理服务器端口
     * @param client_id 客户端ID
     */
    MqttInterface(const std::string& broker_host = "localhost", 
                  int broker_port = 1883, 
                  const std::string& client_id = "boat_safety_system");
    
    /**
     * 析构函数
     */
    ~MqttInterface();
    
    /**
     * 初始化MQTT连接
     * @return 成功返回true，失败返回false
     */
    bool initialize();
    
    /**
     * 连接到MQTT代理
     * @return 成功返回true，失败返回false
     */
    bool connect();
    
    /**
     * 断开MQTT连接
     */
    void disconnect();
    
    /**
     * 订阅主题
     * @param topic 主题名称
     * @param qos 服务质量等级 (0, 1, 2)
     * @return 成功返回true，失败返回false
     */
    bool subscribe(const std::string& topic, int qos = 1);
    
    /**
     * 取消订阅主题
     * @param topic 主题名称
     * @return 成功返回true，失败返回false
     */
    bool unsubscribe(const std::string& topic);
    
    /**
     * 发布消息
     * @param topic 主题名称
     * @param message JSON消息内容
     * @param qos 服务质量等级 (0, 1, 2)
     * @param retain 是否保留消息
     * @return 成功返回true，失败返回false
     */
    bool publish(const std::string& topic, const Json::Value& message, int qos = 1, bool retain = false);
    
    /**
     * 发布字符串消息
     * @param topic 主题名称
     * @param message 字符串消息内容
     * @param qos 服务质量等级 (0, 1, 2)
     * @param retain 是否保留消息
     * @return 成功返回true，失败返回false
     */
    bool publishString(const std::string& topic, const std::string& message, int qos = 1, bool retain = false);
    
    /**
     * 设置消息接收回调函数
     * @param callback 回调函数
     */
    void setMessageCallback(MessageCallback callback);
    
    /**
     * 启动消息循环
     */
    void startLoop();
    
    /**
     * 停止消息循环
     */
    void stopLoop();
    
    /**
     * 检查连接状态
     * @return 已连接返回true，未连接返回false
     */
    bool isConnected() const;

private:
    // MQTT客户端实例
    struct mosquitto* m_client;
    
    // 连接参数
    std::string m_broker_host;
    int m_broker_port;
    std::string m_client_id;
    
    // 状态标志
    std::atomic<bool> m_connected;
    std::atomic<bool> m_loop_running;
    
    // 消息回调
    MessageCallback m_message_callback;
    
    // 消息循环线程
    std::unique_ptr<std::thread> m_loop_thread;
    
    // 静态回调函数
    static void on_connect(struct mosquitto* mosq, void* userdata, int result);
    static void on_disconnect(struct mosquitto* mosq, void* userdata, int result);
    static void on_message(struct mosquitto* mosq, void* userdata, const struct mosquitto_message* message);
    static void on_subscribe(struct mosquitto* mosq, void* userdata, int mid, int qos_count, const int* granted_qos);
    static void on_unsubscribe(struct mosquitto* mosq, void* userdata, int mid);
    static void on_publish(struct mosquitto* mosq, void* userdata, int mid);
    
    // 消息循环函数
    void messageLoop();
};

#endif // MQTT_INTERFACE_H
