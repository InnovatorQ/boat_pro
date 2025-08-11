#include "../src/communication_protocol.cpp"
#include "../src/udp_communicator.cpp"
#include "../src/types.cpp"
#include "../src/geometry_utils.cpp"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

using namespace boat_pro;
using namespace boat_pro::communication;

void testDroneIDProtocol() {
    std::cout << "测试Drone ID协议..." << std::endl;
    
    // 创建测试船只状态
    BoatState boat;
    boat.sysid = 1;
    boat.timestamp = 1722325256.530;
    boat.lat = 30.549832;
    boat.lng = 114.342922;
    boat.heading = 90.0;
    boat.speed = 2.5;
    boat.status = BoatStatus::NORMAL_SAIL;
    boat.route_direction = RouteDirection::CLOCKWISE;
    
    // 转换为Drone ID消息
    auto drone_msg = ProtocolConverter::todroneIDLocation(boat);
    assert(drone_msg != nullptr);
    
    // 序列化
    auto data = drone_msg->serialize();
    std::cout << "Drone ID消息大小: " << data.size() << " 字节" << std::endl;
    
    // 反序列化
    DroneIDLocationMessage new_msg;
    assert(new_msg.deserialize(data));
    
    // 验证数据一致性
    assert(new_msg.location.latitude == static_cast<int32_t>(boat.lat * 1e7));
    assert(new_msg.location.longitude == static_cast<int32_t>(boat.lng * 1e7));
    
    std::cout << "Drone ID协议测试通过!" << std::endl;
}

void testNMEA2000Protocol() {
    std::cout << "测试NMEA 2000协议..." << std::endl;
    
    // 创建测试船只状态
    BoatState boat;
    boat.sysid = 2;
    boat.lat = 30.549900;
    boat.lng = 114.343100;
    boat.heading = 270.0;
    boat.speed = 2.0;
    
    // 转换为NMEA 2000位置消息
    auto pos_msg = ProtocolConverter::toNMEA2000Position(boat);
    assert(pos_msg != nullptr);
    
    // 转换为NMEA 2000航向速度消息
    auto cog_msg = ProtocolConverter::toNMEA2000COGSOG(boat);
    assert(cog_msg != nullptr);
    
    // 序列化
    auto pos_data = pos_msg->serialize();
    auto cog_data = cog_msg->serialize();
    
    std::cout << "NMEA 2000位置消息大小: " << pos_data.size() << " 字节" << std::endl;
    std::cout << "NMEA 2000航向消息大小: " << cog_data.size() << " 字节" << std::endl;
    
    // 反序列化
    NMEA2000PositionMessage new_pos_msg;
    NMEA2000COGSOGMessage new_cog_msg;
    
    assert(new_pos_msg.deserialize(pos_data));
    assert(new_cog_msg.deserialize(cog_data));
    
    // 验证数据一致性
    assert(new_pos_msg.position.latitude == static_cast<int32_t>(boat.lat * 1e7));
    assert(new_pos_msg.position.longitude == static_cast<int32_t>(boat.lng * 1e7));
    
    std::cout << "NMEA 2000协议测试通过!" << std::endl;
}

void testUDPCommunication() {
    std::cout << "测试UDP通信..." << std::endl;
    
    // 创建UDP通信器配置
    UDPConfig config;
    config.local_port = 9999;
    config.remote_port = 9998;
    config.receive_timeout_ms = 500;
    
    UDPCommunicator communicator(config);
    
    // 初始化
    if (!communicator.initialize()) {
        std::cout << "UDP通信器初始化失败，跳过测试" << std::endl;
        return;
    }
    
    // 测试发送船只状态
    BoatState boat;
    boat.sysid = 3;
    boat.lat = 30.549700;
    boat.lng = 114.342800;
    boat.heading = 45.0;
    boat.speed = 1.0;
    
    bool sent = communicator.sendBoatState(boat, true, true);
    
    // 获取统计信息
    auto stats = communicator.getStatistics();
    std::cout << "发送结果: " << (sent ? "成功" : "失败") << std::endl;
    std::cout << "统计 - 发送: " << stats.packets_sent << ", 错误: " << stats.send_errors << std::endl;
    
    communicator.shutdown();
    std::cout << "UDP通信测试完成!" << std::endl;
}

int main() {
    std::cout << "开始运行通信系统测试..." << std::endl;
    
    try {
        testDroneIDProtocol();
        testNMEA2000Protocol();
        testUDPCommunication();
        std::cout << "所有通信测试通过!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "通信测试失败: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
