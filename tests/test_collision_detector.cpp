#include "../src/collision_detector.cpp"
#include "../src/types.cpp"
#include "../src/geometry_utils.cpp"
#include <iostream>
#include <cassert>

using namespace boat_pro;

void testGeometryUtils() {
    std::cout << "测试几何工具函数..." << std::endl;
    
    // 测试距离计算
    GeoPoint p1(30.549832, 114.342922);
    GeoPoint p2(30.549900, 114.343100);
    double distance = geometry::calculateDistance(p1, p2);
    
    std::cout << "距离: " << distance << " 米" << std::endl;
    assert(distance > 0 && distance < 100); // 应该是几十米的距离
    
    // 测试方位角计算
    double bearing = geometry::calculateBearing(p1, p2);
    std::cout << "方位角: " << bearing << " 度" << std::endl;
    assert(bearing >= 0 && bearing < 360);
    
    std::cout << "几何工具函数测试通过!" << std::endl;
}

void testCollisionDetector() {
    std::cout << "测试碰撞检测器..." << std::endl;
    
    SystemConfig config = SystemConfig::getDefault();
    CollisionDetector detector(config);
    
    // 创建测试船只
    std::vector<BoatState> boats;
    
    BoatState boat1;
    boat1.sysid = 1;
    boat1.lat = 30.549832;
    boat1.lng = 114.342922;
    boat1.heading = 90.0;
    boat1.speed = 3.0;
    boat1.status = BoatStatus::NORMAL_SAIL;
    boat1.route_direction = RouteDirection::CLOCKWISE;
    boats.push_back(boat1);
    
    BoatState boat2;
    boat2.sysid = 2;
    boat2.lat = 30.549840; // 非常接近
    boat2.lng = 114.342930;
    boat2.heading = 270.0; // 相向而行
    boat2.speed = 2.0;
    boat2.status = BoatStatus::NORMAL_SAIL;
    boat2.route_direction = RouteDirection::COUNTERCLOCKWISE;
    boats.push_back(boat2);
    
    detector.updateBoatStates(boats);
    
    auto alerts = detector.detectCollisions();
    std::cout << "检测到 " << alerts.size() << " 个告警" << std::endl;
    
    for (const auto& alert : alerts) {
        std::cout << "告警 - 船只: " << alert.current_boat_id 
                  << ", 等级: " << static_cast<int>(alert.level) << std::endl;
    }
    
    std::cout << "碰撞检测器测试完成!" << std::endl;
}

int main() {
    std::cout << "开始运行测试..." << std::endl;
    
    try {
        testGeometryUtils();
        testCollisionDetector();
        std::cout << "所有测试通过!" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "测试失败: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
