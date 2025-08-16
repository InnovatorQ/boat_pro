#!/bin/bash

# 分步手动验证脚本

set -e

echo "=== 无人船作业安全预测系统 - 分步手动验证 ==="
echo ""

# 颜色定义
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# 辅助函数
print_step() {
    echo -e "${BLUE}=== $1 ===${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

wait_for_user() {
    echo -e "${YELLOW}按回车键继续...${NC}"
    read
}

# 步骤1：环境检查
print_step "步骤1：环境检查"
echo "检查系统环境和依赖..."

echo "系统信息："
uname -a
echo ""

echo "检查MQTT服务状态："
if systemctl is-active --quiet mosquitto; then
    print_success "MQTT服务正在运行"
else
    print_warning "MQTT服务未运行，尝试启动..."
    sudo systemctl start mosquitto
    if systemctl is-active --quiet mosquitto; then
        print_success "MQTT服务启动成功"
    else
        print_error "MQTT服务启动失败"
        exit 1
    fi
fi

echo ""
echo "检查依赖包："
if dpkg -l | grep -q libmosquitto-dev; then
    print_success "libmosquitto-dev 已安装"
else
    print_error "libmosquitto-dev 未安装"
fi

if dpkg -l | grep -q libjsoncpp-dev; then
    print_success "libjsoncpp-dev 已安装"
else
    print_error "libjsoncpp-dev 未安装"
fi

wait_for_user

# 步骤2：编译检查
print_step "步骤2：编译检查"
echo "检查项目编译状态..."

if [ -f "build/mqtt_example" ]; then
    print_success "MQTT示例程序已编译"
else
    print_warning "MQTT示例程序未找到，开始编译..."
    mkdir -p build && cd build
    cmake .. && make -j$(nproc)
    cd ..
    if [ -f "build/mqtt_example" ]; then
        print_success "编译成功"
    else
        print_error "编译失败"
        exit 1
    fi
fi

if [ -f "build/simulation/boat_simulator" ]; then
    print_success "船只仿真器已编译"
else
    print_warning "船只仿真器未找到"
fi

wait_for_user

# 步骤3：基础MQTT通信测试
print_step "步骤3：基础MQTT通信测试"
echo "测试基本的MQTT发布/订阅功能..."

echo "启动消息订阅（后台运行3秒）..."
timeout 3s mosquitto_sub -h localhost -t "test/manual" > /tmp/mqtt_test.log 2>&1 &
SUB_PID=$!

sleep 1

echo "发布测试消息..."
mosquitto_pub -h localhost -t "test/manual" -m "手动验证测试消息"

sleep 2

if [ -s "/tmp/mqtt_test.log" ]; then
    print_success "MQTT基础通信正常"
    echo "接收到的消息："
    cat /tmp/mqtt_test.log
else
    print_error "MQTT基础通信失败"
fi

wait_for_user

# 步骤4：JSON消息测试
print_step "步骤4：JSON消息格式测试"
echo "测试JSON格式消息的发布和接收..."

echo "启动JSON消息订阅..."
timeout 3s mosquitto_sub -h localhost -t "boat_safety/test/json" > /tmp/json_test.log 2>&1 &

sleep 1

TIMESTAMP=$(date +%s)
JSON_MSG="{\"test\":\"manual_verification\",\"timestamp\":$TIMESTAMP,\"status\":\"testing\"}"
echo "发布JSON消息: $JSON_MSG"
mosquitto_pub -h localhost -t "boat_safety/test/json" -m "$JSON_MSG"

sleep 2

if [ -s "/tmp/json_test.log" ]; then
    print_success "JSON消息传输正常"
    echo "接收到的JSON消息："
    cat /tmp/json_test.log
    
    # 验证JSON格式
    if cat /tmp/json_test.log | python3 -m json.tool > /dev/null 2>&1; then
        print_success "JSON格式验证通过"
    else
        print_warning "JSON格式验证失败"
    fi
else
    print_error "JSON消息传输失败"
fi

wait_for_user

# 步骤5：MQTT示例程序测试
print_step "步骤5：MQTT示例程序测试"
echo "启动MQTT示例程序进行功能测试..."

echo "启动系统状态监控..."
timeout 15s mosquitto_sub -h localhost -t "boat_safety/output/system_status" > /tmp/system_status.log 2>&1 &
STATUS_SUB_PID=$!

echo "启动碰撞告警监控..."
timeout 15s mosquitto_sub -h localhost -t "boat_safety/output/collision_alert/+" > /tmp/collision_alerts.log 2>&1 &
ALERT_SUB_PID=$!

sleep 1

echo "启动MQTT示例程序（运行10秒）..."
cd build
timeout 12s ./mqtt_example > /tmp/mqtt_example.log 2>&1 &
EXAMPLE_PID=$!
cd ..

echo "程序运行中，PID: $EXAMPLE_PID"
echo "等待程序完成..."

# 等待程序结束
sleep 12

echo ""
echo "检查程序运行结果："

if [ -s "/tmp/mqtt_example.log" ]; then
    print_success "示例程序运行正常"
    echo "程序日志摘要："
    head -5 /tmp/mqtt_example.log
    echo "..."
    tail -5 /tmp/mqtt_example.log
else
    print_error "示例程序运行异常"
fi

echo ""
if [ -s "/tmp/system_status.log" ]; then
    print_success "系统状态消息接收正常"
    echo "系统状态："
    cat /tmp/system_status.log | head -1
else
    print_warning "未接收到系统状态消息"
fi

if [ -s "/tmp/collision_alerts.log" ]; then
    print_success "碰撞告警系统正常"
    echo "告警消息："
    cat /tmp/collision_alerts.log | head -1
else
    print_warning "未接收到碰撞告警（可能正常）"
fi

wait_for_user

# 步骤6：手动数据发送测试
print_step "步骤6：手动数据发送测试"
echo "手动发送各种类型的测试数据..."

# 重新启动示例程序用于接收测试数据
echo "重新启动示例程序..."
cd build
timeout 30s ./mqtt_example > /tmp/manual_test.log 2>&1 &
MANUAL_TEST_PID=$!
cd ..

sleep 2

echo "发送船只状态数据..."
TIMESTAMP=$(date +%s)
BOAT_STATE="{\"sysid\":1,\"timestamp\":$TIMESTAMP,\"lat\":30.549832,\"lng\":114.342922,\"heading\":90.0,\"speed\":2.5,\"status\":2,\"route_direction\":1}"
mosquitto_pub -h localhost -t "boat_safety/input/boat_state/1" -m "$BOAT_STATE"
print_success "船只状态数据已发送"

sleep 1

echo "发送高速船只数据（应触发告警）..."
HIGH_SPEED_BOAT="{\"sysid\":2,\"timestamp\":$TIMESTAMP,\"lat\":30.549900,\"lng\":114.342900,\"heading\":180.0,\"speed\":3.5,\"status\":2,\"route_direction\":1}"
mosquitto_pub -h localhost -t "boat_safety/input/boat_state/2" -m "$HIGH_SPEED_BOAT"
print_success "高速船只数据已发送"

sleep 1

echo "发送船坞信息..."
DOCK_INFO="{\"dock_id\":1,\"lat\":30.549100,\"lng\":114.343000}"
mosquitto_pub -h localhost -t "boat_safety/input/dock_info" -m "$DOCK_INFO"
print_success "船坞信息已发送"

sleep 1

echo "发送航线信息..."
ROUTE_INFO="{\"route_id\":1,\"direction\":1,\"points\":[{\"lat\":30.549500,\"lng\":114.342800},{\"lat\":30.549800,\"lng\":114.343300}]}"
mosquitto_pub -h localhost -t "boat_safety/input/route_info" -m "$ROUTE_INFO"
print_success "航线信息已发送"

sleep 2

echo "发送船坞锁定请求..."
DOCK_LOCK="{\"dock_id\":1,\"boat_id\":3,\"operation\":\"lock\",\"timestamp\":$TIMESTAMP}"
mosquitto_pub -h localhost -t "boat_safety/dock_management/1/lock_request" -m "$DOCK_LOCK"
print_success "船坞锁定请求已发送"

sleep 3

# 停止测试程序
kill $MANUAL_TEST_PID 2>/dev/null || true

echo ""
echo "检查数据处理结果："
if grep -q "Updated boat state" /tmp/manual_test.log; then
    print_success "船只状态数据处理正常"
    BOAT_UPDATES=$(grep -c "Updated boat state" /tmp/manual_test.log)
    echo "  处理了 $BOAT_UPDATES 次船只状态更新"
else
    print_warning "未检测到船只状态数据处理"
fi

if grep -q "Updated dock info" /tmp/manual_test.log; then
    print_success "船坞信息处理正常"
else
    print_warning "未检测到船坞信息处理"
fi

if grep -q "Updated route info" /tmp/manual_test.log; then
    print_success "航线信息处理正常"
else
    print_warning "未检测到航线信息处理"
fi

wait_for_user

# 步骤7：仿真系统测试（可选）
print_step "步骤7：仿真系统测试（可选）"
echo "是否要测试船只仿真系统？(y/n)"
read -r response

if [[ "$response" =~ ^[Yy]$ ]]; then
    if [ -f "build/simulation/boat_simulator" ]; then
        echo "启动碰撞告警监控..."
        timeout 20s mosquitto_sub -h localhost -t "boat_safety/output/collision_alert/+" > /tmp/sim_alerts.log 2>&1 &
        
        echo "启动安全预测系统..."
        cd build
        timeout 20s ./mqtt_example > /tmp/sim_safety.log 2>&1 &
        SAFETY_PID=$!
        cd ..
        
        sleep 2
        
        echo "启动船只仿真器（运行15秒）..."
        cd build/simulation
        timeout 15s ./boat_simulator > /tmp/sim_boats.log 2>&1 &
        SIM_PID=$!
        cd ../..
        
        echo "仿真运行中..."
        echo "安全系统PID: $SAFETY_PID"
        echo "仿真器PID: $SIM_PID"
        
        # 显示进度
        for i in {1..15}; do
            echo -n "."
            sleep 1
        done
        echo ""
        
        # 等待进程结束
        wait $SIM_PID 2>/dev/null || true
        kill $SAFETY_PID 2>/dev/null || true
        
        echo ""
        echo "仿真结果分析："
        
        if [ -s "/tmp/sim_boats.log" ]; then
            print_success "船只仿真器运行正常"
            echo "仿真器日志摘要："
            head -10 /tmp/sim_boats.log
            
            if grep -q "出坞" /tmp/sim_boats.log; then
                UNDOCK_COUNT=$(grep -c "出坞" /tmp/sim_boats.log)
                print_success "检测到 $UNDOCK_COUNT 次出坞活动"
            fi
        else
            print_error "船只仿真器运行异常"
        fi
        
        if [ -s "/tmp/sim_alerts.log" ]; then
            ALERT_COUNT=$(wc -l < /tmp/sim_alerts.log)
            print_success "生成了 $ALERT_COUNT 个碰撞告警"
            if [ $ALERT_COUNT -gt 0 ]; then
                echo "告警示例："
                head -1 /tmp/sim_alerts.log
            fi
        else
            print_warning "未生成碰撞告警"
        fi
        
        if [ -s "/tmp/sim_safety.log" ]; then
            if grep -q "Updated boat state" /tmp/sim_safety.log; then
                UPDATES=$(grep -c "Updated boat state" /tmp/sim_safety.log)
                print_success "安全系统处理了 $UPDATES 次船只状态更新"
            fi
        fi
        
    else
        print_warning "船只仿真器未编译，跳过仿真测试"
    fi
else
    echo "跳过仿真系统测试"
fi

wait_for_user

# 步骤8：结果总结
print_step "步骤8：验证结果总结"
echo "汇总所有验证结果..."

echo ""
echo "=== 验证结果汇总 ==="

# 统计各项功能
TOTAL_TESTS=0
PASSED_TESTS=0

echo ""
echo "基础功能验证："
TOTAL_TESTS=$((TOTAL_TESTS + 1))
if [ -s "/tmp/mqtt_test.log" ]; then
    print_success "MQTT基础通信"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    print_error "MQTT基础通信"
fi

TOTAL_TESTS=$((TOTAL_TESTS + 1))
if [ -s "/tmp/json_test.log" ]; then
    print_success "JSON消息处理"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    print_error "JSON消息处理"
fi

echo ""
echo "系统功能验证："
TOTAL_TESTS=$((TOTAL_TESTS + 1))
if [ -s "/tmp/mqtt_example.log" ] && grep -q "MQTT connected successfully" /tmp/mqtt_example.log; then
    print_success "MQTT示例程序运行"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    print_error "MQTT示例程序运行"
fi

TOTAL_TESTS=$((TOTAL_TESTS + 1))
if [ -s "/tmp/system_status.log" ]; then
    print_success "系统状态发布"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    print_error "系统状态发布"
fi

TOTAL_TESTS=$((TOTAL_TESTS + 1))
if [ -s "/tmp/manual_test.log" ] && grep -q "Updated boat state" /tmp/manual_test.log; then
    print_success "船只数据处理"
    PASSED_TESTS=$((PASSED_TESTS + 1))
else
    print_error "船只数据处理"
fi

# 可选的仿真测试
if [ -s "/tmp/sim_boats.log" ]; then
    echo ""
    echo "仿真系统验证："
    TOTAL_TESTS=$((TOTAL_TESTS + 1))
    if grep -q "开始真实场景仿真" /tmp/sim_boats.log; then
        print_success "船只仿真器运行"
        PASSED_TESTS=$((PASSED_TESTS + 1))
    else
        print_error "船只仿真器运行"
    fi
fi

echo ""
echo "=== 最终评估 ==="
echo "通过测试: $PASSED_TESTS/$TOTAL_TESTS"

PASS_RATE=$((PASSED_TESTS * 100 / TOTAL_TESTS))
echo "通过率: $PASS_RATE%"

if [ $PASS_RATE -ge 90 ]; then
    print_success "验证结果：优秀 - 系统功能完全正常"
    echo "🎉 建议：可以投入生产环境使用"
elif [ $PASS_RATE -ge 70 ]; then
    print_success "验证结果：良好 - 系统基本功能正常"
    echo "👍 建议：修复发现的问题后部署"
elif [ $PASS_RATE -ge 50 ]; then
    print_warning "验证结果：一般 - 系统存在一些问题"
    echo "🔧 建议：需要进一步调试和优化"
else
    print_error "验证结果：不合格 - 系统存在严重问题"
    echo "🚨 建议：需要重新检查和修复"
fi

echo ""
echo "=== 详细日志文件 ==="
echo "以下文件包含详细的验证日志："
ls -la /tmp/*test*.log /tmp/*mqtt*.log /tmp/*sim*.log 2>/dev/null || echo "无日志文件"

echo ""
echo "=== 手动验证完成 ==="
print_success "感谢您完成手动验证！"
echo "如有问题，请查看详细日志文件或参考 docs/MANUAL_VERIFICATION_GUIDE.md"
