#!/bin/bash

# MPC客户端MQTT通信测试脚本
echo "=== MPC客户端MQTT通信测试 ==="
echo "Model Predictive Control Client Testing"
echo "客户端配置:"
echo "  IP地址: 127.0.0.1"
echo "  端口: 2000"
echo "  账号: vEagles"
echo "  密码: 123456"
echo "  客户端ID: 1000"
echo ""

# 检查mosquitto客户端工具
if ! command -v mosquitto_pub &> /dev/null; then
    echo "错误: mosquitto客户端工具未安装"
    echo "请运行: sudo apt-get install mosquitto-clients"
    exit 1
fi

# MQTT代理配置
MQTT_HOST="127.0.0.1"
MQTT_PORT="2000"
MQTT_USER="vEagles"
MQTT_PASS="123456"
CLIENT_ID="1000"

# MPC订阅主题
MPC_SUBSCRIBE_TOPICS=(
    "mpc/BoatState"
    "mpc/DockInfo"
    "mpc/RouteInfo"
    "mpc/Config"
)

# MPC发布主题
MPC_PUBLISH_TOPICS=(
    "gcs/CollisionAlert"
    "gcs/SafetyStatus"
    "gcs/FleetCommand"
    "gcs/SystemStatus"
    "gcs/Heartbeat"
)

echo "=== 测试MPC订阅主题 ==="
for topic in "${MPC_SUBSCRIBE_TOPICS[@]}"; do
    echo "测试订阅主题: $topic"
    timeout 2s mosquitto_sub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "${CLIENT_ID}_sub" -t "$topic" -C 1 &
    sleep 0.5
done

echo ""
echo "=== 测试MPC发布主题 ==="

# 1. 发布船只状态数据到mpc/BoatState (模拟MPC客户端发布)
echo "1. 发布船只状态数据到 mpc/BoatState"
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "${CLIENT_ID}_pub" \
    -t "mpc/BoatState" \
    -m '{
        "sysid": 1,
        "lat": 30.549832,
        "lng": 114.342922,
        "speed": 2.5,
        "heading": 45.0,
        "timestamp": 1692168000,
        "status": "normal",
        "mpc_control": {
            "target_speed": 3.0,
            "target_heading": 50.0,
            "control_mode": "auto"
        }
    }'

# 2. 发布船坞信息到mpc/DockInfo
echo "2. 发布船坞信息到 mpc/DockInfo"
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "${CLIENT_ID}_pub" \
    -t "mpc/DockInfo" \
    -m '{
        "dock_id": 1,
        "position": {
            "lat": 30.550000,
            "lng": 114.343000
        },
        "capacity": 10,
        "status": "available",
        "occupied_slots": 3,
        "mpc_zone": {
            "entry_point": {"lat": 30.549500, "lng": 114.342500},
            "exit_point": {"lat": 30.550500, "lng": 114.343500}
        }
    }'

# 3. 发布航线信息到mpc/RouteInfo
echo "3. 发布航线信息到 mpc/RouteInfo"
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "${CLIENT_ID}_pub" \
    -t "mpc/RouteInfo" \
    -m '{
        "route_id": 1,
        "name": "MPC主航线",
        "direction": "clockwise",
        "waypoints": [
            {"lat": 30.549000, "lng": 114.342000, "speed_limit": 3.0},
            {"lat": 30.550000, "lng": 114.343000, "speed_limit": 2.5}
        ],
        "mpc_parameters": {
            "prediction_horizon": 10,
            "control_horizon": 5,
            "sampling_time": 0.1
        }
    }'

# 4. 发布系统配置到mpc/Config
echo "4. 发布系统配置到 mpc/Config"
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "${CLIENT_ID}_pub" \
    -t "mpc/Config" \
    -m '{
        "boat": {
            "length": 0.75,
            "width": 0.47
        },
        "emergency_threshold_s": 5,
        "warning_threshold_s": 30,
        "max_boats": 30,
        "mpc_config": {
            "enable": true,
            "update_rate": 10,
            "prediction_model": "linear",
            "constraints": {
                "max_speed": 5.0,
                "max_acceleration": 2.0,
                "max_turn_rate": 30.0
            }
        }
    }'

echo ""
echo "=== 测试boat_pro系统发布到GCS主题 ==="

# 模拟boat_pro系统发布碰撞告警到gcs/CollisionAlert
echo "5. 模拟发布碰撞告警到 gcs/CollisionAlert"
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "boat_pro_gcs" \
    -t "gcs/CollisionAlert" \
    -m '{
        "current_boat_id": 1,
        "level": 2,
        "collision_time": 15.5,
        "collision_position": {
            "lat": 30.549832,
            "lng": 114.342922
        },
        "front_boat_ids": [2],
        "oncoming_boat_ids": [3],
        "decision_advice": "MPC紧急避让",
        "mpc_recommendation": {
            "action": "emergency_brake",
            "new_speed": 0.5,
            "new_heading": 30.0
        }
    }'

# 模拟发布安全状态到gcs/SafetyStatus
echo "6. 模拟发布安全状态到 gcs/SafetyStatus"
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "boat_pro_gcs" \
    -t "gcs/SafetyStatus" \
    -m '{
        "boat_id": 1,
        "status": "safe",
        "risk_level": 0,
        "last_check": 1692168000,
        "alerts_count": 0,
        "mpc_status": {
            "controller_active": true,
            "tracking_error": 0.1,
            "prediction_accuracy": 0.95
        }
    }'

# 模拟发布舰队命令到gcs/FleetCommand
echo "7. 模拟发布舰队命令到 gcs/FleetCommand"
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "boat_pro_gcs" \
    -t "gcs/FleetCommand" \
    -m '{
        "command": "formation_control",
        "target_boats": [1, 2, 3],
        "priority": "high",
        "timestamp": 1692168000,
        "mpc_formation": {
            "formation_type": "line",
            "spacing": 5.0,
            "leader_id": 1
        }
    }'

# 模拟发布系统状态到gcs/SystemStatus
echo "8. 模拟发布系统状态到 gcs/SystemStatus"
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "boat_pro_gcs" \
    -t "gcs/SystemStatus" \
    -m '{
        "status": "running",
        "active_boats": 3,
        "alerts": 1,
        "timestamp": 1692168000,
        "uptime": 3600,
        "mpc_system": {
            "controllers_active": 3,
            "average_tracking_error": 0.15,
            "computation_load": 0.65
        }
    }'

# 模拟发布心跳到gcs/Heartbeat
echo "9. 模拟发布心跳到 gcs/Heartbeat"
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "boat_pro_gcs" \
    -t "gcs/Heartbeat" \
    -m '{
        "boat_id": 1,
        "timestamp": 1692168000,
        "status": "alive",
        "sequence": 123,
        "mpc_heartbeat": {
            "controller_health": "good",
            "last_control_update": 1692167999
        }
    }'

echo ""
echo "=== MPC客户端测试完成 ==="
echo "✅ 所有MPC主题测试完成"
echo "注意: 实际测试需要MQTT代理在 $MQTT_HOST:$MQTT_PORT 运行"
echo "并且支持用户名/密码认证: $MQTT_USER/$MQTT_PASS"
echo ""
echo "MPC主题总结:"
echo "订阅主题 (MPC客户端订阅，GCS发布):"
for topic in "${MPC_PUBLISH_TOPICS[@]}"; do
    echo "  - $topic"
done
echo ""
echo "发布主题 (MPC客户端发布，GCS订阅):"
for topic in "${MPC_SUBSCRIBE_TOPICS[@]}"; do
    echo "  - $topic"
done
