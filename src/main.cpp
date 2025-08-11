// ==================== apps/main.cpp ====================
#include "fleet_manager.h"
#include "types.h"
#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>  // jsoncpp header
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <iomanip>  // for std::setprecision
using namespace boat_pro;

// 将航行方向转换为字符串
std::string routeDirectionToString(RouteDirection direction) {
    switch (direction) {
        case RouteDirection::CLOCKWISE:
            return "顺时针";
        case RouteDirection::COUNTERCLOCKWISE:
            return "逆时针";
        default:
            return "未知";
    }
}

// 将船只状态转换为字符串
std::string boatStatusToString(BoatStatus status) {
    switch (status) {
        case BoatStatus::UNDOCKING:
            return "出坞";
        case BoatStatus::NORMAL_SAIL:
            return "正常航行";
        case BoatStatus::DOCKING:
            return "入坞";
        default:
            return "未知";
    }
}

// 告警回调函数
void alertCallback(const CollisionAlert& alert) {
    std::cout << "\n=== 碰撞告警 ===" << std::endl;
    std::cout << "船只ID: " << alert.current_boat_id << std::endl;
    std::cout << "告警等级: ";
    
    switch (alert.level) {
        case AlertLevel::NORMAL:
            std::cout << "正常"; break;
        case AlertLevel::WARNING:
            std::cout << "警告"; break;
        case AlertLevel::EMERGENCY:
            std::cout << "紧急"; break;
    }
    std::cout << std::endl;
    
    if (!alert.front_boat_ids.empty()) {
        std::cout << "前方船只ID: ";
        for (size_t i = 0; i < alert.front_boat_ids.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << alert.front_boat_ids[i];
        }
        std::cout << std::endl;
    }
    
    if (!alert.oncoming_boat_ids.empty()) {
        std::cout << "对向船只ID: ";
        for (size_t i = 0; i < alert.oncoming_boat_ids.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << alert.oncoming_boat_ids[i];
        }
        std::cout << std::endl;
    }
    
    std::cout << "预计碰撞时间: " << alert.collision_time << " 秒" << std::endl;
    std::cout << "碰撞位置: (" << alert.collision_position.lat << ", " 
              << alert.collision_position.lng << ")" << std::endl;
    std::cout << "决策建议: " << alert.decision_advice << std::endl;
    std::cout << "================" << std::endl;
}

// 加载配置文件
SystemConfig loadConfig(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cout << "无法打开配置文件，使用默认配置。" << std::endl;
        return SystemConfig::getDefault();
    }
    
    Json::Value root;
    Json::CharReaderBuilder builder;
    std::string errors;
    
    if (!Json::parseFromStream(builder, file, &root, &errors)) {
        std::cout << "配置文件解析失败，使用默认配置。错误: " << errors << std::endl;
        return SystemConfig::getDefault();
    }
    
    return SystemConfig::fromJson(root);
}

// 【新增】加载通信配置
communication::UDPConfig loadCommunicationConfig() {
    communication::UDPConfig config;
    
    // 可以从配置文件加载，这里使用默认值
    config.local_ip = "0.0.0.0";
    config.local_port = 8888;
    config.remote_ip = "255.255.255.255"; // 广播地址
    config.remote_port = 8889;
    config.enable_broadcast = true;
    
    return config;
}

// 创建测试数据 - 优化为产生不同级别警告的场景
std::vector<BoatState> createTestBoats() {
    std::vector<BoatState> boats;
    
    // 船只1 - 正常航行，顺时针方向
    BoatState boat1;
    boat1.sysid = 1;
    boat1.timestamp = 1722325256.530;
    boat1.lat = 30.549832;
    boat1.lng = 114.342922;
    boat1.heading = 90.0;  // 向东航行
    boat1.speed = 2.0;     // 适中速度
    boat1.status = BoatStatus::NORMAL_SAIL;
    boat1.route_direction = RouteDirection::CLOCKWISE;
    boats.push_back(boat1);
    
    // 船只2 - 对向航行，距离较远（产生警告级别）
    BoatState boat2;
    boat2.sysid = 2;
    boat2.timestamp = 1722325256.530;
    boat2.lat = 30.549832;     // 相同纬度
    boat2.lng = 114.343500;    // 在船只1前方约400米处
    boat2.heading = 270.0;     // 向西航行，与船只1对向
    boat2.speed = 1.5;         // 较慢速度
    boat2.status = BoatStatus::NORMAL_SAIL;
    boat2.route_direction = RouteDirection::COUNTERCLOCKWISE;
    boats.push_back(boat2);
    
    // 船只3 - 同向航行，在船只1前方较远处（产生正常级别）
    BoatState boat3;
    boat3.sysid = 3;
    boat3.timestamp = 1722325256.530;
    boat3.lat = 30.549832;     // 相同纬度
    boat3.lng = 114.343200;    // 在船只1前方约200米处
    boat3.heading = 90.0;      // 同向航行
    boat3.speed = 1.8;         // 稍慢速度
    boat3.status = BoatStatus::NORMAL_SAIL;
    boat3.route_direction = RouteDirection::CLOCKWISE;
    boats.push_back(boat3);
    
    return boats;
}

std::vector<DockInfo> createTestDocks() {
    std::vector<DockInfo> docks;
    
    DockInfo dock1;
    dock1.dock_id = 1;
    dock1.lat = 30.549100;
    dock1.lng = 114.343000;
    docks.push_back(dock1);
    
    DockInfo dock2;
    dock2.dock_id = 2;
    dock2.lat = 30.549200;
    dock2.lng = 114.343100;
    docks.push_back(dock2);
    
    return docks;
}

std::vector<RouteInfo> createTestRoutes() {
    std::vector<RouteInfo> routes;
    
    // 顺时针路线
    RouteInfo route1;
    route1.route_id = 1;
    route1.direction = RouteDirection::CLOCKWISE;
    route1.points = {
        {30.549500, 114.342800},
        {30.549800, 114.343300},
        {30.550100, 114.343800},
        {30.550000, 114.344200},
        {30.549700, 114.344000}
    };
    routes.push_back(route1);
    
    // 逆时针路线
    RouteInfo route2;
    route2.route_id = 2;
    route2.direction = RouteDirection::COUNTERCLOCKWISE;
    route2.points = {
        {30.549700, 114.344000},
        {30.550000, 114.344200},
        {30.550100, 114.343800},
        {30.549800, 114.343300},
        {30.549500, 114.342800}
    };
    routes.push_back(route2);
    
    return routes;
}

int main() {
    std::cout << "无人船作业安全预测系统启动中..." << std::endl;
    
    // 加载配置

    SystemConfig config = loadConfig("../config/system_config.json");
    communication::UDPConfig comm_config = loadCommunicationConfig();
    // 创建船队管理器
    FleetManager fleet_manager(config);
    
    // 设置告警回调
    fleet_manager.setAlertCallback(alertCallback);
    
    // 【新增】初始化通信系统
    std::cout << "初始化通信系统..." << std::endl;
    if (!fleet_manager.initializeCommunication(comm_config)) {
        std::cerr << "通信系统初始化失败，退出程序" << std::endl;
        return -1;
    }
    
    // 【新增】启动通信系统
    if (!fleet_manager.startCommunication()) {
        std::cerr << "启动通信系统失败，退出程序" << std::endl;
        return -1;
    }
    
    // 初始化测试数据
    auto test_docks = createTestDocks();
    auto test_routes = createTestRoutes();
    auto test_boats = createTestBoats();
    
    fleet_manager.initializeDocks(test_docks);
    fleet_manager.initializeRoutes(test_routes);
    
    std::cout << "系统初始化完成，开始安全监控..." << std::endl;
    
    // 显示初始化的船只信息
    std::cout << "\n=== 初始化船只信息 ===" << std::endl;
    for (const auto& boat : test_boats) {
        std::cout << "船只ID: " << boat.sysid 
                  << ", 状态: " << boatStatusToString(boat.status)
                  << ", 航行方向: " << routeDirectionToString(boat.route_direction)
                  << ", 航向角: " << boat.heading << "°"
                  << ", 位置: (" << std::fixed << std::setprecision(6) << boat.lat << ", " << boat.lng << ")"
                  << ", 速度: " << boat.speed << " m/s" << std::endl;
    }
    std::cout << "=====================\n" << std::endl;
    
    // 启动安全监控
    fleet_manager.runSafetyMonitoring();
    
    // 模拟船只状态更新 - 逐步增加碰撞风险
    for (int i = 0; i < 12; ++i) {
        std::cout << "\n--- 第 " << (i+1) << " 秒 ---" << std::endl;
        
        // 更新船只位置
        for (auto& boat : test_boats) {
            // 简单模拟船只移动
            double distance = boat.speed * 1.0; // 1秒移动距离
            boat.lat += distance * cos(boat.heading * M_PI / 180.0) / 111000.0; // 近似转换
            boat.lng += distance * sin(boat.heading * M_PI / 180.0) / (111000.0 * cos(boat.lat * M_PI / 180.0));
            boat.timestamp += 1.0;
        }
        
        // 在特定时间点修改船只参数以产生不同级别的警告
        if (i == 3) {
            std::cout << "\n>>> 船只1加速，可能产生警告级别碰撞风险 <<<" << std::endl;
            test_boats[0].speed = 3.0;  // 增加船只1的速度
        }
        
        if (i == 6) {
            std::cout << "\n>>> 船只2改变航向并加速，增加碰撞风险 <<<" << std::endl;
            test_boats[1].speed = 2.5;  // 增加船只2的速度
            test_boats[1].heading = 260.0;  // 稍微改变航向
        }
        
        if (i == 9) {
            std::cout << "\n>>> 船只1和船只2都加速，产生紧急级别碰撞风险 <<<" << std::endl;
            test_boats[0].speed = 4.0;  // 大幅增加船只1的速度
            test_boats[1].speed = 3.5;  // 大幅增加船只2的速度
        }
        
        fleet_manager.updateBoatStates(test_boats);
        
        // 【新增】广播船只状态
        for (const auto& boat : test_boats) {
            if (i % 2 == 0) { // 每2秒广播一次
                fleet_manager.broadcastBoatState(boat, true, true);
            }
        }
        
        // 【新增】显示通信统计信息
        if (i % 5 == 0) {
            auto stats = fleet_manager.getCommunicationStatistics();
            std::cout << "\n=== 通信统计 ===" << std::endl;
            std::cout << "已发送数据包: " << stats.packets_sent << std::endl;
            std::cout << "已接收数据包: " << stats.packets_received << std::endl;
            std::cout << "已发送字节: " << stats.bytes_sent << std::endl;
            std::cout << "已接收字节: " << stats.bytes_received << std::endl;
            std::cout << "发送错误: " << stats.send_errors << std::endl;
            std::cout << "接收错误: " << stats.receive_errors << std::endl;
            std::cout << "================" << std::endl;
        }
        
        
        // 显示当前所有船只状态
        for (const auto& boat : test_boats) {
            std::cout << "船只" << boat.sysid << ": " 
                      << boatStatusToString(boat.status) << ", "
                      << routeDirectionToString(boat.route_direction) << ", "
                      << "航向" << std::fixed << std::setprecision(1) << boat.heading << "°, "
                      << "速度" << boat.speed << "m/s, "
                      << "位置(" << std::fixed << std::setprecision(6) 
                      << boat.lat << "," << boat.lng << ")" << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    
    std::cout << "测试完成，停止监控..." << std::endl;
    fleet_manager.stopSafetyMonitoring();
    fleet_manager.stopCommunication(); // 【新增】停止通信系统
    return 0;
}