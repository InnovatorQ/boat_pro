// ==================== src/udp_communicator.cpp ====================
#include "udp_communicator.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>

namespace boat_pro {
namespace communication {

UDPCommunicator::UDPCommunicator(const UDPConfig& config)
    : config_(config), socket_fd_(-1), receiving_(false) {
}

UDPCommunicator::~UDPCommunicator() {
    shutdown();
}

bool UDPCommunicator::initialize() {
    return createSocket() && configureSocket();
}

void UDPCommunicator::shutdown() {
    stopReceiving();
    
    if (socket_fd_ != -1) {
        close(socket_fd_);
        socket_fd_ = -1;
    }
}

bool UDPCommunicator::startReceiving() {
    if (receiving_) return false;
    
    receiving_ = true;
    receive_thread_ = std::thread(&UDPCommunicator::receiveLoop, this);
    
    return true;
}

void UDPCommunicator::stopReceiving() {
    receiving_ = false;
    
    if (receive_thread_.joinable()) {
        receive_thread_.join();
    }
}

void UDPCommunicator::setDroneIDCallback(DroneIDMessageCallback callback) {
    drone_id_callback_ = callback;
}

void UDPCommunicator::setNMEA2000Callback(NMEA2000MessageCallback callback) {
    nmea2000_callback_ = callback;
}

void UDPCommunicator::setBoatStateCallback(BoatStateCallback callback) {
    boat_state_callback_ = callback;
}

bool UDPCommunicator::sendDroneIDMessage(const DroneIDMessage& message) {
    auto data = message.serialize();
    
    // 添加协议标识头 (Drone ID)
    std::vector<uint8_t> packet;
    packet.push_back(0xDD); // Drone ID协议标识
    packet.insert(packet.end(), data.begin(), data.end());
    
    return sendRawData(packet);
}

bool UDPCommunicator::sendNMEA2000Message(const NMEA2000Message& message) {
    auto data = message.serialize();
    
    // 添加协议标识头 (NMEA 2000)
    std::vector<uint8_t> packet;
    packet.push_back(0x4E);
    packet.push_back(0x32);// NMEA 2000协议标识 (使用0x4E32)
    packet.insert(packet.end(), data.begin(), data.end());
    
    return sendRawData(packet);
}

bool UDPCommunicator::sendBoatState(const BoatState& boat, bool use_drone_id, bool use_nmea2000) {
    bool success = true;
    
    if (use_drone_id) {
        auto drone_msg = ProtocolConverter::todroneIDLocation(boat);
        success &= sendDroneIDMessage(*drone_msg);
    }
    
    if (use_nmea2000) {
        auto pos_msg = ProtocolConverter::toNMEA2000Position(boat);
        auto cog_msg = ProtocolConverter::toNMEA2000COGSOG(boat);
        success &= sendNMEA2000Message(*pos_msg);
        success &= sendNMEA2000Message(*cog_msg);
    }
    
    return success;
}

UDPCommunicator::Statistics UDPCommunicator::getStatistics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return stats_;
}

void UDPCommunicator::receiveLoop() {
    std::vector<uint8_t> buffer(config_.max_packet_size);
    struct sockaddr_in sender_addr;
    socklen_t addr_len = sizeof(sender_addr);
    
    while (receiving_) {
        ssize_t received = recvfrom(socket_fd_, buffer.data(), buffer.size(), 0,
                                  (struct sockaddr*)&sender_addr, &addr_len);
        
        if (received > 0) {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.packets_received++;
            stats_.bytes_received += received;
            
            buffer.resize(received);
            processReceivedPacket(buffer);
            buffer.resize(config_.max_packet_size);
        } else if (received < 0) {
            if (errno != EAGAIN && errno != EWOULDBLOCK) {
                std::lock_guard<std::mutex> lock(stats_mutex_);
                stats_.receive_errors++;
                
                std::cerr << "接收错误: " << strerror(errno) << std::endl;
            }
        }
    }
}

void UDPCommunicator::processReceivedPacket(const std::vector<uint8_t>& packet) {
    if (packet.empty()) return;
    
    // 检查协议标识
    if (packet[0] == 0xDD) {
        // Drone ID协议
        std::vector<uint8_t> data(packet.begin() + 1, packet.end());
        auto message = parseDroneIDMessage(data);
        
        if (message && drone_id_callback_) {
            drone_id_callback_(std::move(message));
        }
        
        // 转换为船只状态并回调
        if (message && message->getMessageType() == DroneIDMessageType::LOCATION) {
            auto loc_msg = static_cast<DroneIDLocationMessage*>(message.get());
            auto boat = ProtocolConverter::fromDroneIDLocation(*loc_msg, 0); // 临时ID
            
            if (boat_state_callback_) {
                boat_state_callback_(boat);
            }
        }
    } else if (packet.size() >= 3 && packet[0] == 0x4E && packet[1] == 0x32) {
        // NMEA 2000协议
        std::vector<uint8_t> data(packet.begin() + 2, packet.end());
        auto message = parseNMEA2000Message(data);
        
        if (message && nmea2000_callback_) {
            nmea2000_callback_(std::move(message));
        }
        
        // 注意：NMEA 2000需要多个消息才能构成完整的船只状态
        // 这里简化处理，实际应用中需要消息缓存和组合
    }
}

std::unique_ptr<DroneIDMessage> UDPCommunicator::parseDroneIDMessage(const std::vector<uint8_t>& data) {
    if (data.empty()) return nullptr;
    
    DroneIDMessageType type = static_cast<DroneIDMessageType>(data[0]);
    std::unique_ptr<DroneIDMessage> message;
    
    switch (type) {
        case DroneIDMessageType::BASIC_ID:
            message = std::make_unique<DroneIDBasicMessage>();
            break;
        case DroneIDMessageType::LOCATION:
            message = std::make_unique<DroneIDLocationMessage>();
            break;
        default:
            return nullptr;
    }
    
    if (message && message->deserialize(data)) {
        return message;
    }
    
    return nullptr;
}

std::unique_ptr<NMEA2000Message> UDPCommunicator::parseNMEA2000Message(const std::vector<uint8_t>& data) {
    if (data.size() < 4) return nullptr;
    
    uint32_t pgn = *reinterpret_cast<const uint32_t*>(data.data());
    NMEA2000_PGN pgn_enum = static_cast<NMEA2000_PGN>(pgn);
    std::unique_ptr<NMEA2000Message> message;
    
    switch (pgn_enum) {
        case NMEA2000_PGN::POSITION_RAPID_UPDATE:
            message = std::make_unique<NMEA2000PositionMessage>();
            break;
        case NMEA2000_PGN::COG_SOG_RAPID_UPDATE:
            message = std::make_unique<NMEA2000COGSOGMessage>();
            break;
        default:
            return nullptr;
    }
    
    if (message && message->deserialize(data)) {
        return message;
    }
    
    return nullptr;
}

bool UDPCommunicator::createSocket() {
    socket_fd_ = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_fd_ == -1) {
        std::cerr << "创建套接字失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    return true;
}

bool UDPCommunicator::configureSocket() {
    // 设置为非阻塞模式
    int flags = fcntl(socket_fd_, F_GETFL, 0);
    if (flags == -1 || fcntl(socket_fd_, F_SETFL, flags | O_NONBLOCK) == -1) {
        std::cerr << "设置非阻塞模式失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    // 启用地址重用
    int reuse = 1;
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1) {
        std::cerr << "设置地址重用失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    // 如果启用广播，设置广播选项
    if (config_.enable_broadcast) {
        int broadcast = 1;
        if (setsockopt(socket_fd_, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof(broadcast)) == -1) {
            std::cerr << "设置广播失败: " << strerror(errno) << std::endl;
            return false;
        }
    }
    
    // 绑定本地地址
    struct sockaddr_in local_addr;
    std::memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family = AF_INET;
    local_addr.sin_addr.s_addr = inet_addr(config_.local_ip.c_str());
    local_addr.sin_port = htons(config_.local_port);
    
    if (bind(socket_fd_, (struct sockaddr*)&local_addr, sizeof(local_addr)) == -1) {
        std::cerr << "绑定地址失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    // 设置接收超时
    struct timeval timeout;
    timeout.tv_sec = config_.receive_timeout_ms / 1000;
    timeout.tv_usec = (config_.receive_timeout_ms % 1000) * 1000;
    
    if (setsockopt(socket_fd_, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) == -1) {
        std::cerr << "设置接收超时失败: " << strerror(errno) << std::endl;
        return false;
    }
    
    return true;
}

bool UDPCommunicator::sendRawData(const std::vector<uint8_t>& data) {
    struct sockaddr_in remote_addr;
    std::memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_addr.s_addr = inet_addr(config_.remote_ip.c_str());
    remote_addr.sin_port = htons(config_.remote_port);
    
    ssize_t sent = sendto(socket_fd_, data.data(), data.size(), 0,
                         (struct sockaddr*)&remote_addr, sizeof(remote_addr));
    
    std::lock_guard<std::mutex> lock(stats_mutex_);
    
    if (sent > 0) {
        stats_.packets_sent++;
        stats_.bytes_sent += sent;
        return true;
    } else {
        stats_.send_errors++;
        std::cerr << "发送失败: " << strerror(errno) << std::endl;
        return false;
    }
}

} // namespace communication
} // namespace boat_pro