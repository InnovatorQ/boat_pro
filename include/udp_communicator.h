// ==================== include/udp_communicator.h ====================
#ifndef BOAT_PRO_UDP_COMMUNICATOR_H
#define BOAT_PRO_UDP_COMMUNICATOR_H

#include "types.h"
#include "communication_protocol.h"
#include <string>
#include <memory>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <vector>

namespace boat_pro {
namespace communication {

/**
 * UDP通信器配置
 */
struct UDPConfig {
    std::string local_ip = "0.0.0.0";      // 本地IP地址
    uint16_t local_port = 8888;             // 本地端口
    std::string remote_ip = "255.255.255.255"; // 远程IP地址(广播)
    uint16_t remote_port = 8889;            // 远程端口
    bool enable_broadcast = true;           // 是否启用广播
    int receive_timeout_ms = 1000;          // 接收超时(毫秒)
    size_t max_packet_size = 1024;          // 最大数据包大小
};

/**
 * 消息回调函数类型定义
 */
using DroneIDMessageCallback = std::function<void(std::unique_ptr<DroneIDMessage>)>;
using NMEA2000MessageCallback = std::function<void(std::unique_ptr<NMEA2000Message>)>;
using BoatStateCallback = std::function<void(const BoatState&)>;

/**
 * UDP通信器类
 * 负责通过UDP协议发送和接收Drone ID和NMEA 2000消息
 */
class UDPCommunicator {
public:
    UDPCommunicator(const UDPConfig& config = UDPConfig{});
    ~UDPCommunicator();
    
    /**
     * 初始化UDP套接字
     */
    bool initialize();
    
    /**
     * 关闭通信器
     */
    void shutdown();
    
    /**
     * 启动接收线程
     */
    bool startReceiving();
    
    /**
     * 停止接收线程
     */
    void stopReceiving();
    
    /**
     * 设置消息回调函数
     */
    void setDroneIDCallback(DroneIDMessageCallback callback);
    void setNMEA2000Callback(NMEA2000MessageCallback callback);
    void setBoatStateCallback(BoatStateCallback callback);
    
    /**
     * 发送Drone ID消息
     */
    bool sendDroneIDMessage(const DroneIDMessage& message);
    
    /**
     * 发送NMEA 2000消息
     */
    bool sendNMEA2000Message(const NMEA2000Message& message);
    
    /**
     * 发送船只状态(自动转换为协议消息)
     */
    bool sendBoatState(const BoatState& boat, bool use_drone_id = true, bool use_nmea2000 = true);
    
    /**
     * 获取统计信息
     */
    struct Statistics {
        uint64_t packets_sent = 0;
        uint64_t packets_received = 0;
        uint64_t bytes_sent = 0;
        uint64_t bytes_received = 0;
        uint64_t send_errors = 0;
        uint64_t receive_errors = 0;
    };
    Statistics getStatistics() const;
    
private:
    UDPConfig config_;
    int socket_fd_;
    std::atomic<bool> receiving_;
    std::thread receive_thread_;
    mutable std::mutex stats_mutex_;
    Statistics stats_;
    
    // 回调函数
    DroneIDMessageCallback drone_id_callback_;
    NMEA2000MessageCallback nmea2000_callback_;
    BoatStateCallback boat_state_callback_;
    
    /**
     * 接收线程函数
     */
    void receiveLoop();
    
    /**
     * 处理接收到的数据包
     */
    void processReceivedPacket(const std::vector<uint8_t>& packet);
    
    /**
     * 解析Drone ID消息
     */
    std::unique_ptr<DroneIDMessage> parseDroneIDMessage(const std::vector<uint8_t>& data);
    
    /**
     * 解析NMEA 2000消息
     */
    std::unique_ptr<NMEA2000Message> parseNMEA2000Message(const std::vector<uint8_t>& data);
    
    /**
     * 创建UDP套接字
     */
    bool createSocket();
    
    /**
     * 配置套接字选项
     */
    bool configureSocket();
    
    /**
     * 发送原始数据
     */
    bool sendRawData(const std::vector<uint8_t>& data);
};

} // namespace communication
} // namespace boat_pro

#endif