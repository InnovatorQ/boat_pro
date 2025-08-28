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
    mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "mpc/CollisionAlert" -t "mpc/SafetyStatus" -t "mpc/" -t "mpc/SystemStatus" -v | while read line; do
        topic=$(echo $line | cut -d' ' -f1)
        message=$(echo $line | cut -d' ' -f2-)
        case $topic in
            mpc/CollisionAlert)
                echo -e "${RED}[GCS监控] ⚠️  收到碰撞告警: $message${NC}"
                ;;
            mpc/SafetyStatus)
                echo -e "${YELLOW}[GCS监控] 🛡️  收到安全状态: $message${NC}"
                ;;
            mpc/)
                echo -e "${BLUE}[GCS监控] 🚢 收到舰队命令: $message${NC}"
                ;;
            mpc/SystemStatus)
                echo -e "${BLUE}[GCS监控] 🖥️  收到MPC系统状态: $message${NC}"
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
    mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "mpc/BoatState" -t "mpc/DockInfo" -t "mpc/RouteInfo" -t "mpc/Config" -v | while read line; do
        topic=$(echo $line | cut -d' ' -f1)
        message=$(echo $line | cut -d' ' -f2-)
        case $topic in
            mpc/BoatState)
                echo -e "${BLUE}[MPC执行] 📍 收到船只状态: $message${NC}"
                ;;
            mpc/DockInfo)
                echo -e "${BLUE}[MPC执行] 🏠 收到船坞信息: $message${NC}"
                ;;
            mpc/RouteInfo)
                echo -e "${BLUE}[MPC执行] 🗺️  收到航线信息: $message${NC}"
                ;;
            mpc/Config)
                echo -e "${YELLOW}[MPC执行] ⚙️  收到系统配置: $message${NC}"
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
    -t "mpc/SystemStatus" \
    -m '{"system_id":"MPC_001","status":"OPERATIONAL","active_boats":3,"timestamp":'$(date +%s)'}'

sleep 2

# 2. GCS发布船只状态
echo -e "${GREEN}2. GCS下发船只状态数据${NC}"
mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD \
    -t "mpc/BoatState" \
    -m '{"boat_id":1,"lat":30.55,"lng":114.34,"speed":2.5,"heading":90,"status":"ACTIVE","timestamp":'$(date +%s)'}'

sleep 2

# 3. GCS发布航线信息
echo -e "${GREEN}3. GCS下发航线信息${NC}"
mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD \
    -t "mpc/RouteInfo" \
    -m '{"route_id":"ROUTE_001","waypoints":[{"lat":30.55,"lng":114.34},{"lat":30.56,"lng":114.35}],"max_speed":3.0}'

sleep 2

# 4. MPC发布碰撞告警
echo -e "${RED}4. MPC发布碰撞告警${NC}"
mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD \
    -t "mpc/CollisionAlert" \
    -m '{"alert_level":2,"avoidance_decision":"减速避让","alert_boat_id":1,"collision_position":{"lat":30.55,"lng":114.34},"collision_time":15.5,"timestamp":'$(date +%s)'}'

sleep 2

# 5. MPC发布安全状态
echo -e "${YELLOW}5. MPC发布安全状态${NC}"
mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD \
    -t "mpc/SafetyStatus" \
    -m '{"fleet_status":"WARNING","active_boats":3,"collision_risks":1,"emergency_stops":0,"timestamp":'$(date +%s)'}'

sleep 2

# 6. GCS发布系统配置
echo -e "${GREEN}6. GCS下发系统配置${NC}"
mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD \
    -t "mpc/Config" \
    -m '{"emergency_threshold_s":5,"warning_threshold_s":30,"max_boats":30,"min_route_gap_m":10}'

echo
echo -e "${GREEN}=== 演示完成 ===${NC}"
echo -e "${YELLOW}演示了以下通信流程:${NC}"
echo "1. MPC系统启动状态上报"
echo "2. GCS船只状态数据下发"
echo "3. GCS航线信息下发"
echo "4. MPC碰撞告警上报"
echo "5. MPC安全状态上报"
echo "6. GCS系统配置下发"
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
        -t "mpc/BoatState" \
        -m "{\"boat_id\":$boat_id,\"lat\":$lat,\"lng\":$lng,\"speed\":$speed,\"heading\":90,\"timestamp\":$(date +%s)}" &
done
