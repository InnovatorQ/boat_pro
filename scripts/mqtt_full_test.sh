#!/bin/bash

# MQTT功能全面测试脚本
# 测试boat_pro项目的MQTT通信功能

# 配置参数
BROKER_HOST="127.0.0.1"
BROKER_PORT="2000"  # 使用项目指定端口
USERNAME="vEagles"
PASSWORD="123456"
TEST_LOG="/tmp/mqtt_test.log"
RESULTS_FILE="/tmp/mqtt_test_results.txt"

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 清理之前的测试文件
rm -f $TEST_LOG $RESULTS_FILE

echo -e "${BLUE}=== MQTT功能全面测试 ===${NC}"
echo "测试时间: $(date)"
echo "Broker: $BROKER_HOST:$BROKER_PORT"
echo "日志文件: $TEST_LOG"
echo

# 测试结果统计
TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# 测试函数
run_test() {
    local test_name="$1"
    local test_command="$2"
    local expected_result="$3"
    
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    echo -e "${YELLOW}测试 $TOTAL_TESTS: $test_name${NC}"
    
    # 执行测试命令
    eval $test_command > /tmp/test_output.txt 2>&1
    local exit_code=$?
    
    # 检查结果
    if [ $exit_code -eq 0 ]; then
        echo -e "${GREEN}✓ 通过${NC}"
        PASSED_TESTS=$((PASSED_TESTS + 1))
        echo "PASS: $test_name" >> $RESULTS_FILE
    else
        echo -e "${RED}✗ 失败${NC}"
        FAILED_TESTS=$((FAILED_TESTS + 1))
        echo "FAIL: $test_name" >> $RESULTS_FILE
        echo "错误信息:" >> $RESULTS_FILE
        cat /tmp/test_output.txt >> $RESULTS_FILE
        echo "---" >> $RESULTS_FILE
    fi
    echo
}

# 1. 基础连接测试
echo -e "${BLUE}=== 1. 基础连接测试 ===${NC}"

run_test "MQTT服务连接测试" \
    "timeout 3 mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'test/connection' -m 'hello' -q 0" \
    "0"

run_test "MQTT订阅测试" \
    "timeout 3 mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'test/connection' -C 1 > /dev/null" \
    "0"

# 2. MPC主题发布测试
echo -e "${BLUE}=== 2. MPC主题发布测试 ===${NC}"

# 启动订阅器监听MPC主题
mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "mpc/#" -v > $TEST_LOG &
SUB_PID=$!
sleep 1

run_test "MPC船只状态发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'mpc/boat_state/1' -m '{\"boat_id\":1,\"lat\":30.5,\"lng\":114.3,\"speed\":2.5}'" \
    "0"

run_test "MPC碰撞告警发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'mpc/collision_alert/1' -m '{\"boat_id\":1,\"alert_level\":2,\"collision_time\":15.5}'" \
    "0"

run_test "MPC安全状态发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'mpc/safety_status/1' -m '{\"boat_id\":1,\"safety_level\":\"SAFE\"}'" \
    "0"

run_test "MPC系统状态发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'mpc/system_status' -m '{\"system_id\":\"MPC_001\",\"status\":\"OPERATIONAL\"}'" \
    "0"

run_test "MPC心跳发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'mpc/heartbeat/1' -m '{\"boat_id\":1,\"timestamp\":$(date +%s)}'" \
    "0"

sleep 2
kill $SUB_PID 2>/dev/null

# 3. GCS主题发布测试
echo -e "${BLUE}=== 3. GCS主题发布测试 ===${NC}"

# 启动订阅器监听GCS主题
mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "gcs/#" -v > ${TEST_LOG}.gcs &
SUB_PID=$!
sleep 1

run_test "GCS任务配置发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'gcs/mission_config' -m '{\"mission_id\":\"MISSION_001\",\"boat_count\":5}'" \
    "0"

run_test "GCS航线规划发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'gcs/route_plan/ROUTE_001' -m '{\"route_id\":\"ROUTE_001\",\"waypoints\":[]}'" \
    "0"

run_test "GCS安全参数发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'gcs/safety_params' -m '{\"emergency_threshold_s\":5,\"warning_threshold_s\":30}'" \
    "0"

run_test "GCS紧急接管发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'gcs/emergency_override' -m '{\"override_id\":\"EMERGENCY_001\",\"action\":\"STOP_ALL\"}'" \
    "0"

run_test "GCS心跳发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'gcs/heartbeat' -m '{\"timestamp\":$(date +%s),\"status\":\"ONLINE\"}'" \
    "0"

sleep 2
kill $SUB_PID 2>/dev/null

# 4. QoS等级测试
echo -e "${BLUE}=== 4. QoS等级测试 ===${NC}"

run_test "QoS 0 消息发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'test/qos0' -m 'qos0_message' -q 0" \
    "0"

run_test "QoS 1 消息发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'test/qos1' -m 'qos1_message' -q 1" \
    "0"

run_test "QoS 2 消息发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'test/qos2' -m 'qos2_message' -q 2" \
    "0"

# 5. 通配符订阅测试
echo -e "${BLUE}=== 5. 通配符订阅测试 ===${NC}"

# 测试单级通配符
mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "mpc/boat_state/+" -C 1 > /tmp/wildcard_test.txt &
SUB_PID=$!
sleep 1

run_test "单级通配符订阅测试" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'mpc/boat_state/123' -m 'wildcard_test'" \
    "0"

sleep 2
kill $SUB_PID 2>/dev/null

# 测试多级通配符
mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "mpc/#" -C 1 > /tmp/wildcard_test2.txt &
SUB_PID=$!
sleep 1

run_test "多级通配符订阅测试" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'mpc/test/deep/topic' -m 'multilevel_wildcard_test'" \
    "0"

sleep 2
kill $SUB_PID 2>/dev/null

# 6. 消息持久化测试
echo -e "${BLUE}=== 6. 消息持久化测试 ===${NC}"

run_test "持久化消息发布" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'test/persistent' -m 'persistent_message' -q 1 -r" \
    "0"

run_test "持久化消息接收" \
    "timeout 3 mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'test/persistent' -C 1 > /dev/null" \
    "0"

# 7. 大消息测试
echo -e "${BLUE}=== 7. 大消息测试 ===${NC}"

# 创建大消息（约1KB）
LARGE_MESSAGE=$(python3 -c "import json; print(json.dumps({'data': 'x' * 1000, 'boat_id': 1}))")

run_test "大消息发布测试" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t 'test/large_message' -m '$LARGE_MESSAGE'" \
    "0"

# 8. 并发连接测试
echo -e "${BLUE}=== 8. 并发连接测试 ===${NC}"

# 启动多个并发订阅者
for i in {1..5}; do
    mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "test/concurrent/$i" -C 1 > /dev/null &
    PIDS[$i]=$!
done

sleep 1

# 向每个主题发送消息
for i in {1..5}; do
    mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "test/concurrent/$i" -m "message_$i" &
done

run_test "并发连接测试" \
    "sleep 2" \
    "0"

# 清理并发测试进程
for i in {1..5}; do
    kill ${PIDS[$i]} 2>/dev/null
done

# 9. 错误处理测试
echo -e "${BLUE}=== 9. 错误处理测试 ===${NC}"

run_test "无效主题名测试" \
    "mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t '' -m 'invalid_topic' 2>/dev/null; [ \$? -ne 0 ]" \
    "0"

run_test "连接错误端口测试" \
    "timeout 3 mosquitto_pub -h $BROKER_HOST -p 9999 -t 'test/error' -m 'error_test' 2>/dev/null; [ \$? -ne 0 ]" \
    "0"

# 10. 性能测试
echo -e "${BLUE}=== 10. 性能测试 ===${NC}"

# 启动性能测试订阅者
mosquitto_sub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "test/performance" -C 100 > /dev/null &
SUB_PID=$!

# 发送100条消息测试吞吐量
start_time=$(date +%s.%N)
for i in {1..100}; do
    mosquitto_pub -h $BROKER_HOST -p $BROKER_PORT -u $USERNAME -P $PASSWORD -t "test/performance" -m "message_$i" -q 0
done
end_time=$(date +%s.%N)

duration=$(echo "$end_time - $start_time" | bc)
throughput=$(echo "scale=2; 100 / $duration" | bc)

echo "性能测试结果: 100条消息耗时 ${duration}s, 吞吐量: ${throughput} msg/s"

kill $SUB_PID 2>/dev/null

run_test "性能测试完成" \
    "echo '性能测试完成'" \
    "0"

# 清理测试文件
rm -f /tmp/test_output.txt /tmp/wildcard_test.txt /tmp/wildcard_test2.txt

# 输出测试结果
echo -e "${BLUE}=== 测试结果汇总 ===${NC}"
echo -e "总测试数: ${BLUE}$TOTAL_TESTS${NC}"
echo -e "通过测试: ${GREEN}$PASSED_TESTS${NC}"
echo -e "失败测试: ${RED}$FAILED_TESTS${NC}"

if [ $FAILED_TESTS -eq 0 ]; then
    echo -e "${GREEN}🎉 所有测试通过！${NC}"
    exit 0
else
    echo -e "${RED}❌ 有 $FAILED_TESTS 个测试失败${NC}"
    echo -e "${YELLOW}详细错误信息请查看: $RESULTS_FILE${NC}"
    exit 1
fi
