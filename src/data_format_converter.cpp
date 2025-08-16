#include "data_format_converter.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <regex>
#include <ctime>

DataFormatConverter::DataFormatConverter() {
    // 初始化状态映射
    statusMapping["DOCKED"] = 1;
    statusMapping["SAILING"] = 2;
    statusMapping["DOCKING"] = 3;
    statusMapping["UNDOCKING"] = 1;
    statusMapping["ANCHORED"] = 1;
    statusMapping["MOORED"] = 1;
    
    // 初始化航线映射
    routeMapping["ROUTE_A"] = 1;
    routeMapping["ROUTE_B"] = 2;
    routeMapping["CLOCKWISE"] = 1;
    routeMapping["COUNTERCLOCKWISE"] = 2;
    routeMapping["CW"] = 1;
    routeMapping["CCW"] = 2;
}

Json::Value DataFormatConverter::convertBoatState(const Json::Value& clientData) {
    Json::Value standardFormat;
    
    try {
        // 船只ID转换
        if (clientData.isMember("vessel_id")) {
            std::string vessel_id = clientData["vessel_id"].asString();
            standardFormat["sysid"] = extractBoatId(vessel_id);
        } else if (clientData.isMember("boat_id")) {
            standardFormat["sysid"] = clientData["boat_id"].asInt();
        } else if (clientData.isMember("id")) {
            standardFormat["sysid"] = clientData["id"].asInt();
        }
        
        // 时间戳转换
        if (clientData.isMember("timestamp")) {
            standardFormat["timestamp"] = convertTimestamp(clientData["timestamp"]);
        } else {
            standardFormat["timestamp"] = std::time(nullptr);
        }
        
        // 位置信息转换
        if (clientData.isMember("position")) {
            const Json::Value& pos = clientData["position"];
            standardFormat["lat"] = pos["latitude"].asDouble();
            standardFormat["lng"] = pos["longitude"].asDouble();
        } else if (clientData.isMember("location")) {
            const Json::Value& loc = clientData["location"];
            standardFormat["lat"] = loc["lat"].asDouble();
            standardFormat["lng"] = loc["lng"].asDouble();
        } else {
            // 直接字段
            standardFormat["lat"] = clientData["lat"].asDouble();
            standardFormat["lng"] = clientData["lng"].asDouble();
        }
        
        // 导航信息转换
        if (clientData.isMember("navigation")) {
            const Json::Value& nav = clientData["navigation"];
            
            // 航向
            if (nav.isMember("heading")) {
                standardFormat["heading"] = nav["heading"].asDouble();
            } else if (nav.isMember("course")) {
                standardFormat["heading"] = nav["course"].asDouble();
            }
            
            // 速度转换
            if (nav.isMember("speed_knots")) {
                double speed_knots = nav["speed_knots"].asDouble();
                standardFormat["speed"] = knotsToMeterPerSecond(speed_knots);
            } else if (nav.isMember("speed_ms")) {
                standardFormat["speed"] = nav["speed_ms"].asDouble();
            } else if (nav.isMember("speed")) {
                // 假设默认单位是m/s
                standardFormat["speed"] = nav["speed"].asDouble();
            }
        } else {
            // 直接字段
            standardFormat["heading"] = clientData.get("heading", 0.0).asDouble();
            standardFormat["speed"] = clientData.get("speed", 0.0).asDouble();
        }
        
        // 状态转换
        if (clientData.isMember("status")) {
            if (clientData["status"].isString()) {
                std::string status_str = clientData["status"].asString();
                standardFormat["status"] = mapStatus(status_str);
            } else {
                standardFormat["status"] = clientData["status"].asInt();
            }
        } else if (clientData.isMember("vessel_status")) {
            std::string status_str = clientData["vessel_status"].asString();
            standardFormat["status"] = mapStatus(status_str);
        } else {
            standardFormat["status"] = 2; // 默认为航行状态
        }
        
        // 航线方向转换
        if (clientData.isMember("route")) {
            if (clientData["route"].isString()) {
                std::string route_str = clientData["route"].asString();
                standardFormat["route_direction"] = mapRoute(route_str);
            } else {
                standardFormat["route_direction"] = clientData["route"].asInt();
            }
        } else if (clientData.isMember("route_direction")) {
            standardFormat["route_direction"] = clientData["route_direction"].asInt();
        } else {
            standardFormat["route_direction"] = 1; // 默认顺时针
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error converting boat state data: " << e.what() << std::endl;
        return Json::Value(); // 返回空值表示转换失败
    }
    
    return standardFormat;
}

Json::Value DataFormatConverter::convertDockInfo(const Json::Value& clientData) {
    Json::Value standardFormat;
    
    try {
        // 船坞ID
        if (clientData.isMember("dock_id")) {
            standardFormat["dock_id"] = clientData["dock_id"].asInt();
        } else if (clientData.isMember("berth_id")) {
            standardFormat["dock_id"] = clientData["berth_id"].asInt();
        } else if (clientData.isMember("id")) {
            standardFormat["dock_id"] = clientData["id"].asInt();
        }
        
        // 位置信息
        if (clientData.isMember("position")) {
            const Json::Value& pos = clientData["position"];
            standardFormat["lat"] = pos["latitude"].asDouble();
            standardFormat["lng"] = pos["longitude"].asDouble();
        } else {
            standardFormat["lat"] = clientData["lat"].asDouble();
            standardFormat["lng"] = clientData["lng"].asDouble();
        }
        
        // 可选字段：占用状态
        if (clientData.isMember("occupied")) {
            standardFormat["occupied"] = clientData["occupied"].asBool();
        }
        
        if (clientData.isMember("boat_id") && !clientData["boat_id"].isNull()) {
            standardFormat["boat_id"] = clientData["boat_id"].asInt();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error converting dock info data: " << e.what() << std::endl;
        return Json::Value();
    }
    
    return standardFormat;
}

Json::Value DataFormatConverter::convertRouteInfo(const Json::Value& clientData) {
    Json::Value standardFormat;
    
    try {
        // 航线ID
        if (clientData.isMember("route_id")) {
            standardFormat["route_id"] = clientData["route_id"].asInt();
        } else if (clientData.isMember("id")) {
            standardFormat["route_id"] = clientData["id"].asInt();
        }
        
        // 方向
        if (clientData.isMember("direction")) {
            if (clientData["direction"].isString()) {
                std::string dir_str = clientData["direction"].asString();
                standardFormat["direction"] = mapRoute(dir_str);
            } else {
                standardFormat["direction"] = clientData["direction"].asInt();
            }
        }
        
        // 航线点
        if (clientData.isMember("waypoints")) {
            standardFormat["points"] = convertWaypoints(clientData["waypoints"]);
        } else if (clientData.isMember("points")) {
            standardFormat["points"] = convertWaypoints(clientData["points"]);
        } else if (clientData.isMember("coordinates")) {
            standardFormat["points"] = convertWaypoints(clientData["coordinates"]);
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error converting route info data: " << e.what() << std::endl;
        return Json::Value();
    }
    
    return standardFormat;
}

// 私有辅助函数实现

int DataFormatConverter::extractBoatId(const std::string& vessel_id) {
    // 从字符串中提取数字ID
    std::regex id_regex(R"(\d+)");
    std::smatch match;
    
    if (std::regex_search(vessel_id, match, id_regex)) {
        return std::stoi(match[0].str());
    }
    
    // 如果没有找到数字，使用哈希值
    return std::hash<std::string>{}(vessel_id) % 1000 + 1;
}

double DataFormatConverter::convertTimestamp(const Json::Value& timestamp) {
    if (timestamp.isString()) {
        // ISO 8601格式转换
        return convertISOToUnixTimestamp(timestamp.asString());
    } else if (timestamp.isDouble()) {
        return timestamp.asDouble();
    } else if (timestamp.isInt64()) {
        return static_cast<double>(timestamp.asInt64());
    }
    
    return std::time(nullptr);
}

double DataFormatConverter::convertISOToUnixTimestamp(const std::string& iso_time) {
    // 简化的ISO 8601解析
    std::tm tm = {};
    std::istringstream ss(iso_time);
    
    // 尝试解析 YYYY-MM-DDTHH:MM:SSZ 格式
    if (ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S")) {
        return std::mktime(&tm);
    }
    
    // 如果解析失败，返回当前时间
    return std::time(nullptr);
}

double DataFormatConverter::knotsToMeterPerSecond(double knots) {
    return knots * 0.514444; // 1节 = 0.514444米/秒
}

int DataFormatConverter::mapStatus(const std::string& status) {
    std::string upper_status = status;
    std::transform(upper_status.begin(), upper_status.end(), upper_status.begin(), ::toupper);
    
    auto it = statusMapping.find(upper_status);
    if (it != statusMapping.end()) {
        return it->second;
    }
    
    // 默认返回航行状态
    return 2;
}

int DataFormatConverter::mapRoute(const std::string& route) {
    std::string upper_route = route;
    std::transform(upper_route.begin(), upper_route.end(), upper_route.begin(), ::toupper);
    
    auto it = routeMapping.find(upper_route);
    if (it != routeMapping.end()) {
        return it->second;
    }
    
    // 默认返回顺时针
    return 1;
}

Json::Value DataFormatConverter::convertWaypoints(const Json::Value& waypoints) {
    Json::Value points(Json::arrayValue);
    
    for (const auto& waypoint : waypoints) {
        Json::Value point;
        
        if (waypoint.isMember("latitude") && waypoint.isMember("longitude")) {
            point["lat"] = waypoint["latitude"].asDouble();
            point["lng"] = waypoint["longitude"].asDouble();
        } else if (waypoint.isMember("lat") && waypoint.isMember("lng")) {
            point["lat"] = waypoint["lat"].asDouble();
            point["lng"] = waypoint["lng"].asDouble();
        } else if (waypoint.isArray() && waypoint.size() >= 2) {
            // [lat, lng] 格式
            point["lat"] = waypoint[0].asDouble();
            point["lng"] = waypoint[1].asDouble();
        }
        
        if (!point.empty()) {
            points.append(point);
        }
    }
    
    return points;
}
