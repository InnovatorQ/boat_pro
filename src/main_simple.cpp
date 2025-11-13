#include "avoidance_decision_types.h"
#include <iostream>
#include <vector>

using namespace boat_pro;

int main() {
    std::cout << "=== 无人船碰撞预测系统 ===" << std::endl;
    std::cout << "系统启动成功！" << std::endl;
    
    // 测试避碰决策枚举
    std::cout << "\n避碰决策测试:" << std::endl;
    AvoidanceDecision decision = AvoidanceDecision::REDUCE_SPEED_AND_TURN_RIGHT;
    std::cout << "决策代码: " << static_cast<int>(decision) << std::endl;
    std::cout << "决策描述: " << AvoidanceDecisionMapper::getChineseDescription(decision) << std::endl;
    
    // 测试不同的决策类型
    std::vector<AvoidanceDecision> decisions = {
        AvoidanceDecision::CONTINUE_NORMAL,
        AvoidanceDecision::REDUCE_SPEED,
        AvoidanceDecision::TURN_RIGHT,
        AvoidanceDecision::EMERGENCY_STOP,
        AvoidanceDecision::HEAD_ON_TURN_RIGHT
    };
    
    std::cout << "\n所有决策类型测试:" << std::endl;
    for (const auto& dec : decisions) {
        std::cout << "代码 " << static_cast<int>(dec) << ": " 
                  << AvoidanceDecisionMapper::getChineseDescription(dec) << std::endl;
    }
    
    std::cout << "\n碰撞类型测试:" << std::endl;
    std::vector<CollisionType> types = {
        CollisionType::OVERTAKING,
        CollisionType::OBLIQUE,
        CollisionType::CROSSING,
        CollisionType::HEAD_ON
    };
    
    for (const auto& type : types) {
        std::cout << "类型 " << static_cast<int>(type) << ": 碰撞类型" << std::endl;
    }
    
    std::cout << "\n系统运行完成！" << std::endl;
    return 0;
}
