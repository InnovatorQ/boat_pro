#!/bin/bash

# MQTT实时通信演示脚本
# 演示MPC和GCS之间的双向通信

BROKER_HOST="127.0.0.1"
BROKER_PORT="2000"
USERNAME="vEagles"
PASSWORD="123456"

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
NC='\033[0m'

echo -e "${BLUE}=== MQTT实时通信演示 ===${NC}"
echo "演示MPC和GCS之间的双向通信"
echo "Broker: $BROKER_HOST:$BROKER_PORT"
echo

# 清理函数
cleanup() {
    echo -e "\n${YELLOW}正在清理进程...${NC}"
    kill $GCS_PID $MPC_PID 2>/dev/null
    exit 0
}

trap cleanup SIGINT SIGTERM

# 启动GCS监听器（订阅MPC发布的主题）
echo -e "${GREEN}🖥️  启动GCS监控中心（监听MPC数据）...${NC}"
{
    echo -e "${GREEN}[GCS监控] 等待MPC数据...${NC}"
    mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "mpc/#" -v | while read line; do
        topic=$(echo $line | cut -d' ' -f1)
        message=$(echo $line | cut -d' ' -f2-)
        case $topic in
            mpc/boat_state/*)
                boat_id=$(echo $topic | cut -d'/' -f3)
                echo -e "${GREEN}[GCS监控] 📍 收到船只${boat_id}状态: $message${NC}"
                ;;
            mpc/collision_alert/*)
                boat_id=$(echo $topic | cut -d'/' -f3)
                echo -e "${RED}[GCS监控] ⚠️  收到船只${boat_id}碰撞告警: $message${NC}"
                ;;
            mpc/safety_status/*)
                boat_id=$(echo $topic | cut -d'/' -f3)
                echo -e "${YELLOW}[GCS监控] 🛡️  收到船只${boat_id}安全状态: $message${NC}"
                ;;
            mpc/system_status)
                echo -e "${BLUE}[GCS监控] 🖥️  收到MPC系统状态: $message${NC}"
                ;;
            mpc/heartbeat/*)
                boat_id=$(echo $topic | cut -d'/' -f3)
                echo -e "${CYAN}[GCS监控] 💓 收到船只${boat_id}心跳: $message${NC}"
                ;;
            *)
                echo -e "${PURPLE}[GCS监控] 📨 收到其他消息 [$topic]: $message${NC}"
                ;;
        esac
    done
} &
GCS_PID=$!

sleep 1

# 启动MPC监听器（订阅GCS发布的主题）
echo -e "${BLUE}🚢 启动MPC执行单元（监听GCS指令）...${NC}"
{
    echo -e "${BLUE}[MPC执行] 等待GCS指令...${NC}"
    mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "gcs/#" -v | while read line; do
        topic=$(echo $line | cut -d' ' -f1)
        message=$(echo $line | cut -d' ' -f2-)
        case $topic in
            gcs/mission_config)
                echo -e "${BLUE}[MPC执行] 📋 收到任务配置: $message${NC}"
                ;;
            gcs/route_plan/*)
                route_id=$(echo $topic | cut -d'/' -f3)
                echo -e "${BLUE}[MPC执行] 🗺️  收到航线规划${route_id}: $message${NC}"
                ;;
            gcs/safety_params)
                echo -e "${YELLOW}[MPC执行] ⚙️  收到安全参数: $message${NC}"
                ;;
            gcs/emergency_override)
                echo -e "${RED}[MPC执行] 🚨 收到紧急接管指令: $message${NC}"
                ;;
            gcs/heartbeat)
                echo -e "${CYAN}[MPC执行] 💓 收到GCS心跳: $message${NC}"
                ;;
            *)
                echo -e "${PURPLE}[MPC执行] 📨 收到其他指令 [$topic]: $message${NC}"
                ;;
        esac
    done
} &
MPC_PID=$!

sleep 2

echo -e "${YELLOW}=== 开始演示通信流程 ===${NC}"
echo

# 1. MPC启动并发布系统状态
echo -e "${BLUE}1. MPC系统启动${NC}"
mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD \
    -t "mpc/system_status" \
    -m '{"system_id":"MPC_001","status":"OPERATIONAL","active_boats":3,"timestamp":'$(date +%s)'}'

sleep 2

# 2. GCS发布任务配置
echo -e "${GREEN}2. GCS下发任务配置${NC}"
mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD \
    -t "gcs/mission_config" \
    -m '{"mission_id":"DEMO_001","boat_count":3,"formation_type":"LINE","max_speed":3.0}'

sleep 2

# 3. MPC发布船只状态
echo -e "${BLUE}3. MPC上报船只状态${NC}"
for boat_id in 1 2 3; do
    lat=$(echo "30.55 + $boat_id * 0.001" | bc)
    lng=$(echo "114.34 + $boat_id * 0.001" | bc)
    mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD \
        -t "mpc/boat_state/$boat_id" \
        -m "{\"boat_id\":$boat_id,\"lat\":$lat,\"lng\":$lng,\"speed\":2.5,\"heading\":90,\"timestamp\":$(date +%s)}"
    sleep 0.5
done

sleep 2

# 4. MPC发布碰撞告警
echo -e "${RED}4. MPC发布碰撞告警${NC}"
mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD \
    -t "mpc/collision_alert/1" \
    -m '{"boat_id":1,"alert_level":2,"collision_time":15.5,"involved_boats":[2],"avoidance_action":"减速避让","timestamp":'$(date +%s)'}'

sleep 2

# 5. GCS发布紧急接管
echo -e "${RED}5. GCS发布紧急接管指令${NC}"
mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD \
    -t "gcs/emergency_override" \
    -m '{"override_id":"EMERGENCY_001","boat_ids":[1,2],"action":"STOP_ALL","reason":"碰撞风险","operator":"DEMO_OPERATOR","timestamp":'$(date +%s)'}'

sleep 2

# 6. 持续心跳演示
echo -e "${CYAN}6. 心跳监控演示（10秒）${NC}"
for i in {1..10}; do
    # MPC心跳
    mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD \
        -t "mpc/heartbeat/1" \
        -m "{\"boat_id\":1,\"status\":\"ACTIVE\",\"timestamp\":$(date +%s)}"
    
    # GCS心跳
    mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD \
        -t "gcs/heartbeat" \
        -m "{\"gcs_id\":\"GCS_001\",\"status\":\"ONLINE\",\"timestamp\":$(date +%s)}"
    
    sleep 1
done

echo
echo -e "${GREEN}=== 演示完成 ===${NC}"
echo -e "${YELLOW}演示了以下通信流程:${NC}"
echo "1. MPC系统启动状态上报"
echo "2. GCS任务配置下发"
echo "3. MPC船只状态实时上报"
echo "4. MPC碰撞告警上报"
echo "5. GCS紧急接管指令下发"
echo "6. 双向心跳监控"
echo
echo -e "${BLUE}按 Ctrl+C 退出演示${NC}"

# 保持演示运行
while true; do
    sleep 5
    # 模拟持续的船只状态更新
    boat_id=$((RANDOM % 3 + 1))
    lat=$(echo "30.55 + $boat_id * 0.001 + $(date +%s) * 0.0001" | bc)
    lng=$(echo "114.34 + $boat_id * 0.001 + $(date +%s) * 0.0001" | bc)
    speed=$(echo "2.0 + $RANDOM % 20 * 0.1" | bc)
    
    mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD \
        -t "mpc/boat_state/$boat_id" \
        -m "{\"boat_id\":$boat_id,\"lat\":$lat,\"lng\":$lng,\"speed\":$speed,\"heading\":90,\"timestamp\":$(date +%s)}" &
done
