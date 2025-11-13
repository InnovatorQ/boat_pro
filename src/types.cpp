// ==================== src/types.cpp ====================
#include "types.h"
#include <cmath>
#include <ctime>

namespace boat_pro {

// GeoPoint implementation
Json::Value GeoPoint::toJson() const {
    Json::Value json;
    json["lat"] = lat;
    json["lng"] = lng;
    return json;
}

GeoPoint GeoPoint::fromJson(const Json::Value& json) {
    GeoPoint point;
    point.lat = json["lat"].asDouble();
    point.lng = json["lng"].asDouble();
    return point;
}

// BoatState implementation
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

void BoatState::loadFromJson(const Json::Value& json) {
    sysid = json["sysid"].asInt();
    timestamp = json["timestamp"].asDouble();
    lat = json["lat"].asDouble();
    lng = json["lng"].asDouble();
    heading = json["heading"].asDouble();
    speed = json["speed"].asDouble();
    status = static_cast<BoatStatus>(json["status"].asInt());
}

// DockInfo implementation
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

// RouteInfo implementation
Json::Value RouteInfo::toJson() const {
    Json::Value json;
    json["sysid"] = sysid;
    Json::Value points_array(Json::arrayValue);
    for (const auto& point : points) {
        points_array.append(point.toJson());
    }
    json["points"] = points_array;
    return json;
}

RouteInfo RouteInfo::fromJson(const Json::Value& json) {
    RouteInfo route;
    route.sysid = json["sysid"].asInt();
    const Json::Value& points_array = json["points"];
    for (const auto& point_json : points_array) {
        route.points.push_back(GeoPoint::fromJson(point_json));
    }
    return route;
}

// BoatDimensions implementation
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

// SystemConfig implementation
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

// CollisionAlert implementation
Json::Value CollisionAlert::toJson() const {
    Json::Value json;
    json["alert_level"] = static_cast<int>(level) + 1;
    json["avoidance_decision"] = static_cast<int>(avoidance_decision);
    json["current_boat_id"] = current_boat_id;
    json["collision_angle"] = collision_angle;
    json["collision_time"] = collision_time;
    json["collision_position"] = collision_position.toJson();
    
    Json::Value front_boats(Json::arrayValue);
    for (int id : front_boat_ids) {
        front_boats.append(id);
    }
    json["front_collision_boat_id"] = front_boats;
    
    Json::Value oncoming_boats(Json::arrayValue);
    for (int id : oncoming_boat_ids) {
        oncoming_boats.append(id);
    }
    json["oncoming_collision_boat_ids"] = oncoming_boats;
    
    if (!oncoming_boat_ids.empty()) {
        Json::Value oncoming_info;
        oncoming_info["current_boat_heading"] = current_heading;
        Json::Value oncoming_headings(Json::arrayValue);
        oncoming_headings.append(other_heading);
        oncoming_info["oncoming_boats_heading"] = oncoming_headings;
        json["oncoming_collision_info"] = oncoming_info;
    }
    
    json["timestamp"] = static_cast<int>(std::time(nullptr));
    return json;
}

} // namespace boat_pro
