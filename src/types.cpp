// ==================== src/types.cpp ====================
#include "types.h"
#include <cmath>

namespace boat_pro {

// GeoPoint implementations
Json::Value GeoPoint::toJson() const {
    Json::Value json;
    json["lat"] = lat;
    json["lng"] = lng;
    return json;
}

GeoPoint GeoPoint::fromJson(const Json::Value& json) {
    return GeoPoint(
        json["lat"].asDouble(),
        json["lng"].asDouble()
    );
}

// BoatState implementations - 适配C# mqtt_mpc_BoatState
Json::Value BoatState::toJson() const {
    Json::Value json;
    json["sysid"] = sysid;
    json["timestamp"] = timestamp;
    json["lat"] = lat;
    json["lng"] = lng;
    json["heading"] = heading;
    json["speed"] = speed;
    json["status"] = static_cast<int>(status);
    return json;
}

BoatState BoatState::fromJson(const Json::Value& json) {
    BoatState boat;
    boat.sysid = json["sysid"].asInt();
    boat.timestamp = json["timestamp"].asDouble();
    boat.lat = json["lat"].asDouble();
    boat.lng = json["lng"].asDouble();
    boat.heading = json["heading"].asDouble();
    boat.speed = json["speed"].asDouble();
    boat.status = static_cast<BoatStatus>(json["status"].asInt());
    return boat;
}

// 实例方法版本
void BoatState::loadFromJson(const Json::Value& json) {
    sysid = json["sysid"].asInt();
    timestamp = json["timestamp"].asDouble();
    lat = json["lat"].asDouble();
    lng = json["lng"].asDouble();
    heading = json["heading"].asDouble();
    speed = json["speed"].asDouble();
    status = static_cast<BoatStatus>(json["status"].asInt());
}
    timestamp = json["timestamp"].asDouble();
    lat = json["lat"].asDouble();
    lng = json["lng"].asDouble();
    heading = json["heading"].asDouble();
    speed = json["speed"].asDouble();
    status = static_cast<BoatStatus>(json["status"].asInt());
    // 注意：新的数据格式不包含route_direction字段
}

// DockInfo implementations
Json::Value DockInfo::toJson() const {
    Json::Value json;
    json["dock_id"] = dock_id;
    json["lat"] = lat;
    json["lng"] = lng;
    return json;
}

DockInfo DockInfo::fromJson(const Json::Value& json) {
    DockInfo dock;
    dock.dock_id = json["dock_id"].asInt();
    dock.lat = json["lat"].asDouble();
    dock.lng = json["lng"].asDouble();
    return dock;
}

// RouteInfo implementations - 适配C# mqtt_mpc_RouteInfo
Json::Value RouteInfo::toJson() const {
    Json::Value json;
    json["sysid"] = sysid;
    
    Json::Value points_json(Json::arrayValue);
    for (const auto& point : points) {
        points_json.append(point.toJson());
    }
    json["points"] = points_json;
    
    return json;
}

RouteInfo RouteInfo::fromJson(const Json::Value& json) {
    RouteInfo route;
    route.sysid = json["sysid"].asInt();
    
    const Json::Value& points_json = json["points"];
    for (const auto& point_json : points_json) {
        route.points.push_back(GeoPoint::fromJson(point_json));
    }
    
    return route;
}

// BoatDimensions implementations - 适配C# BoatDimensions
Json::Value BoatDimensions::toJson() const {
    Json::Value json;
    json["length"] = length;
    json["width"] = width;
    return json;
}

BoatDimensions BoatDimensions::fromJson(const Json::Value& json) {
    BoatDimensions dimensions;
    dimensions.length = json["length"].asDouble();
    dimensions.width = json["width"].asDouble();
    return dimensions;
}

// SystemConfig implementations - 适配C# mqtt_mpc_SystemConfiguration
Json::Value SystemConfig::toJson() const {
    Json::Value json;
    json["boat"] = boat.toJson();
    json["emergency_threshold_s"] = emergency_threshold_s;
    json["warning_threshold_s"] = warning_threshold_s;
    json["max_boats"] = max_boats;
    json["min_route_gap_m"] = min_route_gap_m;
    return json;
}

SystemConfig SystemConfig::fromJson(const Json::Value& json) {
    SystemConfig config;
    config.boat = BoatDimensions::fromJson(json["boat"]);
    config.emergency_threshold_s = json["emergency_threshold_s"].asInt();
    config.warning_threshold_s = json["warning_threshold_s"].asInt();
    config.max_boats = json["max_boats"].asInt();
    config.min_route_gap_m = json["min_route_gap_m"].asInt();
    return config;
}

// 实例方法版本
void SystemConfig::loadFromJson(const Json::Value& json) {
    boat = BoatDimensions::fromJson(json["boat"]);
    emergency_threshold_s = json["emergency_threshold_s"].asInt();
    warning_threshold_s = json["warning_threshold_s"].asInt();
    max_boats = json["max_boats"].asInt();
    min_route_gap_m = json["min_route_gap_m"].asInt();
}

SystemConfig SystemConfig::getDefault() {
    SystemConfig config;
    config.boat.length = 0.75;
    config.boat.width = 0.47;
    config.emergency_threshold_s = 5;
    config.warning_threshold_s = 30;
    config.max_boats = 30;
    config.min_route_gap_m = 10;
    return config;
}

// CollisionAlert implementations
Json::Value CollisionAlert::toJson() const {
    Json::Value json;
    json["alert_level"] = static_cast<int>(level) + 1; // 转换为1-3范围
    json["avoidance_decision"] = static_cast<int>(avoidance_decision);
    json["current_boat_id"] = current_boat_id;
    json["collision_angle"] = collision_angle;
    json["collision_type"] = collisionTypeToString(collision_type);
    
    // 前向碰撞船只ID (仅最近的一个，如果有的话)
    if (!front_boat_ids.empty()) {
        json["front_collision_boat_id"] = front_boat_ids[0];
    } else {
        json["front_collision_boat_id"] = Json::Value::null;
    }
    
    // 对向碰撞船只ID列表
    Json::Value oncoming_boats(Json::arrayValue);
    for (int id : oncoming_boat_ids) {
        oncoming_boats.append(id);
    }
    json["oncoming_collision_boat_ids"] = oncoming_boats;
    
    json["collision_position"] = collision_position.toJson();
    json["collision_time"] = collision_time;
    
    // 对向碰撞信息 (仅在有对向碰撞时包含)
    if (!oncoming_boat_ids.empty()) {
        Json::Value oncoming_info;
        oncoming_info["current_boat_heading"] = current_heading;
        
        Json::Value oncoming_headings(Json::arrayValue);
        oncoming_headings.append(other_heading); // 简化处理，实际应该是数组
        oncoming_info["oncoming_boats_heading"] = oncoming_headings;
        
        json["oncoming_collision_info"] = oncoming_info;
    }
    
    json["timestamp"] = static_cast<long>(collision_time); // 简化处理
    
    return json;
}

// 碰撞角度计算
double CollisionAlert::calculateCollisionAngle(double heading1, double heading2) {
    double angle = std::abs(heading1 - heading2);
    if (angle > 180.0) {
        angle = 360.0 - angle;  // 取较小的角度
    }
    return angle;
}

// 碰撞类型判断
CollisionType CollisionAlert::determineCollisionType(double angle) {
    if (angle <= 30.0) {
        return CollisionType::OVERTAKING;
    } else if (angle <= 60.0) {
        return CollisionType::OBLIQUE;
    } else if (angle <= 120.0) {
        return CollisionType::CROSSING;
    } else if (angle <= 150.0) {
        return CollisionType::OBLIQUE_HEAD_ON;
    } else {
        return CollisionType::HEAD_ON;
    }
}

// 碰撞类型转字符串
std::string CollisionAlert::collisionTypeToString(CollisionType type) {
    switch (type) {
        case CollisionType::OVERTAKING:
            return "overtaking";
        case CollisionType::OBLIQUE:
            return "oblique";
        case CollisionType::CROSSING:
            return "crossing";
        case CollisionType::OBLIQUE_HEAD_ON:
            return "oblique_head_on";
        case CollisionType::HEAD_ON:
            return "head_on";
        default:
            return "unknown";
    }
}

} // namespace boat_pro
