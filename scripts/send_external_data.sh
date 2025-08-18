#!/bin/bash

# 外部系统数据发送示例脚本
# 演示外部系统如何向无人船系统发送数据

echo "=========================================="
echo "  外部系统数据发送示例"
echo "=========================================="

# 检查参数
if [ $# -eq 0 ]; then
    echo "用法: $0 [boat_state|collision_alert|system_config|fleet_command|all]"
    echo ""
    echo "示例:"
    echo "  $0 boat_state     # 发送船只状态数据"
    echo "  $0 collision_alert # 发送碰撞告警"
    echo "  $0 system_config  # 发送系统配置"
    echo "  $0 fleet_command  # 发送舰队命令"
    echo "  $0 all           # 发送所有类型数据"
    exit 1
fi

MQTT_HOST="localhost"
MQTT_PORT="1883"

# 检查mosquitto客户端工具
if ! command -v mosquitto_pub &> /dev/null; then
    echo "❌ mosquitto_pub命令未找到，请安装mosquitto-clients"
    echo "   sudo apt-get install mosquitto-clients"
    exit 1
fi

echo "✅ MQTT客户端工具已就绪"
echo "📡 MQTT代理: $MQTT_HOST:$MQTT_PORT"
echo ""

# 发送船只状态数据
send_boat_state() {
    echo "🚢 发送船只状态数据..."
    
    # 船只1状态
    BOAT1_DATA='{
  "sysid": 1,
  "timestamp": '$(date +%s.%3N)',
  "lat": 30.549832,
  "lng": 114.342922,
  "heading": 90.0,
  "speed": 2.5,
  "status": 2,
  "route_direction": 1
}'
    
    echo "  发送船只1状态..."
    echo "$BOAT1_DATA" | mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/boat_state/1" -s
    
    # 船只2状态
    BOAT2_DATA='{
  "sysid": 2,
  "timestamp": '$(date +%s.%3N)',
  "lat": 30.549900,
  "lng": 114.343000,
  "heading": 270.0,
  "speed": 1.8,
  "status": 2,
  "route_direction": 2
}'
    
    echo "  发送船只2状态..."
    echo "$BOAT2_DATA" | mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/boat_state/2" -s
    
    # 船只3状态（入坞状态）
    BOAT3_DATA='{
  "sysid": 3,
  "timestamp": '$(date +%s.%3N)',
  "lat": 30.549100,
  "lng": 114.343000,
  "heading": 180.0,
  "speed": 0.5,
  "status": 3,
  "route_direction": 1
}'
    
    echo "  发送船只3状态（入坞）..."
    echo "$BOAT3_DATA" | mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/boat_state/3" -s
    
    echo "✅ 船只状态数据发送完成"
}

# 发送碰撞告警
send_collision_alert() {
    echo "🚨 发送碰撞告警..."
    
    ALERT_DATA='{
  "current_boat_id": 1,
  "level": 2,
  "collision_time": 15.5,
  "collision_position": {
    "lat": 30.549832,
    "lng": 114.342922
  },
  "front_boat_ids": [2],
  "oncoming_boat_ids": [],
  "current_heading": 90.0,
  "other_heading": 270.0,
  "decision_advice": "减速避让"
}'
    
    echo "$ALERT_DATA" | mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/collision_alert/1" -s
    
    # 发送紧急告警
    EMERGENCY_ALERT='{
  "current_boat_id": 2,
  "level": 3,
  "collision_time": 3.2,
  "collision_position": {
    "lat": 30.549900,
    "lng": 114.343000
  },
  "front_boat_ids": [],
  "oncoming_boat_ids": [1],
  "current_heading": 270.0,
  "other_heading": 90.0,
  "decision_advice": "立即停船"
}'
    
    echo "  发送紧急告警..."
    echo "$EMERGENCY_ALERT" | mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/collision_alert/2" -s
    
    echo "✅ 碰撞告警发送完成"
}

# 发送系统配置
send_system_config() {
    echo "⚙️ 发送系统配置更新..."
    
    CONFIG_DATA='{
  "boat": {
    "length": 0.75,
    "width": 0.47
  },
  "emergency_threshold_s": 5,
  "warning_threshold_s": 30,
  "max_boats": 30,
  "min_route_gap_m": 10
}'
    
    echo "$CONFIG_DATA" | mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/system_config" -s -r
    
    echo "✅ 系统配置发送完成（已保留）"
}

# 发送命令
send_fleet_command() {
    echo "📋 发送命令..."
    
    # 减速命令
    SLOW_DOWN_CMD='{
  "boat_id": 1,
  "command": "slow_down",
  "target_speed": 1.0,
  "timestamp": '$(date +%s)',
  "reason": "collision_warning"
}'
    
    echo "  发送减速命令给船只1..."
    echo "$SLOW_DOWN_CMD" | mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/fleet_command/1" -s
    
    # 紧急停船命令
    EMERGENCY_STOP_CMD='{
  "boat_id": 2,
  "command": "emergency_stop",
  "timestamp": '$(date +%s)',
  "reason": "collision_alert"
}'
    
    echo "  发送紧急停船命令给船只2..."
    echo "$EMERGENCY_STOP_CMD" | mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/fleet_command/2" -s
    
    # 航向调整命令
    HEADING_CMD='{
  "boat_id": 3,
  "command": "change_heading",
  "target_heading": 200.0,
  "timestamp": '$(date +%s)',
  "reason": "route_optimization"
}'
    
    echo "  发送航向调整命令给船只3..."
    echo "$HEADING_CMD" | mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/fleet_command/3" -s
    
    echo "✅ 舰队命令发送完成"
}

# 发送其他数据
send_other_data() {
    echo "📡 发送其他数据..."
    
    # 心跳消息
    echo "  发送心跳消息..."
    for i in {1..3}; do
        HEARTBEAT='{
  "boat_id": '$i',
  "timestamp": '$(date +%s)',
  "status": "alive"
}'
        echo "$HEARTBEAT" | mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/heartbeat/$i" -s
    done
    
    # 船坞信息
    echo "  发送船坞信息..."
    DOCK_INFO='{
  "dock_id": 1,
  "lat": 30.549100,
  "lng": 114.343000,
  "status": "available",
  "capacity": 2
}'
    echo "$DOCK_INFO" | mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/dock_info" -s -r
    
    # 航线信息
    echo "  发送航线信息..."
    ROUTE_INFO='{
  "route_id": 1,
  "direction": 1,
  "points": [
    {"lat": 30.549500, "lng": 114.342800},
    {"lat": 30.549800, "lng": 114.343300},
    {"lat": 30.550100, "lng": 114.343800}
  ],
  "status": "active"
}'
    echo "$ROUTE_INFO" | mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "boat_pro/route_info" -s -r
    
    echo "✅ 其他数据发送完成"
}

# 根据参数执行相应操作
case "$1" in
    "boat_state")
        send_boat_state
        ;;
    "collision_alert")
        send_collision_alert
        ;;
    "system_config")
        send_system_config
        ;;
    "fleet_command")
        send_fleet_command
        ;;
    "all")
        echo "📤 发送所有类型的测试数据..."
        echo ""
        send_boat_state
        echo ""
        sleep 1
        send_collision_alert
        echo ""
        sleep 1
        send_system_config
        echo ""
        sleep 1
        send_fleet_command
        echo ""
        sleep 1
        send_other_data
        echo ""
        echo "🎉 所有数据发送完成！"
        ;;
    *)
        echo "❌ 未知参数: $1"
        echo "支持的参数: boat_state, collision_alert, system_config, fleet_command, all"
        exit 1
        ;;
esac

echo ""
echo "💡 提示: 可以使用以下命令监控接收到的消息:"
echo "   mosquitto_sub -h $MQTT_HOST -p $MQTT_PORT -t 'boat_pro/#' -v"
echo ""
