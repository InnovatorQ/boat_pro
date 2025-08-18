#!/bin/bash

# MQTT客户端配置测试脚本
# 用于验证boat_pro系统与客户端MQTT配置的兼容性

echo "=== boat_pro MQTT客户端配置测试 ==="
echo "客户端配置:"
echo "  IP地址: 127.0.0.1"
echo "  端口: 2000"
echo "  账号: vEagles"
echo "  密码: 123456"
echo "  客户端ID: 1000"
echo ""

# 检查mosquitto客户端工具是否可用
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

# 订阅主题
SUBSCRIBE_TOPICS=(
    "dock/BoatState"
    "dock/DockInfo"
    "dock/RouteInfo"
    "dock/Config"
)

# 发布主题
PUBLISH_TOPICS=(
    "dock/CollisionAlert"
    "dock/SafetyStatus"
    "dock/FleetCommand"
    "dock/SystemStatus"
    "dock/Heartbeat"
)

echo "=== 测试订阅主题 ==="
for topic in "${SUBSCRIBE_TOPICS[@]}"; do
    echo "测试订阅主题: $topic"
    timeout 2s mosquitto_sub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "${CLIENT_ID}_sub" -t "$topic" -C 1 &
    sleep 0.5
done

echo ""
echo "=== 测试发布主题 ==="

# 测试发布船只状态数据（模拟客户端发布）
echo "发布测试船只状态数据..."
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "${CLIENT_ID}_pub" \
    -t "dock/BoatState" \
    -m '{"sysid":1,"lat":30.549832,"lng":114.342922,"speed":2.5,"heading":45.0,"timestamp":1692168000}'

# 测试发布船坞信息
echo "发布测试船坞信息..."
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "${CLIENT_ID}_pub" \
    -t "dock/DockInfo" \
    -m '{"dock_id":1,"position":{"lat":30.550000,"lng":114.343000},"capacity":10,"status":"available"}'

# 测试发布航线信息
echo "发布测试航线信息..."
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "${CLIENT_ID}_pub" \
    -t "dock/RouteInfo" \
    -m '{"route_id":1,"name":"主航线","waypoints":[{"lat":30.549000,"lng":114.342000},{"lat":30.550000,"lng":114.343000}]}'

# 测试发布系统配置
echo "发布测试系统配置..."
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "${CLIENT_ID}_pub" \
    -t "dock/Config" \
    -m '{"boat_length":0.75,"boat_width":0.47,"emergency_threshold_s":5,"warning_threshold_s":30}'

echo ""
echo "=== 测试boat_pro系统发布的主题 ==="

# 模拟boat_pro系统发布碰撞告警
echo "模拟发布碰撞告警..."
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "boat_pro_test" \
    -t "dock/CollisionAlert" \
    -m '{"current_boat_id":1,"level":2,"collision_time":15.5,"collision_position":{"lat":30.549832,"lng":114.342922},"front_boat_ids":[2],"decision_advice":"减速避让"}'

# 模拟发布安全状态
echo "模拟发布安全状态..."
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "boat_pro_test" \
    -t "dock/SafetyStatus" \
    -m '{"boat_id":1,"status":"safe","last_check":1692168000}'

# 模拟发布舰队命令
echo "模拟发布舰队命令..."
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "boat_pro_test" \
    -t "dock/FleetCommand" \
    -m '{"command":"stop","target_boats":[1,2,3],"timestamp":1692168000}'

# 模拟发布系统状态
echo "模拟发布系统状态..."
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "boat_pro_test" \
    -t "dock/SystemStatus" \
    -m '{"status":"running","active_boats":3,"alerts":1,"timestamp":1692168000}'

# 模拟发布心跳消息
echo "模拟发布心跳消息..."
mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -u $MQTT_USER -P $MQTT_PASS -i "boat_pro_test" \
    -t "dock/Heartbeat" \
    -m '{"boat_id":1,"timestamp":1692168000,"status":"alive"}'

echo ""
echo "=== 测试完成 ==="
echo "如果没有错误信息，说明MQTT客户端配置正确"
echo "注意: 实际测试需要MQTT代理在 $MQTT_HOST:$MQTT_PORT 运行"
echo "并且支持用户名/密码认证: $MQTT_USER/$MQTT_PASS"
