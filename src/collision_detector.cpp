// ==================== src/collision_detector.cpp ====================
#include "collision_detector.h"
#include "geometry_utils.h"
#include <algorithm>
#include <sstream>

namespace boat_pro {

CollisionDetector::CollisionDetector(const SystemConfig& config) 
    : config_(config) {
}

void CollisionDetector::updateBoatStates(const std::vector<BoatState>& boats) {
    boat_states_.clear();
    for (const auto& boat : boats) {
        boat_states_[boat.sysid] = boat;
    }
}

void CollisionDetector::setDockInfo(const std::vector<DockInfo>& docks) {
    dock_info_ = docks;
}

void CollisionDetector::setRouteInfo(const std::vector<RouteInfo>& routes) {
    route_info_ = routes;
}

std::vector<CollisionAlert> CollisionDetector::detectCollisions() {
    std::vector<CollisionAlert> alerts;
    
    // 检测各类型碰撞
    auto undocking_alerts = detectUndockingCollisions();
    auto docking_alerts = detectDockingCollisions();
    auto following_alerts = detectFollowingCollisions();
    auto oncoming_alerts = detectOncomingCollisions();
    
    // 合并所有告警
    alerts.insert(alerts.end(), undocking_alerts.begin(), undocking_alerts.end());
    alerts.insert(alerts.end(), docking_alerts.begin(), docking_alerts.end());
    alerts.insert(alerts.end(), following_alerts.begin(), following_alerts.end());
    alerts.insert(alerts.end(), oncoming_alerts.begin(), oncoming_alerts.end());
    
    return alerts;
}

std::vector<CollisionAlert> CollisionDetector::detectUndockingCollisions() {
    std::vector<CollisionAlert> alerts;
    
    // 查找所有出坞状态的船只
    for (const auto& [boat_id, boat] : boat_states_) {
        if (boat.status != BoatStatus::UNDOCKING) continue;
        
        CollisionAlert alert;
        alert.current_boat_id = boat_id;
        alert.current_heading = boat.heading;
        alert.level = AlertLevel::NORMAL;
        
        double min_collision_time = std::numeric_limits<double>::max();
        GeoPoint predicted_collision_pos;
        
        // 检查与其他船只的碰撞风险
        for (const auto& [other_id, other_boat] : boat_states_) {
            if (other_id == boat_id) continue;
            
            // 计算碰撞时间
            GeoPoint boat_vel = geometry::calculateDestination(
                GeoPoint(0, 0), boat.heading, boat.speed);
            GeoPoint other_vel = geometry::calculateDestination(
                GeoPoint(0, 0), other_boat.heading, other_boat.speed);
                
            double collision_time = geometry::calculateCollisionTime(
                boat.getPosition(), boat_vel,
                other_boat.getPosition(), other_vel,
                getCollisionRadius()
            );
            
            if (collision_time > 0 && collision_time < min_collision_time) {
                min_collision_time = collision_time;
                
                // 计算碰撞位置
                predicted_collision_pos = geometry::calculateDestination(
                    boat.getPosition(), boat.heading, boat.speed * collision_time);
                
                // 根据优先级判断
                if (other_boat.status == BoatStatus::DOCKING ||
                    other_boat.status == BoatStatus::NORMAL_SAIL) {
                    alert.level = calculateAlertLevel(collision_time);
                    
                    if (isOncomingTraffic(boat, other_boat)) {
                        alert.oncoming_boat_ids.push_back(other_id);
                        alert.other_heading = other_boat.heading;
                    } else {
                        alert.front_boat_ids.push_back(other_id);
                    }
                }
            }
        }
        
        if (alert.level != AlertLevel::NORMAL) {
            alert.collision_time = min_collision_time;
            alert.collision_position = predicted_collision_pos;
            alert.decision_advice = generateDecisionAdvice(alert);
            alerts.push_back(alert);
        }
    }
    
    return alerts;
}

std::vector<CollisionAlert> CollisionDetector::detectDockingCollisions() {
    std::vector<CollisionAlert> alerts;
    
    // 查找所有入坞状态的船只
    for (const auto& [boat_id, boat] : boat_states_) {
        if (boat.status != BoatStatus::DOCKING) continue;
        
        CollisionAlert alert;
        alert.current_boat_id = boat_id;
        alert.current_heading = boat.heading;
        alert.level = AlertLevel::NORMAL;
        
        double min_collision_time = std::numeric_limits<double>::max();
        GeoPoint predicted_collision_pos;
        
        // 检查与跟随船只的碰撞风险
        for (const auto& [other_id, other_boat] : boat_states_) {
            if (other_id == boat_id) continue;
            
            // 入坞船只具有最高优先级，其他船只需要避让
            if (isOnSameRoute(boat, other_boat)) {
                // 计算碰撞时间
                GeoPoint boat_vel = geometry::calculateDestination(
                    GeoPoint(0, 0), boat.heading, boat.speed);
                GeoPoint other_vel = geometry::calculateDestination(
                    GeoPoint(0, 0), other_boat.heading, other_boat.speed);
                    
                double collision_time = geometry::calculateCollisionTime(
                    boat.getPosition(), boat_vel,
                    other_boat.getPosition(), other_vel,
                    getCollisionRadius()
                );
                
                if (collision_time > 0 && collision_time < min_collision_time) {
                    min_collision_time = collision_time;
                    alert.level = calculateAlertLevel(collision_time);
                    alert.front_boat_ids.push_back(other_id);
                    
                    // 计算碰撞位置
                    predicted_collision_pos = geometry::calculateDestination(
                        boat.getPosition(), boat.heading, boat.speed * collision_time);
                }
            }
        }
        
        if (alert.level != AlertLevel::NORMAL) {
            alert.collision_time = min_collision_time;
            alert.collision_position = predicted_collision_pos;
            alert.decision_advice = generateDecisionAdvice(alert);
            alerts.push_back(alert);
        }
    }
    
    return alerts;
}

std::vector<CollisionAlert> CollisionDetector::detectFollowingCollisions() {
    std::vector<CollisionAlert> alerts;
    
    // 查找所有正常航行状态的船只
    for (const auto& [boat_id, boat] : boat_states_) {
        if (boat.status != BoatStatus::NORMAL_SAIL) continue;
        
        CollisionAlert alert;
        alert.current_boat_id = boat_id;
        alert.current_heading = boat.heading;
        alert.level = AlertLevel::NORMAL;
        
        double min_collision_time = std::numeric_limits<double>::max();
        GeoPoint predicted_collision_pos;
        int closest_front_boat = -1;
        
        // 检查与同航线前方船只的碰撞风险
        for (const auto& [other_id, other_boat] : boat_states_) {
            if (other_id == boat_id) continue;
            
            if (isOnSameRoute(boat, other_boat) && !isOncomingTraffic(boat, other_boat)) {
                // 判断是否为前方船只
                double bearing_to_other = geometry::calculateBearing(
                    boat.getPosition(), other_boat.getPosition());
                double heading_diff = geometry::angleDifference(boat.heading, bearing_to_other);
                
                if (heading_diff < 45.0) { // 前方45度范围内
                    // 计算碰撞时间
                    GeoPoint boat_vel = geometry::calculateDestination(
                        GeoPoint(0, 0), boat.heading, boat.speed);
                    GeoPoint other_vel = geometry::calculateDestination(
                        GeoPoint(0, 0), other_boat.heading, other_boat.speed);
                        
                    double collision_time = geometry::calculateCollisionTime(
                        boat.getPosition(), boat_vel,
                        other_boat.getPosition(), other_vel,
                        getCollisionRadius()
                    );
                    
                    if (collision_time > 0 && collision_time < min_collision_time) {
                        min_collision_time = collision_time;
                        closest_front_boat = other_id;
                        
                        // 计算碰撞位置
                        predicted_collision_pos = geometry::calculateDestination(
                            boat.getPosition(), boat.heading, boat.speed * collision_time);
                        
                        alert.level = calculateAlertLevel(collision_time);
                    }
                }
            }
        }
        
        if (alert.level != AlertLevel::NORMAL && closest_front_boat != -1) {
            alert.front_boat_ids.push_back(closest_front_boat);
            alert.collision_time = min_collision_time;
            alert.collision_position = predicted_collision_pos;
            alert.decision_advice = generateDecisionAdvice(alert);
            alerts.push_back(alert);
        }
    }
    
    return alerts;
}

std::vector<CollisionAlert> CollisionDetector::detectOncomingCollisions() {
    std::vector<CollisionAlert> alerts;
    
    // 查找所有正常航行状态的船只
    for (const auto& [boat_id, boat] : boat_states_) {
        if (boat.status != BoatStatus::NORMAL_SAIL) continue;
        
        CollisionAlert alert;
        alert.current_boat_id = boat_id;
        alert.current_heading = boat.heading;
        alert.level = AlertLevel::NORMAL;
        
        double min_collision_time = std::numeric_limits<double>::max();
        GeoPoint predicted_collision_pos;
        
        // 检查与对向船只的碰撞风险
        for (const auto& [other_id, other_boat] : boat_states_) {
            if (other_id == boat_id) continue;
            
            if (isOncomingTraffic(boat, other_boat)) {
                // 计算碰撞时间
                GeoPoint boat_vel = geometry::calculateDestination(
                    GeoPoint(0, 0), boat.heading, boat.speed);
                GeoPoint other_vel = geometry::calculateDestination(
                    GeoPoint(0, 0), other_boat.heading, other_boat.speed);
                    
                double collision_time = geometry::calculateCollisionTime(
                    boat.getPosition(), boat_vel,
                    other_boat.getPosition(), other_vel,
                    getCollisionRadius()
                );
                
                if (collision_time > 0 && collision_time < min_collision_time) {
                    min_collision_time = collision_time;
                    alert.level = calculateAlertLevel(collision_time);
                    alert.oncoming_boat_ids.push_back(other_id);
                    alert.other_heading = other_boat.heading;
                    
                    // 计算碰撞位置
                    predicted_collision_pos = geometry::calculateDestination(
                        boat.getPosition(), boat.heading, boat.speed * collision_time);
                }
            }
        }
        
        if (alert.level != AlertLevel::NORMAL) {
            alert.collision_time = min_collision_time;
            alert.collision_position = predicted_collision_pos;
            alert.decision_advice = generateDecisionAdvice(alert);
            alerts.push_back(alert);
        }
    }
    
    return alerts;
}

AlertLevel CollisionDetector::calculateAlertLevel(double collision_time) const {
    if (collision_time <= config_.emergency_threshold_s) {
        return AlertLevel::EMERGENCY;
    } else if (collision_time <= config_.warning_threshold_s) {
        return AlertLevel::WARNING;
    }
    return AlertLevel::NORMAL;
}

std::string CollisionDetector::generateDecisionAdvice(const CollisionAlert& alert) const {
    std::stringstream advice;
    
    switch (alert.level) {
        case AlertLevel::EMERGENCY:
            advice << "紧急停船！";
            break;
        case AlertLevel::WARNING:
            if (!alert.oncoming_boat_ids.empty()) {
                advice << "对向来船，建议减速并向右避让；";
            }
            if (!alert.front_boat_ids.empty()) {
                advice << "前方有船，建议减速或停船等待；";
            }
            break;
        case AlertLevel::NORMAL:
            advice << "保持正常航行；";
            break;
    }
    
    return advice.str();
}

double CollisionDetector::getCollisionRadius() const {
    // 安全距离设为船只长度的2倍
    return config_.boat.length * 2.0;
}

bool CollisionDetector::isOnSameRoute(const BoatState& boat1, const BoatState& boat2) const {
    // 简化判断：同一航线方向的船只认为在同一航线上
    return boat1.route_direction == boat2.route_direction;
}

bool CollisionDetector::isOncomingTraffic(const BoatState& boat1, const BoatState& boat2) const {
    // 对向交通：不同航线方向且航向差接近180度
    if (boat1.route_direction == boat2.route_direction) {
        return false;
    }
    
    double heading_diff = geometry::angleDifference(boat1.heading, boat2.heading);
    return heading_diff > 135.0 && heading_diff < 225.0; // 允许45度误差
}

} // namespace boat_pro