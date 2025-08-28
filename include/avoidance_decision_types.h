#ifndef AVOIDANCE_DECISION_TYPES_H
#define AVOIDANCE_DECISION_TYPES_H

#include <string>
#include <map>

namespace boat_pro {

/**
 * 避碰决策枚举类型
 * 专注于两船避让的核心场景
 */
enum class AvoidanceDecision : int {
    // ========== 正常状态 (1-9) ==========
    CONTINUE_NORMAL = 1,              // 保持当前航向和速度，继续正常航行
    MONITOR_OTHER_VESSEL = 2,         // 监控对方船只，保持警戒
    
    // ========== 预防性避让 (10-19) ==========
    REDUCE_SPEED_SLIGHTLY = 10,       // 轻微减速，保持安全距离
    PREPARE_AVOIDANCE = 11,           // 准备避让动作
    INCREASE_DISTANCE = 12,           // 增加与对方船只的距离
    
    // ========== 主动避让 (20-39) ==========
    REDUCE_SPEED = 20,                // 减速避让
    TURN_RIGHT = 21,                  // 向右转向
    TURN_LEFT = 22,                   // 向左转向
    REDUCE_SPEED_AND_TURN_RIGHT = 23, // 减速并右转
    REDUCE_SPEED_AND_TURN_LEFT = 24,  // 减速并左转
    STOP_AND_WAIT = 25,               // 停船等待对方通过
    
    // ========== 紧急避让 (40-59) ==========
    EMERGENCY_STOP = 40,              // 紧急停船
    EMERGENCY_TURN_RIGHT = 41,        // 紧急右转
    EMERGENCY_TURN_LEFT = 42,         // 紧急左转
    EMERGENCY_REVERSE = 43,           // 紧急倒车
    EMERGENCY_FULL_STOP = 44,         // 立即全停
    
    // ========== 优先级避让 (60-79) ==========
    YIELD_TO_PRIORITY_VESSEL = 60,    // 给优先船只让行
    MAINTAIN_PRIORITY_RIGHT = 61,     // 保持优先通行权
    UNDOCKING_MUST_YIELD = 62,        // 出坞船只必须避让
    DOCKING_HAS_PRIORITY = 63,        // 入坞船只具有优先权
    NORMAL_SAILING_PRIORITY = 64,     // 正常航行船只优先权
    
    // ========== 特定角度避让 (80-99) ==========
    HEAD_ON_TURN_RIGHT = 80,          // 正面相遇向右转
    OVERTAKING_KEEP_CLEAR = 81,       // 追越时保持清晰航道
    CROSSING_GIVE_WAY = 82,           // 交叉相遇时让路
    CROSSING_STAND_ON = 83,           // 交叉相遇时保持航向
    
    // ========== 未知情况 (99) ==========
    UNKNOWN_SITUATION = 99            // 未知情况，需要人工判断
};

/**
 * 避碰决策描述映射类
 */
class AvoidanceDecisionMapper {
public:
    /**
     * 获取中文描述
     */
    static std::string getChineseDescription(AvoidanceDecision decision);
    
    /**
     * 获取英文描述
     */
    static std::string getEnglishDescription(AvoidanceDecision decision);
    
    /**
     * 获取详细操作指令
     */
    static std::string getDetailedInstruction(AvoidanceDecision decision);
    
    /**
     * 根据场景参数推荐决策
     * @param alert_level 告警等级 (1-3)
     * @param collision_angle 碰撞角度 (0-180度)
     * @param current_boat_priority 当前船只优先级 (1=入坞最高, 2=正常航行, 3=出坞最低)
     * @param other_boat_priority 对方船只优先级
     * @param collision_time 预计碰撞时间(秒)
     * @return 推荐的避碰决策
     */
    static AvoidanceDecision recommendDecision(
        int alert_level,
        double collision_angle,
        int current_boat_priority,
        int other_boat_priority,
        double collision_time
    );
    
    /**
     * 验证决策是否有效
     */
    static bool isValidDecision(int decision_code);
    
    /**
     * 从整数转换为枚举
     */
    static AvoidanceDecision fromInt(int decision_code);
    
    /**
     * 从枚举转换为整数
     */
    static int toInt(AvoidanceDecision decision);

private:
    // 中文描述映射表
    static const std::map<AvoidanceDecision, std::string> chinese_descriptions_;
    
    // 英文描述映射表
    static const std::map<AvoidanceDecision, std::string> english_descriptions_;
    
    // 详细指令映射表
    static const std::map<AvoidanceDecision, std::string> detailed_instructions_;
};

/**
 * 船只优先级枚举
 */
enum class BoatPriority : int {
    DOCKING = 1,        // 入坞船只 - 最高优先级
    NORMAL_SAILING = 2, // 正常航行 - 中等优先级
    UNDOCKING = 3       // 出坞船只 - 最低优先级
};

/**
 * 碰撞类型枚举
 */
enum class CollisionType : int {
    NO_COLLISION = 0,   // 无碰撞风险
    HEAD_ON = 1,        // 正面对撞 (150-180度)
    OVERTAKING = 2,     // 追尾 (0-30度)
    CROSSING = 3,       // 交叉相遇 (60-120度)
    OBLIQUE = 4         // 斜向碰撞 (30-60度, 120-150度)
};

} // namespace boat_pro

#endif // AVOIDANCE_DECISION_TYPES_H
