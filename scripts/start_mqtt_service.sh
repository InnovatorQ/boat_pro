#!/bin/bash

# MQTT服务启动脚本 - 使用端口2000
echo "=== 启动boat_pro MQTT服务 ==="

MQTT_CONFIG="/home/qzx/code/cpp/boat_pro/config/mosquitto_custom.conf"
MQTT_PID_FILE="/tmp/mosquitto_boat_pro.pid"
MQTT_LOG_FILE="/tmp/mosquitto_boat_pro.log"

# 颜色定义
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

# 检查是否已经运行
if [ -f "$MQTT_PID_FILE" ]; then
    PID=$(cat "$MQTT_PID_FILE")
    if ps -p "$PID" > /dev/null 2>&1; then
        echo -e "${YELLOW}MQTT服务已在运行 (PID: $PID)${NC}"
        echo "端口2000监听状态:"
        ss -tlnp | grep ":2000 " || echo "端口2000未监听"
        exit 0
    else
        echo -e "${YELLOW}清理旧的PID文件${NC}"
        rm -f "$MQTT_PID_FILE"
    fi
fi

# 创建必要的目录
mkdir -p /tmp/mosquitto_boat_pro

# 检查配置文件
if [ ! -f "$MQTT_CONFIG" ]; then
    echo -e "${RED}错误: 配置文件不存在: $MQTT_CONFIG${NC}"
    exit 1
fi

echo -e "${BLUE}使用配置文件: $MQTT_CONFIG${NC}"
echo -e "${BLUE}日志文件: $MQTT_LOG_FILE${NC}"

# 启动MQTT服务
echo -e "${YELLOW}启动MQTT服务...${NC}"
mosquitto -c "$MQTT_CONFIG" -d

# 等待服务启动
sleep 2

# 检查服务状态
if [ -f "$MQTT_PID_FILE" ]; then
    PID=$(cat "$MQTT_PID_FILE")
    if ps -p "$PID" > /dev/null 2>&1; then
        echo -e "${GREEN}✓ MQTT服务启动成功 (PID: $PID)${NC}"
        
        # 检查端口监听
        if ss -tlnp | grep -q ":2000 "; then
            echo -e "${GREEN}✓ 端口2000监听正常${NC}"
        else
            echo -e "${RED}✗ 端口2000未监听${NC}"
        fi
        
        # 显示日志
        if [ -f "$MQTT_LOG_FILE" ]; then
            echo -e "${BLUE}最新日志:${NC}"
            tail -5 "$MQTT_LOG_FILE"
        fi
        
    else
        echo -e "${RED}✗ MQTT服务启动失败${NC}"
        if [ -f "$MQTT_LOG_FILE" ]; then
            echo -e "${RED}错误日志:${NC}"
            tail -10 "$MQTT_LOG_FILE"
        fi
        exit 1
    fi
else
    echo -e "${RED}✗ PID文件未创建，服务可能启动失败${NC}"
    exit 1
fi

echo
echo -e "${GREEN}=== MQTT服务配置信息 ===${NC}"
echo "监听地址: 127.0.0.1:2000"
echo "认证方式: 匿名连接"
echo "PID文件: $MQTT_PID_FILE"
echo "日志文件: $MQTT_LOG_FILE"
echo "配置文件: $MQTT_CONFIG"
echo
echo -e "${YELLOW}测试连接:${NC}"
echo "mosquitto_pub -h 127.0.0.1 -p 2000 -t 'test' -m 'hello'"
echo "mosquitto_sub -h 127.0.0.1 -p 2000 -t 'test'"
