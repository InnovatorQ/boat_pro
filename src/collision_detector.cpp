#include "collision_detector.h"
#include "geometry_utils.h"
#include <algorithm>
#include <cmath>
#include <iostream>

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
    
    // 检测各种类型的碰撞
    auto undocking_alerts = detectUndockingCollisions();
    auto docking_alerts = detectDockingCollisions();
    auto route_alerts = detectRouteCollisions();
    auto oncoming_alerts = detectOncomingCollisions();
    
    // 合并所有告警
    alerts.insert(alerts.end(), undocking_alerts.begin(), undocking_alerts.end());
    alerts.insert(alerts.end(), docking_alerts.begin(), docking_alerts.end());
    alerts.insert(alerts.end(), route_alerts.begin(), route_alerts.end());
    alerts.insert(alerts.end(), oncoming_alerts.begin(), oncoming_alerts.end());
    
    return alerts;
}

std::vector<CollisionAlert> CollisionDetector::detectUndockingCollisions() {
    std::vector<CollisionAlert> alerts;
    
    for (const auto& [boat_id, boat] : boat_states_) {
        if (boat.status == BoatStatus::UNDOCKING) {
            for (const auto& [other_id, other_boat] : boat_states_) {
                if (boat_id != other_id) {
                    double collision_time = calculateCollisionTime(boat, other_boat);
                    if (collision_time > 0 && collision_time < config_.warning_threshold_s) {
                        CollisionAlert alert;
                        alert.current_boat_id = boat_id;
                        alert.level = determineAlertLevel(collision_time);
                        alert.collision_time = collision_time;
                        alert.collision_position = calculateCollisionPosition(boat, other_boat);
                        alert.avoidance_decision = generateDecisionAdvice(alert, boat);
                        alerts.push_back(alert);
                    }
                }
            }
        }
    }
    
    return alerts;
}

std::vector<CollisionAlert> CollisionDetector::detectDockingCollisions() {
    std::vector<CollisionAlert> alerts;
    
    for (const auto& [boat_id, boat] : boat_states_) {
        if (boat.status == BoatStatus::DOCKING) {
            for (const auto& [other_id, other_boat] : boat_states_) {
                if (boat_id != other_id) {
                    double collision_time = calculateCollisionTime(boat, other_boat);
                    if (collision_time > 0 && collision_time < config_.warning_threshold_s) {
                        CollisionAlert alert;
                        alert.current_boat_id = boat_id;
                        alert.level = determineAlertLevel(collision_time);
                        alert.collision_time = collision_time;
                        alert.collision_position = calculateCollisionPosition(boat, other_boat);
                        alert.avoidance_decision = generateDecisionAdvice(alert, boat);
                        alerts.push_back(alert);
                    }
                }
            }
        }
    }
    
    return alerts;
}

std::vector<CollisionAlert> CollisionDetector::detectRouteCollisions() {
    std::vector<CollisionAlert> alerts;
    
    for (const auto& [boat_id, boat] : boat_states_) {
        if (boat.status == BoatStatus::NORMAL_SAIL) {
            for (const auto& [other_id, other_boat] : boat_states_) {
                if (boat_id != other_id && other_boat.status == BoatStatus::NORMAL_SAIL) {
                    double collision_time = calculateCollisionTime(boat, other_boat);
                    if (collision_time > 0 && collision_time < config_.warning_threshold_s) {
                        CollisionAlert alert;
                        alert.current_boat_id = boat_id;
                        alert.level = determineAlertLevel(collision_time);
                        alert.collision_time = collision_time;
                        alert.collision_position = calculateCollisionPosition(boat, other_boat);
                        alert.avoidance_decision = generateDecisionAdvice(alert, boat);
                        alerts.push_back(alert);
                    }
                }
            }
        }
    }
    
    return alerts;
}

std::vector<CollisionAlert> CollisionDetector::detectOncomingCollisions() {
    std::vector<CollisionAlert> alerts;
    
    for (const auto& [boat_id, boat] : boat_states_) {
        for (const auto& [other_id, other_boat] : boat_states_) {
            if (boat_id != other_id) {
                double heading_diff = std::abs(boat.heading - other_boat.heading);
                if (heading_diff > 90 && heading_diff < 270) {
                    double collision_time = calculateCollisionTime(boat, other_boat);
                    if (collision_time > 0 && collision_time < config_.warning_threshold_s) {
                        CollisionAlert alert;
                        alert.current_boat_id = boat_id;
                        alert.level = determineAlertLevel(collision_time);
                        alert.collision_time = collision_time;
                        alert.collision_position = calculateCollisionPosition(boat, other_boat);
                        alert.oncoming_boat_ids.push_back(other_id);
                        alert.current_heading = boat.heading;
                        alert.other_heading = other_boat.heading;
                        alert.avoidance_decision = generateDecisionAdvice(alert, boat);
                        alerts.push_back(alert);
                    }
                }
            }
        }
    }
    
    return alerts;
}

double CollisionDetector::calculateCollisionTime(const BoatState& boat1, const BoatState& boat2) const {
    double distance = geometry::calculateDistance(
        GeoPoint(boat1.lat, boat1.lng), 
        GeoPoint(boat2.lat, boat2.lng)
    );
    
    double relative_speed = boat1.speed + boat2.speed;
    
    if (relative_speed > 0) {
        return distance / relative_speed;
    }
    
    return -1;
}

GeoPoint CollisionDetector::calculateCollisionPosition(const BoatState& boat1, const BoatState& boat2) const {
    return GeoPoint(
        (boat1.lat + boat2.lat) / 2.0,
        (boat1.lng + boat2.lng) / 2.0
    );
}

AlertLevel CollisionDetector::determineAlertLevel(double collision_time) const {
    if (collision_time <= config_.emergency_threshold_s) {
        return AlertLevel::EMERGENCY;
    } else if (collision_time <= config_.warning_threshold_s) {
        return AlertLevel::WARNING;
    } else {
        return AlertLevel::NORMAL;
    }
}

AvoidanceDecision CollisionDetector::generateDecisionAdvice(const CollisionAlert& alert, const BoatState& current_boat) const {
    if (alert.level == AlertLevel::EMERGENCY) {
        return AvoidanceDecision::EMERGENCY_STOP;
    } else if (alert.level == AlertLevel::WARNING) {
        return AvoidanceDecision::REDUCE_SPEED_AND_TURN_RIGHT;
    } else {
        return AvoidanceDecision::CONTINUE_NORMAL;
    }
}

}
