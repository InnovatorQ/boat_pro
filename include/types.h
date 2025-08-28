// ==================== include/types.h ====================
#ifndef BOAT_PRO_TYPES_H
#define BOAT_PRO_TYPES_H

#include "avoidance_decision_types.h"
#include <jsoncpp/json/json.h>  // jsoncpp header
#include <vector>
#include <string>
#include <memory>

namespace boat_pro {

// 船只状态枚举
enum class BoatStatus : int {
    UNDOCKING = 1,    // 出坞
    NORMAL_SAIL = 2,  // 正常航行
    DOCKING = 3       // 入坞
};

// 航线方向枚举
enum class RouteDirection : int {
    CLOCKWISE = 1,        // 顺时针
    COUNTERCLOCKWISE = 2  // 逆时针
};

// 碰撞紧急程度枚举
enum class AlertLevel {
    NORMAL = 0,    // 正常
    WARNING = 1,   // 警告
    EMERGENCY = 2  // 紧急
};

// 地理坐标点
struct GeoPoint {
    double lat;  // 纬度
    double lng;  // 经度
    
    GeoPoint() : lat(0.0), lng(0.0) {}
    GeoPoint(double latitude, double longitude) : lat(latitude), lng(longitude) {}
    
    Json::Value toJson() const;
    static GeoPoint fromJson(const Json::Value& json);
};

// 船只动态数据 - 适配C# mqtt_mpc_BoatState
struct BoatState {
    int sysid;                    // 船只系统ID
    double timestamp;             // 时间戳（UTC或本机时间，单位：秒）
    double lat;                   // 纬度（WGS84）
    double lng;                   // 经度（WGS84）
    double heading;               // 航向角（0°为正北，顺时针增加）
    double speed;                 // 船速，单位：米/秒
    BoatStatus status;            // 航行状态（1-出坞，2-正常航行，3-入坞）
    
    Json::Value toJson() const;
    static BoatState fromJson(const Json::Value& json);
    void loadFromJson(const Json::Value& json);  // 实例方法版本
    GeoPoint getPosition() const { return GeoPoint(lat, lng); }
};

// 船坞静态数据 - 适配C# mqtt_mpc_DockInfo
struct DockInfo {
    int dock_id;                  // 船坞ID（与停靠boat sysid相同）
    double lat;                   // 纬度（WGS84）
    double lng;                   // 经度（WGS84）
    
    Json::Value toJson() const;
    static DockInfo fromJson(const Json::Value& json);
    GeoPoint getPosition() const { return GeoPoint(lat, lng); }
};

// 航线定义数据 - 适配C# mqtt_mpc_RouteInfo
struct RouteInfo {
    int sysid;                    // 船只ID
    std::vector<GeoPoint> points; // 航线点列表（可包含任意个数的点，max 500）
    
    Json::Value toJson() const;
    static RouteInfo fromJson(const Json::Value& json);
};

// 船只尺寸信息 - 适配C# BoatDimensions
struct BoatDimensions {
    double length;                // 船只长度（单位：米）
    double width;                 // 船只宽度（单位：米）
    
    Json::Value toJson() const;
    static BoatDimensions fromJson(const Json::Value& json);
};

// 系统配置 - 适配C# mqtt_mpc_SystemConfiguration
struct SystemConfig {
    BoatDimensions boat;          // 船只尺寸信息
    int emergency_threshold_s;    // 紧急判断时间阈值（秒）
    int warning_threshold_s;      // 警告判断时间阈值（秒）
    int max_boats;                // 最大船只数量
    int min_route_gap_m;          // 最小航线横向间距（米）
    
    Json::Value toJson() const;
    static SystemConfig fromJson(const Json::Value& json);
    void loadFromJson(const Json::Value& json);  // 实例方法版本
    static SystemConfig getDefault();
};

// 碰撞类型枚举
enum class CollisionType {
    OVERTAKING,      // 追尾碰撞 (0-30°)
    OBLIQUE,         // 斜向碰撞 (30-60°)
    CROSSING,        // 交叉碰撞 (60-120°)
    OBLIQUE_HEAD_ON, // 斜向对撞 (120-150°)
    HEAD_ON          // 正面对撞 (150-180°)
};

// 碰撞告警信息
struct CollisionAlert {
    AlertLevel level;              // 紧急程度
    int current_boat_id;           // 当前船ID
    double collision_angle;        // 碰撞角度 (度，0-180°范围)
    CollisionType collision_type;  // 碰撞类型
    std::vector<int> front_boat_ids;    // 前向被碰撞船ID列表
    std::vector<int> oncoming_boat_ids; // 对向被碰撞船ID列表
    GeoPoint collision_position;   // 预计碰撞位置
    double collision_time;         // 预计碰撞时间(秒)
    double current_heading;        // 当前船航向
    double other_heading;          // 对方船航向(对向碰撞时)
    AvoidanceDecision avoidance_decision; // 避碰决策枚举
    
    // 便利方法：获取决策代码
    int getDecisionCode() const {
        return static_cast<int>(avoidance_decision);
    }
    
    // 便利方法：设置决策代码
    void setDecisionCode(int code) {
        avoidance_decision = static_cast<AvoidanceDecision>(code);
    }
    
    // 便利方法：获取决策描述
    std::string getDecisionDescription() const {
        return AvoidanceDecisionMapper::getChineseDescription(avoidance_decision);
    }
    
    // 碰撞角度计算
    static double calculateCollisionAngle(double heading1, double heading2);
    
    // 碰撞类型判断
    static CollisionType determineCollisionType(double angle);
    
    // 碰撞类型转字符串
    static std::string collisionTypeToString(CollisionType type);
    
    Json::Value toJson() const;
};

} // namespace boat_pro

#endif