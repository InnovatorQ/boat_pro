// ==================== include/fleet_manager.h ====================
#ifndef BOAT_PRO_FLEET_MANAGER_H
#define BOAT_PRO_FLEET_MANAGER_H

#include "types.h"
#include "collision_detector.h"
#include <memory>
#include <functional>

namespace boat_pro {

/**
 * 无人船集群管理系统
 * 负责船队的整体管理、调度和安全监控
 */
class FleetManager {
public:
    using AlertCallback = std::function<void(const CollisionAlert&)>;
    
    FleetManager(const SystemConfig& config = SystemConfig::getDefault());
    
    /**
     * 设置碰撞告警回调函数
     */
    void setAlertCallback(AlertCallback callback);
    
    /**
     * 初始化船坞信息
     */
    void initializeDocks(const std::vector<DockInfo>& docks);
    
    /**
     * 初始化航线信息
     */
    void initializeRoutes(const std::vector<RouteInfo>& routes);
    
    /**
     * 更新船只状态
     */
    void updateBoatState(const BoatState& boat);
    
    /**
     * 批量更新船只状态
     */
    void updateBoatStates(const std::vector<BoatState>& boats);
    
    /**
     * 处理出坞请求
     */
    bool requestUndocking(int boat_id, int dock_id);
    
    /**
     * 处理入坞请求
     */
    bool requestDocking(int boat_id, int dock_id);
    
    /**
     * 执行碰撞检测
     */
    void performCollisionDetection();
    
private:
    SystemConfig config_;
    std::unique_ptr<CollisionDetector> collision_detector_;
    AlertCallback alert_callback_;
    std::vector<BoatState> current_boat_states_;
    std::vector<DockInfo> dock_info_;
    std::vector<RouteInfo> route_info_;
    
    /**
     * 处理碰撞告警
     */
    void handleCollisionAlert(const CollisionAlert& alert);
};

} // namespace boat_pro

#endif
