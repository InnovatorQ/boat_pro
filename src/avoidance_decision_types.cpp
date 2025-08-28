#include "avoidance_decision_types.h"
#include <cmath>

namespace boat_pro {

// 中文描述映射表
const std::map<AvoidanceDecision, std::string> AvoidanceDecisionMapper::chinese_descriptions_ = {
    // 正常状态 (1-9)
    {AvoidanceDecision::CONTINUE_NORMAL, "保持当前航向和速度，继续正常航行"},
    {AvoidanceDecision::MONITOR_OTHER_VESSEL, "监控对方船只，保持警戒"},
    
    // 预防性避让 (10-19)
    {AvoidanceDecision::REDUCE_SPEED_SLIGHTLY, "轻微减速，保持安全距离"},
    {AvoidanceDecision::PREPARE_AVOIDANCE, "准备避让动作"},
    {AvoidanceDecision::INCREASE_DISTANCE, "增加与对方船只的距离"},
    
    // 主动避让 (20-39)
    {AvoidanceDecision::REDUCE_SPEED, "减速避让"},
    {AvoidanceDecision::TURN_RIGHT, "向右转向"},
    {AvoidanceDecision::TURN_LEFT, "向左转向"},
    {AvoidanceDecision::REDUCE_SPEED_AND_TURN_RIGHT, "减速并右转"},
    {AvoidanceDecision::REDUCE_SPEED_AND_TURN_LEFT, "减速并左转"},
    {AvoidanceDecision::STOP_AND_WAIT, "停船等待对方通过"},
    
    // 紧急避让 (40-59)
    {AvoidanceDecision::EMERGENCY_STOP, "紧急停船"},
    {AvoidanceDecision::EMERGENCY_TURN_RIGHT, "紧急右转"},
    {AvoidanceDecision::EMERGENCY_TURN_LEFT, "紧急左转"},
    {AvoidanceDecision::EMERGENCY_REVERSE, "紧急倒车"},
    {AvoidanceDecision::EMERGENCY_FULL_STOP, "立即全停"},
    
    // 优先级避让 (60-79)
    {AvoidanceDecision::YIELD_TO_PRIORITY_VESSEL, "给优先船只让行"},
    {AvoidanceDecision::MAINTAIN_PRIORITY_RIGHT, "保持优先通行权"},
    {AvoidanceDecision::UNDOCKING_MUST_YIELD, "出坞船只必须避让"},
    {AvoidanceDecision::DOCKING_HAS_PRIORITY, "入坞船只具有优先权"},
    {AvoidanceDecision::NORMAL_SAILING_PRIORITY, "正常航行船只优先权"},
    
    // 特定角度避让 (80-99)
    {AvoidanceDecision::HEAD_ON_TURN_RIGHT, "正面相遇向右转"},
    {AvoidanceDecision::OVERTAKING_KEEP_CLEAR, "追越时保持清晰航道"},
    {AvoidanceDecision::CROSSING_GIVE_WAY, "交叉相遇时让路"},
    {AvoidanceDecision::CROSSING_STAND_ON, "交叉相遇时保持航向"},
    
    // 未知情况
    {AvoidanceDecision::UNKNOWN_SITUATION, "未知情况，需要人工判断"}
};

// 英文描述映射表
const std::map<AvoidanceDecision, std::string> AvoidanceDecisionMapper::english_descriptions_ = {
    // 正常状态 (1-9)
    {AvoidanceDecision::CONTINUE_NORMAL, "Continue normal navigation"},
    {AvoidanceDecision::MONITOR_OTHER_VESSEL, "Monitor other vessel, maintain vigilance"},
    
    // 预防性避让 (10-19)
    {AvoidanceDecision::REDUCE_SPEED_SLIGHTLY, "Reduce speed slightly, maintain safe distance"},
    {AvoidanceDecision::PREPARE_AVOIDANCE, "Prepare for avoidance maneuver"},
    {AvoidanceDecision::INCREASE_DISTANCE, "Increase distance from other vessel"},
    
    // 主动避让 (20-39)
    {AvoidanceDecision::REDUCE_SPEED, "Reduce speed for avoidance"},
    {AvoidanceDecision::TURN_RIGHT, "Turn right"},
    {AvoidanceDecision::TURN_LEFT, "Turn left"},
    {AvoidanceDecision::REDUCE_SPEED_AND_TURN_RIGHT, "Reduce speed and turn right"},
    {AvoidanceDecision::REDUCE_SPEED_AND_TURN_LEFT, "Reduce speed and turn left"},
    {AvoidanceDecision::STOP_AND_WAIT, "Stop and wait for other vessel to pass"},
    
    // 紧急避让 (40-59)
    {AvoidanceDecision::EMERGENCY_STOP, "Emergency stop"},
    {AvoidanceDecision::EMERGENCY_TURN_RIGHT, "Emergency turn right"},
    {AvoidanceDecision::EMERGENCY_TURN_LEFT, "Emergency turn left"},
    {AvoidanceDecision::EMERGENCY_REVERSE, "Emergency reverse"},
    {AvoidanceDecision::EMERGENCY_FULL_STOP, "Immediate full stop"},
    
    // 优先级避让 (60-79)
    {AvoidanceDecision::YIELD_TO_PRIORITY_VESSEL, "Yield to priority vessel"},
    {AvoidanceDecision::MAINTAIN_PRIORITY_RIGHT, "Maintain priority right of way"},
    {AvoidanceDecision::UNDOCKING_MUST_YIELD, "Undocking vessel must yield"},
    {AvoidanceDecision::DOCKING_HAS_PRIORITY, "Docking vessel has priority"},
    {AvoidanceDecision::NORMAL_SAILING_PRIORITY, "Normal sailing vessel priority"},
    
    // 特定角度避让 (80-99)
    {AvoidanceDecision::HEAD_ON_TURN_RIGHT, "Head-on encounter turn right"},
    {AvoidanceDecision::OVERTAKING_KEEP_CLEAR, "Overtaking keep clear"},
    {AvoidanceDecision::CROSSING_GIVE_WAY, "Crossing give way"},
    {AvoidanceDecision::CROSSING_STAND_ON, "Crossing stand on"},
    
    // 未知情况
    {AvoidanceDecision::UNKNOWN_SITUATION, "Unknown situation, manual intervention required"}
};

// 详细指令映射表
const std::map<AvoidanceDecision, std::string> AvoidanceDecisionMapper::detailed_instructions_ = {
    {AvoidanceDecision::EMERGENCY_STOP, "立即切断推进器动力，启动紧急制动系统"},
    {AvoidanceDecision::EMERGENCY_TURN_RIGHT, "满舵右转，同时减速至最低航行速度"},
    {AvoidanceDecision::REDUCE_SPEED_AND_TURN_RIGHT, "将速度降低至当前速度的60%，舵角右转15-30度"},
    {AvoidanceDecision::YIELD_TO_PRIORITY_VESSEL, "减速至最低航行速度，向右侧避让，等待优先船只通过"},
    {AvoidanceDecision::CONTINUE_NORMAL, "保持当前航向和速度，继续按照预定航线航行"},
    {AvoidanceDecision::HEAD_ON_TURN_RIGHT, "正面相遇时双方均向右转向，避免碰撞"},
    {AvoidanceDecision::STOP_AND_WAIT, "停止前进，保持当前位置，等待对方船只安全通过"}
};

std::string AvoidanceDecisionMapper::getChineseDescription(AvoidanceDecision decision) {
    auto it = chinese_descriptions_.find(decision);
    if (it != chinese_descriptions_.end()) {
        return it->second;
    }
    return "未知决策类型";
}

std::string AvoidanceDecisionMapper::getEnglishDescription(AvoidanceDecision decision) {
    auto it = english_descriptions_.find(decision);
    if (it != english_descriptions_.end()) {
        return it->second;
    }
    return "Unknown decision type";
}

std::string AvoidanceDecisionMapper::getDetailedInstruction(AvoidanceDecision decision) {
    auto it = detailed_instructions_.find(decision);
    if (it != detailed_instructions_.end()) {
        return it->second;
    }
    return getChineseDescription(decision); // 如果没有详细指令，返回基本描述
}

AvoidanceDecision AvoidanceDecisionMapper::recommendDecision(
    int alert_level,
    double collision_angle,
    int current_boat_priority,
    int other_boat_priority,
    double collision_time
) {
    // 紧急情况 (碰撞时间 < 5秒)
    if (collision_time < 5.0 && alert_level == 3) {
        // 正面对撞 (150-180度)
        if (collision_angle >= 150.0) {
            return AvoidanceDecision::HEAD_ON_TURN_RIGHT;
        }
        // 交叉碰撞 (60-120度)
        else if (collision_angle >= 60.0 && collision_angle <= 120.0) {
            // 根据优先级决定
            if (current_boat_priority > other_boat_priority) {
                return AvoidanceDecision::EMERGENCY_STOP; // 低优先级船只停船
            } else {
                return AvoidanceDecision::MAINTAIN_PRIORITY_RIGHT; // 高优先级船只保持航行
            }
        }
        // 追尾情况 (0-30度)
        else if (collision_angle <= 30.0) {
            return AvoidanceDecision::EMERGENCY_TURN_RIGHT;
        }
        // 其他角度
        else {
            return AvoidanceDecision::EMERGENCY_TURN_RIGHT;
        }
    }
    
    // 警告情况 (碰撞时间 5-30秒)
    else if (collision_time >= 5.0 && collision_time <= 30.0 && alert_level == 2) {
        // 根据优先级和角度决定
        if (current_boat_priority > other_boat_priority) {
            // 当前船只优先级低，需要避让
            if (collision_angle >= 150.0) {
                return AvoidanceDecision::TURN_RIGHT;
            } else if (collision_angle >= 60.0 && collision_angle <= 120.0) {
                return AvoidanceDecision::CROSSING_GIVE_WAY;
            } else {
                return AvoidanceDecision::REDUCE_SPEED_AND_TURN_RIGHT;
            }
        } else {
            // 当前船只优先级高，保持航行
            if (collision_angle >= 60.0 && collision_angle <= 120.0) {
                return AvoidanceDecision::CROSSING_STAND_ON;
            } else {
                return AvoidanceDecision::MAINTAIN_PRIORITY_RIGHT;
            }
        }
    }
    
    // 正常情况
    else if (alert_level == 1) {
        return AvoidanceDecision::CONTINUE_NORMAL;
    }
    
    // 特殊优先级处理
    if (current_boat_priority == static_cast<int>(BoatPriority::DOCKING)) {
        return AvoidanceDecision::DOCKING_HAS_PRIORITY;
    } else if (current_boat_priority == static_cast<int>(BoatPriority::UNDOCKING)) {
        return AvoidanceDecision::UNDOCKING_MUST_YIELD;
    }
    
    // 默认情况
    return AvoidanceDecision::MONITOR_OTHER_VESSEL;
}

bool AvoidanceDecisionMapper::isValidDecision(int decision_code) {
    return chinese_descriptions_.find(static_cast<AvoidanceDecision>(decision_code)) != chinese_descriptions_.end();
}

AvoidanceDecision AvoidanceDecisionMapper::fromInt(int decision_code) {
    if (isValidDecision(decision_code)) {
        return static_cast<AvoidanceDecision>(decision_code);
    }
    return AvoidanceDecision::UNKNOWN_SITUATION;
}

int AvoidanceDecisionMapper::toInt(AvoidanceDecision decision) {
    return static_cast<int>(decision);
}

} // namespace boat_pro
