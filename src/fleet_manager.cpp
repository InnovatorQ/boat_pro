// ==================== src/fleet_manager.cpp ====================
#include "fleet_manager.h"
#include "geometry_utils.h"
#include <thread>
#include <chrono>
#include <iostream>

namespace boat_pro {

FleetManager::FleetManager(const SystemConfig& config) 
    : config_(config), monitoring_active_(false) {
    collision_detector_ = std::make_unique<CollisionDetector>(config);
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

// 【新增】初始化通信系统
bool FleetManager::initializeCommunication(const communication::UDPConfig& config) {
    communicator_ = std::make_unique<communication::UDPCommunicator>(config);
    
    if (!communicator_->initialize()) {
        std::cerr << "通信系统初始化失败" << std::endl;
        return false;
    }
    
    // 设置消息回调
    communicator_->setDroneIDCallback(
        [this](std::unique_ptr<communication::DroneIDMessage> msg) {
            onDroneIDMessageReceived(std::move(msg));
        });
    
    communicator_->setNMEA2000Callback(
        [this](std::unique_ptr<communication::NMEA2000Message> msg) {
            onNMEA2000MessageReceived(std::move(msg));
        });
    
    communicator_->setBoatStateCallback(
        [this](const BoatState& boat) {
            onBoatStateReceived(boat);
        });
    
    std::cout << "通信系统初始化成功" << std::endl;
    return true;
}

// 【新增】启动通信系统
bool FleetManager::startCommunication() {
    if (!communicator_) {
        std::cerr << "通信系统未初始化" << std::endl;
        return false;
    }
    
    if (!communicator_->startReceiving()) {
        std::cerr << "启动通信接收失败" << std::endl;
        return false;
    }
    
    std::cout << "通信系统已启动" << std::endl;
    return true;
}

// 【新增】停止通信系统
void FleetManager::stopCommunication() {
    if (communicator_) {
        communicator_->stopReceiving();
        std::cout << "通信系统已停止" << std::endl;
    }
}

void FleetManager::updateBoatState(const BoatState& boat) {
    std::vector<BoatState> boats = {boat};
    collision_detector_->updateBoatStates(boats);
}

void FleetManager::updateBoatStates(const std::vector<BoatState>& boats) {
    collision_detector_->updateBoatStates(boats);
}

// 【新增】通过网络广播船只状态
bool FleetManager::broadcastBoatState(const BoatState& boat, bool use_drone_id, bool use_nmea2000) {
    if (!communicator_) {
        std::cerr << "通信系统未初始化，无法广播" << std::endl;
        return false;
    }
    
    bool success = communicator_->sendBoatState(boat, use_drone_id, use_nmea2000);
    
    if (success) {
        std::cout << "船只 " << boat.sysid << " 状态广播成功" << std::endl;
    } else {
        std::cerr << "船只 " << boat.sysid << " 状态广播失败" << std::endl;
    }
    
    return success;
}

bool FleetManager::requestUndocking(int boat_id, int dock_id) {
    // 检查船只是否可以安全出坞
    if (!canUndock(boat_id, dock_id)) {
        std::cout << "船只 " << boat_id << " 暂时无法从船坞 " << dock_id << " 出坞，存在碰撞风险。" << std::endl;
        return false;
    }
    
    std::cout << "船只 " << boat_id << " 获准从船坞 " << dock_id << " 出坞。" << std::endl;
    return true;
}

bool FleetManager::requestDocking(int boat_id) {
    // 获取推荐船坞
    int recommended_dock = getRecommendedDock(boat_id);
    if (recommended_dock == -1) {
        std::cout << "船只 " << boat_id << " 暂时无可用船坞。" << std::endl;
        return false;
    }
    
    if (!canDock(boat_id, recommended_dock)) {
        std::cout << "船只 " << boat_id << " 暂时无法入坞船坞 " << recommended_dock << "，存在碰撞风险。" << std::endl;
        return false;
    }
    
    std::cout << "船只 " << boat_id << " 获准进入船坞 " << recommended_dock << "。" << std::endl;
    return true;
}

int FleetManager::getRecommendedDock(int boat_id) {
    // 这里应该获取当前船只状态，但为了简化，返回第一个可用船坞
    return dock_info_.empty() ? -1 : dock_info_[0].dock_id;
}

void FleetManager::runSafetyMonitoring() {
    monitoring_active_ = true;
    
    std::thread monitoring_thread([this]() {
        while (monitoring_active_) {
            // 检测碰撞风险
            auto alerts = collision_detector_->detectCollisions();
            
            // 处理告警
            for (const auto& alert : alerts) {
                if (alert_callback_) {
                    alert_callback_(alert);
                } else {
                    // 默认输出告警信息
                    std::cout << "碰撞告警 - 船只ID: " << alert.current_boat_id 
                              << ", 等级: " << static_cast<int>(alert.level)
                              << ", 建议: " << alert.decision_advice << std::endl;
                }
            }
            
            // 休眠100毫秒后继续检测
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    monitoring_thread.detach();
}

void FleetManager::stopSafetyMonitoring() {
    monitoring_active_ = false;
}

std::vector<CollisionAlert> FleetManager::getCurrentAlerts() {
    return collision_detector_->detectCollisions();
}

// 【新增】获取通信统计信息
communication::UDPCommunicator::Statistics FleetManager::getCommunicationStatistics() const {
    if (!communicator_) {
        return communication::UDPCommunicator::Statistics{};
    }
    return communicator_->getStatistics();
}

bool FleetManager::canUndock(int boat_id, int dock_id) {
    // 简化实现：检查是否有高优先级船只在附近
    auto alerts = collision_detector_->detectCollisions();
    
    for (const auto& alert : alerts) {
        if (alert.current_boat_id == boat_id && alert.level != AlertLevel::NORMAL) {
            return false;
        }
    }
    
    return true;
}

bool FleetManager::canDock(int boat_id, int dock_id) {
    // 简化实现：入坞船只具有最高优先级，通常允许入坞
    return true;
}

int FleetManager::findNearestAvailableDock(const BoatState& boat) {
    if (dock_info_.empty()) return -1;
    
    int nearest_dock = dock_info_[0].dock_id;
    double min_distance = std::numeric_limits<double>::max();
    
    for (const auto& dock : dock_info_) {
        double distance = geometry::calculateDistance(
            boat.getPosition(), dock.getPosition());
        
        if (distance < min_distance) {
            min_distance = distance;
            nearest_dock = dock.dock_id;
        }
    }
    
    return nearest_dock;
}

// 【新增】处理接收到的Drone ID消息
void FleetManager::onDroneIDMessageReceived(std::unique_ptr<communication::DroneIDMessage> message) {
    std::cout << "收到Drone ID消息，类型: " << static_cast<int>(message->getMessageType()) << std::endl;
    
    // 如果是位置消息，转换为船只状态并更新
    if (message->getMessageType() == communication::DroneIDMessageType::LOCATION) {
        auto* loc_msg = dynamic_cast<communication::DroneIDLocationMessage*>(message.get());
        if (loc_msg) {
            // 这里需要从消息中提取船只ID，简化处理使用固定ID
            BoatState boat = communication::ProtocolConverter::fromDroneIDLocation(*loc_msg, 999);
            updateBoatState(boat);
        }
    }
}

// 【新增】处理接收到的NMEA 2000消息
void FleetManager::onNMEA2000MessageReceived(std::unique_ptr<communication::NMEA2000Message> message) {
    std::cout << "收到NMEA 2000消息，PGN: " << static_cast<uint32_t>(message->getPGN()) << std::endl;
    
    // 这里可以实现NMEA 2000消息的缓存和组合逻辑
    // 简化处理，仅记录接收
}

// 【新增】处理接收到的船只状态
void FleetManager::onBoatStateReceived(const BoatState& boat) {
    std::cout << "收到外部船只状态 - ID: " << boat.sysid 
              << ", 位置: (" << boat.lat << ", " << boat.lng << ")" << std::endl;
    
    // 更新船只状态到系统中
    updateBoatState(boat);
}

} // namespace boat_pro