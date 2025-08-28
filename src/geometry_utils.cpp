// ==================== src/geometry_utils.cpp ====================
#include "geometry_utils.h"
#include <cmath>
#include <algorithm>

namespace boat_pro {
namespace geometry {

double calculateDistance(const GeoPoint& p1, const GeoPoint& p2) {
    double lat1_rad = toRadians(p1.lat);
    double lat2_rad = toRadians(p2.lat);
    double delta_lat = toRadians(p2.lat - p1.lat);
    double delta_lng = toRadians(p2.lng - p1.lng);
    
    double a = std::sin(delta_lat / 2) * std::sin(delta_lat / 2) +
               std::cos(lat1_rad) * std::cos(lat2_rad) *
               std::sin(delta_lng / 2) * std::sin(delta_lng / 2);
    
    double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
    
    return EARTH_RADIUS * c;
}

double calculateBearing(const GeoPoint& from, const GeoPoint& to) {
    double lat1_rad = toRadians(from.lat);
    double lat2_rad = toRadians(to.lat);
    double delta_lng = toRadians(to.lng - from.lng);
    
    double y = std::sin(delta_lng) * std::cos(lat2_rad);
    double x = std::cos(lat1_rad) * std::sin(lat2_rad) -
               std::sin(lat1_rad) * std::cos(lat2_rad) * std::cos(delta_lng);
    
    double bearing = toDegrees(std::atan2(y, x));
    return normalizeAngle(bearing);
}

GeoPoint calculateDestination(const GeoPoint& start, double bearing, double distance) {
    double lat1_rad = toRadians(start.lat);
    double lng1_rad = toRadians(start.lng);
    double bearing_rad = toRadians(bearing);
    double angular_distance = distance / EARTH_RADIUS;
    
    double lat2_rad = std::asin(std::sin(lat1_rad) * std::cos(angular_distance) +
                                std::cos(lat1_rad) * std::sin(angular_distance) * 
                                std::cos(bearing_rad));
    
    double lng2_rad = lng1_rad + std::atan2(std::sin(bearing_rad) * 
                                            std::sin(angular_distance) * std::cos(lat1_rad),
                                            std::cos(angular_distance) - 
                                            std::sin(lat1_rad) * std::sin(lat2_rad));
    
    return GeoPoint(toDegrees(lat2_rad), toDegrees(lng2_rad));
}

double normalizeAngle(double angle) {
    while (angle < 0) angle += 360.0;
    while (angle >= 360.0) angle -= 360.0;
    return angle;
}

double angleDifference(double angle1, double angle2) {
    double diff = std::abs(angle1 - angle2);
    return std::min(diff, 360.0 - diff);
}

double pointToLineDistance(const GeoPoint& point, const GeoPoint& lineStart, const GeoPoint& lineEnd) {
    double A = calculateDistance(point, lineStart);
    double B = calculateDistance(point, lineEnd);
    double C = calculateDistance(lineStart, lineEnd);
    
    if (C == 0) return A; // 线段退化为点
    
    // 使用海伦公式计算三角形面积，然后求高
    double s = (A + B + C) / 2;
    double area = std::sqrt(s * (s - A) * (s - B) * (s - C));
    return 2 * area / C;
}

double calculateCollisionTime(const GeoPoint& pos1, const GeoPoint& vel1,
                             const GeoPoint& pos2, const GeoPoint& vel2,
                             double radius) {
    // 将地理坐标转换为局部笛卡尔坐标系进行计算
    // 计算两船之间的相对位置 (米)
    double dx = calculateDistance(GeoPoint(pos1.lat, pos1.lng), GeoPoint(pos1.lat, pos2.lng));
    if (pos2.lng < pos1.lng) dx = -dx;
    
    double dy = calculateDistance(GeoPoint(pos1.lat, pos1.lng), GeoPoint(pos2.lat, pos1.lng));
    if (pos2.lat < pos1.lat) dy = -dy;
    
    // vel1和vel2应该是速度向量，这里假设它们已经是笛卡尔坐标系下的速度分量
    // 如果传入的是基于航向和速度的向量，需要转换
    double dvx = vel2.lng - vel1.lng;  // 相对速度x分量
    double dvy = vel2.lat - vel1.lat;  // 相对速度y分量
    
    // 求解二次方程: |p1 + v1*t - p2 - v2*t|^2 = radius^2
    // 简化为: |relative_pos + relative_vel*t|^2 = radius^2
    double a = dvx * dvx + dvy * dvy;
    double b = 2 * (dx * dvx + dy * dvy);
    double c = dx * dx + dy * dy - radius * radius;
    
    // 如果相对速度为0，检查当前距离
    if (a < 1e-6) {
        if (c <= 0) return 0; // 已经在碰撞范围内
        return -1; // 静止状态，不会碰撞
    }
    
    double discriminant = b * b - 4 * a * c;
    
    if (discriminant < 0) {
        return -1; // 无碰撞
    }
    
    double t1 = (-b - std::sqrt(discriminant)) / (2 * a);
    double t2 = (-b + std::sqrt(discriminant)) / (2 * a);
    
    // 返回最小的正值时间
    if (t1 > 0.1) return t1;  // 至少0.1秒后
    if (t2 > 0.1) return t2;
    
    return -1; // 无有效碰撞时间
}

} // namespace geometry
} // namespace boat_pro