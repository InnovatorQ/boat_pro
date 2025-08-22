#!/bin/bash

# 运行测试脚本

echo "运行无人船安全预测系统测试..."

cd "$(dirname "$0")/.."

# 检查构建目录
if [ ! -d "build" ]; then
    echo "构建目录不存在，请先构建项目"
    exit 1
fi

# 运行碰撞检测测试
if [ -f "build/test_collision_detector" ]; then
    echo "运行碰撞检测测试..."
    ./build/test_collision_detector
    if [ $? -ne 0 ]; then
        echo "碰撞检测测试失败!"
        exit 1
    fi
    echo "✓ 碰撞检测测试通过"
else
    echo "⚠ 碰撞检测测试程序不存在"
fi

# 运行通信测试
if [ -f "build/test_communication" ]; then
    echo "运行通信测试..."
    ./build/test_communication
    if [ $? -ne 0 ]; then
        echo "通信测试失败!"
        exit 1
    fi
    echo "✓ 通信测试通过"
else
    echo "⚠ 通信测试程序不存在"
fi

# 运行MQTT测试
if [ -f "build/mqtt_test" ]; then
    echo "运行MQTT测试..."
    ./build/mqtt_test
    if [ $? -ne 0 ]; then
        echo "MQTT测试失败!"
        exit 1
    fi
    echo "✓ MQTT测试通过"
else
    echo "⚠ MQTT测试程序不存在"
fi

echo "所有可用测试通过!"