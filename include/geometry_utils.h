// ==================== include/geometry_utils.h ====================
#ifndef BOAT_PRO_GEOMETRY_UTILS_H
#define BOAT_PRO_GEOMETRY_UTILS_H

#include "types.h"
#include <cmath>

namespace boat_pro {
namespace geometry {

// 地球半径 (米)
constexpr double EARTH_RADIUS = 6371000.0;

/**
 * 计算两个地理坐标点之间的距离 (米)
 * 使用Haversine公式
 */
double calculateDistance(const GeoPoint& p1, const GeoPoint& p2);

/**
 * 计算两个地理坐标点之间的方位角 (度)
 * 0度为正北，顺时针增加
 */
double calculateBearing(const GeoPoint& from, const GeoPoint& to);

/**
 * 根据起始点、方位角和距离计算目标点
 */
GeoPoint calculateDestination(const GeoPoint& start, double bearing, double distance);

/**
 * 将度数转换为弧度
 */
inline double toRadians(double degrees) {
    return degrees * M_PI / 180.0;
}

/**
 * 将弧度转换为度数
 */
inline double toDegrees(double radians) {
    return radians * 180.0 / M_PI;
}

/**
 * 标准化角度到[0, 360)范围
 */
double normalizeAngle(double angle);

/**
 * 计算两个角度之间的最小差值
 */
double angleDifference(double angle1, double angle2);

/**
 * 计算点到线段的最短距离
 */
double pointToLineDistance(const GeoPoint& point, const GeoPoint& lineStart, const GeoPoint& lineEnd);

/**
 * 判断两个运动物体是否会发生碰撞
 * @param pos1 物体1当前位置
 * @param vel1 物体1速度向量(m/s)
 * @param pos2 物体2当前位置  
 * @param vel2 物体2速度向量(m/s)
 * @param radius 碰撞半径(m)
 * @return 碰撞时间(秒)，如果不会碰撞返回-1
 */
double calculateCollisionTime(const GeoPoint& pos1, const GeoPoint& vel1,
                             const GeoPoint& pos2, const GeoPoint& vel2,
                             double radius);

} // namespace geometry
} // namespace boat_pro

#endif