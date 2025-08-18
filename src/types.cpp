// ==================== src/types.cpp ====================
#include "types.h"

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

// BoatState implementations
Json::Value BoatState::toJson() const {
    Json::Value json;
    json["sysid"] = sysid;
    json["timestamp"] = timestamp;
    json["lat"] = lat;
    json["lng"] = lng;
    json["heading"] = heading;
    json["speed"] = speed;
    json["status"] = static_cast<int>(status);
    json["route_direction"] = static_cast<int>(route_direction);
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
    boat.route_direction = static_cast<RouteDirection>(json["route_direction"].asInt());
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
    route_direction = static_cast<RouteDirection>(json["route_direction"].asInt());
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

// RouteInfo implementations
Json::Value RouteInfo::toJson() const {
    Json::Value json;
    json["route_id"] = route_id;
    json["direction"] = static_cast<int>(direction);
    
    Json::Value points_json(Json::arrayValue);
    for (const auto& point : points) {
        points_json.append(point.toJson());
    }
    json["points"] = points_json;
    
    return json;
}

RouteInfo RouteInfo::fromJson(const Json::Value& json) {
    RouteInfo route;
    route.route_id = json["route_id"].asInt();
    route.direction = static_cast<RouteDirection>(json["direction"].asInt());
    
    const Json::Value& points_json = json["points"];
    for (const auto& point_json : points_json) {
        route.points.push_back(GeoPoint::fromJson(point_json));
    }
    
    return route;
}

// SystemConfig implementations
Json::Value SystemConfig::toJson() const {
    Json::Value json;
    json["boat"]["length"] = boat.length;
    json["boat"]["width"] = boat.width;
    json["emergency_threshold_s"] = emergency_threshold_s;
    json["warning_threshold_s"] = warning_threshold_s;
    json["max_boats"] = max_boats;
    json["min_route_gap_m"] = min_route_gap_m;
    return json;
}

SystemConfig SystemConfig::fromJson(const Json::Value& json) {
    SystemConfig config;
    config.boat.length = json["boat"]["length"].asDouble();
    config.boat.width = json["boat"]["width"].asDouble();
    config.emergency_threshold_s = json["emergency_threshold_s"].asDouble();
    config.warning_threshold_s = json["warning_threshold_s"].asDouble();
    config.max_boats = json["max_boats"].asInt();
    config.min_route_gap_m = json["min_route_gap_m"].asDouble();
    return config;
}

// 实例方法版本
void SystemConfig::loadFromJson(const Json::Value& json) {
    boat.length = json["boat"]["length"].asDouble();
    boat.width = json["boat"]["width"].asDouble();
    emergency_threshold_s = json["emergency_threshold_s"].asDouble();
    warning_threshold_s = json["warning_threshold_s"].asDouble();
    max_boats = json["max_boats"].asInt();
    min_route_gap_m = json["min_route_gap_m"].asDouble();
}

SystemConfig SystemConfig::getDefault() {
    SystemConfig config;
    config.boat.length = 0.75;
    config.boat.width = 0.47;
    config.emergency_threshold_s = 5.0;
    config.warning_threshold_s = 30.0;
    config.max_boats = 30;
    config.min_route_gap_m = 10.0;
    return config;
}

// CollisionAlert implementations
Json::Value CollisionAlert::toJson() const {
    Json::Value json;
    json["level"] = static_cast<int>(level);
    json["current_boat_id"] = current_boat_id;
    
    Json::Value front_boats(Json::arrayValue);
    for (int id : front_boat_ids) {
        front_boats.append(id);
    }
    json["front_boat_ids"] = front_boats;
    
    Json::Value oncoming_boats(Json::arrayValue);
    for (int id : oncoming_boat_ids) {
        oncoming_boats.append(id);
    }
    json["oncoming_boat_ids"] = oncoming_boats;
    
    json["collision_position"] = collision_position.toJson();
    json["collision_time"] = collision_time;
    json["current_heading"] = current_heading;
    json["other_heading"] = other_heading;
    json["decision_advice"] = decision_advice;
    
    return json;
}

} // namespace boat_pro
