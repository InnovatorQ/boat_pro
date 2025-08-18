#!/bin/bash

# MQTT数据接收测试脚本
# 用于演示如何通过MQTT接口接收外界发来的数据

echo "=========================================="
echo "  无人船MQTT数据接收测试"
echo "=========================================="

# 检查mosquitto是否运行
if ! systemctl is-active --quiet mosquitto; then
    echo "❌ mosquitto服务未运行，正在启动..."
    sudo systemctl start mosquitto
    sleep 2
fi

echo "✅ mosquitto服务正在运行"

# 检查可执行文件是否存在
if [ ! -f "../build/mqtt_data_receiver" ]; then
    echo "❌ mqtt_data_receiver程序不存在，请先编译项目"
    exit 1
fi

echo "✅ 找到数据接收器程序"

# 创建测试数据目录
mkdir -p test_data

# 创建测试数据文件
cat > test_data/boat1_state.json << EOF
{
  "sysid": 1,
  "timestamp": $(date +%s.%3N),
  "lat": 30.549832,
  "lng": 114.342922,
  "heading": 90.0,
  "speed": 2.5,
  "status": 2,
  "route_direction": 1
}
EOF

cat > test_data/boat2_state.json << EOF
{
  "sysid": 2,
  "timestamp": $(date +%s.%3N),
  "lat": 30.549900,
  "lng": 114.343000,
  "heading": 270.0,
  "speed": 1.8,
  "status": 2,
  "route_direction": 2
}
EOF

cat > test_data/collision_alert.json << EOF
{
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
}
EOF

cat > test_data/system_config.json << EOF
{
  "boat": {
    "length": 0.80,
    "width": 0.50
  },
  "emergency_threshold_s": 3,
  "warning_threshold_s": 25,
  "max_boats": 25,
  "min_route_gap_m": 12
}
EOF

cat > test_data/fleet_command.json << EOF
{
  "boat_id": 1,
  "command": "slow_down",
  "target_speed": 1.0,
  "timestamp": $(date +%s),
  "reason": "collision_warning"
}
EOF

echo "✅ 测试数据文件已创建"

# 启动数据接收器（后台运行）
echo "🚀 启动MQTT数据接收器..."
cd ../build
./mqtt_data_receiver &
RECEIVER_PID=$!

# 等待接收器启动
sleep 3

echo ""
echo "📡 开始发送测试数据..."
echo ""

# 发送船只1状态数据
echo "1. 发送船只1状态数据..."
mosquitto_pub -h localhost -t "boat_pro/boat_state/1" -f ../scripts/test_data/boat1_state.json
sleep 2

# 发送船只2状态数据
echo "2. 发送船只2状态数据..."
mosquitto_pub -h localhost -t "boat_pro/boat_state/2" -f ../scripts/test_data/boat2_state.json
sleep 2

# 发送碰撞告警
echo "3. 发送碰撞告警..."
mosquitto_pub -h localhost -t "boat_pro/collision_alert/1" -f ../scripts/test_data/collision_alert.json
sleep 2

# 发送系统配置更新
echo "4. 发送系统配置更新..."
mosquitto_pub -h localhost -t "boat_pro/system_config" -f ../scripts/test_data/system_config.json
sleep 2

# 发送舰队命令
echo "5. 发送舰队命令..."
mosquitto_pub -h localhost -t "boat_pro/fleet_command/1" -f ../scripts/test_data/fleet_command.json
sleep 2

# 发送心跳消息
echo "6. 发送心跳消息..."
mosquitto_pub -h localhost -t "boat_pro/heartbeat/1" -m '{"boat_id":1,"timestamp":'$(date +%s)',"status":"alive"}'
sleep 2

# 发送船坞信息
echo "7. 发送船坞信息..."
mosquitto_pub -h localhost -t "boat_pro/dock_info" -m '{"dock_id":1,"lat":30.549100,"lng":114.343000}'
sleep 2

# 发送航线信息
echo "8. 发送航线信息..."
mosquitto_pub -h localhost -t "boat_pro/route_info" -m '{"route_id":1,"direction":1,"points":[{"lat":30.549500,"lng":114.342800},{"lat":30.549800,"lng":114.343300}]}'
sleep 2

echo ""
echo "✅ 所有测试数据已发送完成"
echo ""
echo "📊 数据接收器将继续运行并显示统计信息..."
echo "   按 Ctrl+C 停止接收器"
echo ""

# 等待用户中断
wait $RECEIVER_PID

# 清理测试数据
rm -rf test_data

echo ""
echo "🏁 测试完成"
