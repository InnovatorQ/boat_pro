// ==================== src/fleet_manager.cpp ====================
#include "fleet_manager.h"
#include <iostream>
#include <algorithm>

namespace boat_pro {

FleetManager::FleetManager(const SystemConfig& config) 
    : config_(config), collision_detector_(std::make_unique<CollisionDetector>(config)) {
}

void FleetManager::setAlertCallback(AlertCallback callback) {
    alert_callback_ = callback;
}

void FleetManager::initializeDocks(const std::vector<DockInfo>& docks) {
    dock_info_ = docks;
    collision_detector_->setDockInfo(docks);
}

void FleetManager::initializeRoutes(const std::vector<RouteInfo>& routes) {
    route_info_ = routes;
    collision_detector_->setRouteInfo(routes);
}

void FleetManager::updateBoatState(const BoatState& boat) {
    // 更新或添加船只状态
    auto it = std::find_if(current_boat_states_.begin(), current_boat_states_.end(),
                          [&boat](const BoatState& b) { return b.sysid == boat.sysid; });
    
    if (it != current_boat_states_.end()) {
        *it = boat;
    } else {
        current_boat_states_.push_back(boat);
    }
    
    // 更新碰撞检测器
    collision_detector_->updateBoatStates(current_boat_states_);
}

void FleetManager::updateBoatStates(const std::vector<BoatState>& boats) {
    current_boat_states_ = boats;
    collision_detector_->updateBoatStates(boats);
}

bool FleetManager::requestUndocking(int boat_id, int dock_id) {
    // 简化实现：总是允许出坞
    std::cout << "船只 " << boat_id << " 请求从船坞 " << dock_id << " 出坞" << std::endl;
    return true;
}

bool FleetManager::requestDocking(int boat_id, int dock_id) {
    // 简化实现：总是允许入坞
    std::cout << "船只 " << boat_id << " 请求进入船坞 " << dock_id << std::endl;
    return true;
}

void FleetManager::performCollisionDetection() {
    auto alerts = collision_detector_->detectCollisions();
    
    for (const auto& alert : alerts) {
        handleCollisionAlert(alert);
    }
}

void FleetManager::handleCollisionAlert(const CollisionAlert& alert) {
    if (alert_callback_) {
        alert_callback_(alert);
    }
}

} // namespace boat_pro
