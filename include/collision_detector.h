// ==================== include/collision_detector.h ====================
#ifndef BOAT_PRO_COLLISION_DETECTOR_H
#define BOAT_PRO_COLLISION_DETECTOR_H

#include "types.h"
#include <vector>
#include <map>

namespace boat_pro {

/**
 * 碰撞检测器
 * 负责检测各种类型的碰撞风险并生成告警
 */
class CollisionDetector {
public:
    CollisionDetector(const SystemConfig& config);
    
    /**
     * 更新船只状态数据
     */
    void updateBoatStates(const std::vector<BoatState>& boats);
    
    /**
     * 设置船坞信息
     */
    void setDockInfo(const std::vector<DockInfo>& docks);
    
    /**
     * 设置航线信息
     */
    void setRouteInfo(const std::vector<RouteInfo>& routes);
    
    /**
     * 检测所有碰撞风险
     * @return 碰撞告警列表
     */
    std::vector<CollisionAlert> detectCollisions();
    
private:
    SystemConfig config_;
    std::map<int, BoatState> boat_states_;
    std::vector<DockInfo> dock_info_;
    std::vector<RouteInfo> route_info_;
    
    /**
     * 检测出坞碰撞
     */
    std::vector<CollisionAlert> detectUndockingCollisions();
    
    /**
     * 检测入坞碰撞
     */
    std::vector<CollisionAlert> detectDockingCollisions();
    
    /**
     * 检测航线碰撞
     */
    std::vector<CollisionAlert> detectRouteCollisions();
    
    /**
     * 检测对向碰撞
     */
    std::vector<CollisionAlert> detectOncomingCollisions();
    
    /**
     * 生成决策建议
     */
    AvoidanceDecision generateDecisionAdvice(const CollisionAlert& alert, const BoatState& current_boat) const;
    
    /**
     * 计算碰撞时间
     */
    double calculateCollisionTime(const BoatState& boat1, const BoatState& boat2) const;
    
    /**
     * 计算碰撞位置
     */
    GeoPoint calculateCollisionPosition(const BoatState& boat1, const BoatState& boat2) const;
    
    /**
     * 判断告警等级
     */
    AlertLevel determineAlertLevel(double collision_time) const;
};

} // namespace boat_pro

#endif
