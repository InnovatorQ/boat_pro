#include "src/communication_protocol.cpp"
#include "src/udp_communicator.cpp"
#include "src/types.cpp"
#include "src/geometry_utils.cpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>

using namespace boat_pro;
using namespace boat_pro::communication;

// 测试UDP服务器
class TestUDPServer {
private:
    int socket_fd;
    std::atomic<bool> running;
    std::thread server_thread;
    int port;
    
public:
    TestUDPServer(int p) : socket_fd(-1), running(false), port(p) {}
    
    bool start() {
        socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
        if (socket_fd < 0) {
            std::cerr << "创建socket失败" << std::endl;
            return false;
        }
        
        // 设置socket选项
        int opt = 1;
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        
        struct sockaddr_in server_addr;
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);
        
        if (bind(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            std::cerr << "绑定端口 " << port << " 失败" << std::endl;
            close(socket_fd);
            return false;
        }
        
        running = true;
        server_thread = std::thread(&TestUDPServer::serverLoop, this);
        
        std::cout << "UDP测试服务器启动在端口 " << port << std::endl;
        return true;
    }
    
    void stop() {
        running = false;
        if (server_thread.joinable()) {
            server_thread.join();
        }
        if (socket_fd >= 0) {
            close(socket_fd);
        }
        std::cout << "UDP测试服务器已停止" << std::endl;
    }
    
private:
    void serverLoop() {
        char buffer[1024];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        
        while (running) {
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(socket_fd, &read_fds);
            
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            
            int result = select(socket_fd + 1, &read_fds, nullptr, nullptr, &timeout);
            if (result > 0 && FD_ISSET(socket_fd, &read_fds)) {
                ssize_t bytes_received = recvfrom(socket_fd, buffer, sizeof(buffer), 0,
                                                (struct sockaddr*)&client_addr, &client_len);
                if (bytes_received > 0) {
                    std::cout << "服务器收到 " << bytes_received << " 字节数据" << std::endl;
                    
                    // 回显数据
                    sendto(socket_fd, buffer, bytes_received, 0,
                          (struct sockaddr*)&client_addr, client_len);
                }
            }
        }
    }
};

void testNetworkConnectivity() {
    std::cout << "=== 网络连接测试 ===" << std::endl;
    
    // 启动测试服务器
    TestUDPServer server(9998);
    if (!server.start()) {
        std::cout << "无法启动测试服务器，跳过网络测试" << std::endl;
        return;
    }
    
    // 等待服务器启动
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // 创建客户端通信器
    UDPConfig config;
    config.local_port = 9999;
    config.remote_ip = "127.0.0.1";
    config.remote_port = 9998;
    config.receive_timeout_ms = 1000;
    
    UDPCommunicator client(config);
    if (!client.initialize()) {
        std::cout << "客户端初始化失败" << std::endl;
        server.stop();
        return;
    }
    
    // 测试发送船只状态
    BoatState boat;
    boat.sysid = 100;
    boat.lat = 30.549832;
    boat.lng = 114.342922;
    boat.heading = 90.0;
    boat.speed = 2.5;
    boat.status = BoatStatus::NORMAL_SAIL;
    boat.route_direction = RouteDirection::CLOCKWISE;
    
    std::cout << "发送船只状态数据..." << std::endl;
    bool sent = client.sendBoatState(boat, true, true);
    std::cout << "发送结果: " << (sent ? "成功" : "失败") << std::endl;
    
    // 等待一段时间让数据传输
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    // 获取统计信息
    auto stats = client.getStatistics();
    std::cout << "通信统计:" << std::endl;
    std::cout << "  发送包数: " << stats.packets_sent << std::endl;
    std::cout << "  接收包数: " << stats.packets_received << std::endl;
    std::cout << "  发送错误: " << stats.send_errors << std::endl;
    std::cout << "  接收错误: " << stats.receive_errors << std::endl;
    
    client.shutdown();
    server.stop();
    
    if (stats.packets_sent > 0 && stats.send_errors == 0) {
        std::cout << "网络连接测试: 通过 ✓" << std::endl;
    } else {
        std::cout << "网络连接测试: 失败 ✗" << std::endl;
    }
}

void testProtocolCompatibility() {
    std::cout << "\n=== 协议兼容性测试 ===" << std::endl;
    
    // 测试不同协议之间的数据转换
    BoatState boat;
    boat.sysid = 42;
    boat.lat = 30.549832;
    boat.lng = 114.342922;
    boat.heading = 90.0;
    boat.speed = 2.5;
    boat.status = BoatStatus::NORMAL_SAIL;
    boat.route_direction = RouteDirection::CLOCKWISE;
    
    // Drone ID协议测试
    auto drone_msg = ProtocolConverter::todroneIDLocation(boat);
    if (drone_msg) {
        auto drone_data = drone_msg->serialize();
        DroneIDLocationMessage new_drone_msg;
        if (new_drone_msg.deserialize(drone_data)) {
            std::cout << "Drone ID协议兼容性: 通过 ✓" << std::endl;
        } else {
            std::cout << "Drone ID协议兼容性: 失败 ✗" << std::endl;
        }
    }
    
    // NMEA 2000协议测试
    auto nmea_pos = ProtocolConverter::toNMEA2000Position(boat);
    auto nmea_cog = ProtocolConverter::toNMEA2000COGSOG(boat);
    
    if (nmea_pos && nmea_cog) {
        auto pos_data = nmea_pos->serialize();
        auto cog_data = nmea_cog->serialize();
        
        NMEA2000PositionMessage new_pos_msg;
        NMEA2000COGSOGMessage new_cog_msg;
        
        bool pos_ok = new_pos_msg.deserialize(pos_data);
        bool cog_ok = new_cog_msg.deserialize(cog_data);
        
        if (pos_ok && cog_ok) {
            std::cout << "NMEA 2000协议兼容性: 通过 ✓" << std::endl;
        } else {
            std::cout << "NMEA 2000协议兼容性: 失败 ✗" << std::endl;
        }
    }
}

void testConfigurationLoading() {
    std::cout << "\n=== 配置加载测试 ===" << std::endl;
    
    // 测试系统配置加载
    try {
        auto config = SystemConfig::getDefault();
        std::cout << "默认系统配置:" << std::endl;
        std::cout << "  船只长度: " << config.boat.length << "m" << std::endl;
        std::cout << "  船只宽度: " << config.boat.width << "m" << std::endl;
        std::cout << "  紧急阈值: " << config.emergency_threshold_s << "s" << std::endl;
        std::cout << "  警告阈值: " << config.warning_threshold_s << "s" << std::endl;
        std::cout << "系统配置加载: 通过 ✓" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "系统配置加载: 失败 ✗ - " << e.what() << std::endl;
    }
}

int main() {
    std::cout << "无人船通信系统验证测试" << std::endl;
    std::cout << "========================" << std::endl;
    
    try {
        testProtocolCompatibility();
        testConfigurationLoading();
        testNetworkConnectivity();
        
        std::cout << "\n=== 测试总结 ===" << std::endl;
        std::cout << "通信系统基本功能验证完成" << std::endl;
        std::cout << "建议在实际环境中进行进一步测试" << std::endl;
        
    } catch (const std::exception& e) {
        std::cout << "测试过程中发生错误: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
