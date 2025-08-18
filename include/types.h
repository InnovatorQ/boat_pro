// ==================== include/types.h ====================
#ifndef BOAT_PRO_TYPES_H
#define BOAT_PRO_TYPES_H

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

// 船只动态数据
struct BoatState {
    int sysid;                    // 船只系统ID
    double timestamp;             // 时间戳
    double lat;                   // 纬度
    double lng;                   // 经度
    double heading;               // 航向角
    double speed;                 // 速度 m/s
    BoatStatus status;            // 航行状态
    RouteDirection route_direction; // 航线方向
    
    Json::Value toJson() const;
    static BoatState fromJson(const Json::Value& json);
    void loadFromJson(const Json::Value& json);  // 实例方法版本
    GeoPoint getPosition() const { return GeoPoint(lat, lng); }
};

// 船坞静态数据
struct DockInfo {
    int dock_id;
    double lat;
    double lng;
    
    Json::Value toJson() const;
    static DockInfo fromJson(const Json::Value& json);
    GeoPoint getPosition() const { return GeoPoint(lat, lng); }
};

// 航线定义数据
struct RouteInfo {
    int route_id;
    RouteDirection direction;
    std::vector<GeoPoint> points;
    
    Json::Value toJson() const;
    static RouteInfo fromJson(const Json::Value& json);
};

// 系统配置
struct SystemConfig {
    struct {
        double length;  // 船只长度(米)
        double width;   // 船只宽度(米)
    } boat;
    
    double emergency_threshold_s;  // 紧急判断时间阈值(秒)
    double warning_threshold_s;    // 警告判断时间阈值(秒)
    int max_boats;                 // 最大船只数量
    double min_route_gap_m;        // 最小航线横向间距
    
    Json::Value toJson() const;
    static SystemConfig fromJson(const Json::Value& json);
    void loadFromJson(const Json::Value& json);  // 实例方法版本
    static SystemConfig getDefault();
};

// 碰撞告警信息
struct CollisionAlert {
    AlertLevel level;              // 紧急程度
    int current_boat_id;           // 当前船ID
    std::vector<int> front_boat_ids;    // 前向被碰撞船ID列表
    std::vector<int> oncoming_boat_ids; // 对向被碰撞船ID列表
    GeoPoint collision_position;   // 预计碰撞位置
    double collision_time;         // 预计碰撞时间(秒)
    double current_heading;        // 当前船航向
    double other_heading;          // 对方船航向(对向碰撞时)
    std::string decision_advice;   // 避碰决策建议
    
    Json::Value toJson() const;
};

} // namespace boat_pro

#endif