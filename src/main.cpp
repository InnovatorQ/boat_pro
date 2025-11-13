// ==================== src/main.cpp ====================
#include "fleet_manager.h"
#include "types.h"
#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <vector>
#include <thread>
#include <chrono>
#include <cmath>
#include <iomanip>

using namespace boat_pro;

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
    std::cout << "决策建议: " << alert.getDecisionDescription() << std::endl;
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

// 创建测试数据
std::vector<BoatState> createTestBoats() {
    std::vector<BoatState> boats;
    
    // 船只1 - 正常航行，向东
    BoatState boat1;
    boat1.sysid = 1;
    boat1.timestamp = 1722325256.530;
    boat1.lat = 30.549832;
    boat1.lng = 114.342922;
    boat1.heading = 90.0;  // 向东航行
    boat1.speed = 2.0;     // 适中速度
    boat1.status = BoatStatus::NORMAL_SAIL;
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
    boats.push_back(boat2);
    
    // 船只3 - 同向航行，在船只1前方较远处
    BoatState boat3;
    boat3.sysid = 3;
    boat3.timestamp = 1722325256.530;
    boat3.lat = 30.549832;     // 相同纬度
    boat3.lng = 114.343200;    // 在船只1前方约200米处
    boat3.heading = 90.0;      // 同向航行
    boat3.speed = 1.8;         // 稍慢速度
    boat3.status = BoatStatus::NORMAL_SAIL;
    boats.push_back(boat3);
    
    return boats;
}

// 创建测试船坞
std::vector<DockInfo> createTestDocks() {
    std::vector<DockInfo> docks;
    
    DockInfo dock1;
    dock1.dock_id = 1;
    dock1.lat = 30.549500;
    dock1.lng = 114.342500;
    docks.push_back(dock1);
    
    DockInfo dock2;
    dock2.dock_id = 2;
    dock2.lat = 30.550000;
    dock2.lng = 114.343000;
    docks.push_back(dock2);
    
    return docks;
}

// 创建测试航线
std::vector<RouteInfo> createTestRoutes() {
    std::vector<RouteInfo> routes;
    
    // 航线1 - 顺时针
    RouteInfo route1;
    route1.sysid = 1;
    route1.points.push_back(GeoPoint(30.549500, 114.342500));
    route1.points.push_back(GeoPoint(30.549800, 114.342800));
    route1.points.push_back(GeoPoint(30.550100, 114.343100));
    route1.points.push_back(GeoPoint(30.549800, 114.343400));
    route1.points.push_back(GeoPoint(30.549500, 114.343100));
    routes.push_back(route1);
    
    // 航线2 - 逆时针
    RouteInfo route2;
    route2.sysid = 2;
    route2.points.push_back(GeoPoint(30.549500, 114.343100));
    route2.points.push_back(GeoPoint(30.549800, 114.343400));
    route2.points.push_back(GeoPoint(30.550100, 114.343100));
    route2.points.push_back(GeoPoint(30.549800, 114.342800));
    route2.points.push_back(GeoPoint(30.549500, 114.342500));
    routes.push_back(route2);
    
    return routes;
}

int main() {
    std::cout << "=== 无人船碰撞预测系统 ===" << std::endl;
    
    // 加载系统配置
    SystemConfig config = loadConfig("../config/system_config.json");
    std::cout << "系统配置加载完成" << std::endl;
    std::cout << "船只尺寸: " << config.boat.length << "m x " << config.boat.width << "m" << std::endl;
    std::cout << "紧急阈值: " << config.emergency_threshold_s << "秒" << std::endl;
    std::cout << "警告阈值: " << config.warning_threshold_s << "秒" << std::endl;
    
    // 创建舰队管理器
    FleetManager fleet_manager(config);
    
    // 设置告警回调
    fleet_manager.setAlertCallback(alertCallback);
    
    // 初始化船坞和航线
    auto docks = createTestDocks();
    auto routes = createTestRoutes();
    fleet_manager.initializeDocks(docks);
    fleet_manager.initializeRoutes(routes);
    
    std::cout << "初始化了 " << docks.size() << " 个船坞和 " << routes.size() << " 条航线" << std::endl;
    
    // 创建测试船只
    auto boats = createTestBoats();
    std::cout << "\n创建了 " << boats.size() << " 条测试船只:" << std::endl;
    for (const auto& boat : boats) {
        std::cout << "船只ID: " << boat.sysid 
                  << ", 位置: (" << std::fixed << std::setprecision(6) << boat.lat << ", " << boat.lng << ")"
                  << ", 航向: " << boat.heading << "°"
                  << ", 速度: " << boat.speed << "m/s"
                  << ", 状态: " << boatStatusToString(boat.status) << std::endl;
    }
    
    // 更新船只状态并执行碰撞检测
    std::cout << "\n开始碰撞检测..." << std::endl;
    fleet_manager.updateBoatStates(boats);
    fleet_manager.performCollisionDetection();
    
    // 模拟动态场景
    std::cout << "\n模拟动态场景..." << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        
        // 更新船只位置
        for (auto& boat : boats) {
            double heading_rad = boat.heading * M_PI / 180.0;
            double distance = boat.speed * 1.0; // 1秒移动距离
            
            // 简化的位置更新（忽略地球曲率）
            boat.lat += distance * cos(heading_rad) / 111000.0; // 约111km/度
            boat.lng += distance * sin(heading_rad) / (111000.0 * cos(boat.lat * M_PI / 180.0));
            boat.timestamp += 1.0;
        }
        
        std::cout << "\n第 " << (i+1) << " 秒后的状态:" << std::endl;
        for (const auto& boat : boats) {
            std::cout << "船只" << boat.sysid << " 位置: (" 
                      << std::fixed << std::setprecision(6) << boat.lat << ", " << boat.lng << ")" << std::endl;
        }
        
        fleet_manager.updateBoatStates(boats);
        fleet_manager.performCollisionDetection();
    }
    
    // 保存结果到JSON文件
    Json::Value results(Json::arrayValue);
    for (const auto& boat : boats) {
        results.append(boat.toJson());
    }
    
    std::ofstream output_file("collision_results.json");
    if (output_file.is_open()) {
        Json::StreamWriterBuilder builder;
        builder["indentation"] = "  ";
        std::unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
        writer->write(results, &output_file);
        output_file.close();
        std::cout << "\n结果已保存到 collision_results.json" << std::endl;
    }
    
    std::cout << "\n系统运行完成！" << std::endl;
    return 0;
}
