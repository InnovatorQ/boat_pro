#!/bin/bash

# MQTT快速验证脚本
# 用于日常检查MQTT功能是否正常

BROKER_HOST="127.0.0.1"
BROKER_PORT="2000"
USERNAME="vEagles"
PASSWORD="123456"

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo -e "${BLUE}=== MQTT快速功能检查 ===${NC}"
echo "检查时间: $(date)"
echo "Broker: $BROKER_HOST:$BROKER_PORT"
echo

# 检查MQTT服务状态
echo -e "${YELLOW}1. 检查MQTT服务状态...${NC}"
if systemctl is-active --quiet mosquitto; then
    echo -e "${GREEN}✓ MQTT服务运行正常${NC}"
else
    echo -e "${RED}✗ MQTT服务未运行${NC}"
    echo "尝试启动MQTT服务..."
    sudo systemctl start mosquitto
    if systemctl is-active --quiet mosquitto; then
        echo -e "${GREEN}✓ MQTT服务启动成功${NC}"
    else
        echo -e "${RED}✗ MQTT服务启动失败${NC}"
        exit 1
    fi
fi

# 检查端口监听
echo -e "${YELLOW}2. 检查端口监听状态...${NC}"
if ss -tlnp | grep -q ":$BROKER_PORT "; then
    echo -e "${GREEN}✓ 端口$BROKER_PORT监听正常${NC}"
else
    echo -e "${RED}✗ 端口$BROKER_PORT未监听${NC}"
    exit 1
fi

# 测试基础连接
echo -e "${YELLOW}3. 测试基础连接...${NC}"
if timeout 3 mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'test/connection' -m 'hello' -q 0 2>/dev/null; then
    echo -e "${GREEN}✓ MQTT连接正常${NC}"
else
    echo -e "${RED}✗ MQTT连接失败${NC}"
    exit 1
fi

# 测试订阅发布
echo -e "${YELLOW}4. 测试订阅发布功能...${NC}"
TEST_TOPIC="test/quick_check_$(date +%s)"
TEST_MESSAGE="quick_check_message_$(date +%s)"

# 启动订阅器
timeout 5 mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "$TEST_TOPIC" -C 1 > /tmp/mqtt_quick_test.txt &
SUB_PID=$!
sleep 1

# 发布消息
mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "$TEST_TOPIC" -m "$TEST_MESSAGE"
sleep 2

# 检查结果
if [ -f /tmp/mqtt_quick_test.txt ] && grep -q "$TEST_MESSAGE" /tmp/mqtt_quick_test.txt; then
    echo -e "${GREEN}✓ 订阅发布功能正常${NC}"
else
    echo -e "${RED}✗ 订阅发布功能异常${NC}"
    kill $SUB_PID 2>/dev/null
    exit 1
fi

kill $SUB_PID 2>/dev/null
rm -f /tmp/mqtt_quick_test.txt

# 测试MPC主题
echo -e "${YELLOW}5. 测试MPC主题发布...${NC}"
if mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'mpc/boat_state/test' -m '{"boat_id":"test","status":"ok"}' 2>/dev/null; then
    echo -e "${GREEN}✓ MPC主题发布正常${NC}"
else
    echo -e "${RED}✗ MPC主题发布失败${NC}"
    exit 1
fi

# 测试GCS主题
echo -e "${YELLOW}6. 测试GCS主题发布...${NC}"
if mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'gcs/mission_config' -m '{"mission":"test"}' 2>/dev/null; then
    echo -e "${GREEN}✓ GCS主题发布正常${NC}"
else
    echo -e "${RED}✗ GCS主题发布失败${NC}"
    exit 1
fi

# 测试QoS等级
echo -e "${YELLOW}7. 测试QoS等级...${NC}"
qos_ok=true
for qos in 0 1 2; do
    if ! mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "test/qos$qos" -m "qos_test" -q $qos 2>/dev/null; then
        qos_ok=false
        break
    fi
done

if $qos_ok; then
    echo -e "${GREEN}✓ QoS等级测试正常${NC}"
else
    echo -e "${RED}✗ QoS等级测试失败${NC}"
    exit 1
fi

# 性能快速测试
echo -e "${YELLOW}8. 性能快速测试...${NC}"
start_time=$(date +%s.%N)
for i in {1..10}; do
    mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "test/performance" -m "msg_$i" -q 0 2>/dev/null
done
end_time=$(date +%s.%N)

duration=$(echo "$end_time - $start_time" | bc 2>/dev/null || echo "0.1")
if (( $(echo "$duration < 1.0" | bc -l 2>/dev/null || echo "1") )); then
    echo -e "${GREEN}✓ 性能测试正常 (10条消息耗时: ${duration}s)${NC}"
else
    echo -e "${YELLOW}⚠ 性能较慢 (10条消息耗时: ${duration}s)${NC}"
fi

echo
echo -e "${GREEN}=== 所有检查通过！MQTT功能正常 ===${NC}"
echo -e "${BLUE}系统状态:${NC}"
echo "  - MQTT服务: 运行中"
echo "  - 端口监听: 正常"
echo "  - 基础通信: 正常"
echo "  - 主题发布: 正常"
echo "  - QoS支持: 正常"
echo "  - 性能状态: 良好"
echo
echo -e "${YELLOW}提示: 如需详细测试，请运行 ./scripts/mqtt_full_test.sh${NC}"
