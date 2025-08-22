#!/bin/bash

# 无人船作业安全预测系统构建脚本

set -e  # 遇到错误时退出

echo "=== 无人船作业安全预测系统 MQTT 接口构建脚本 ==="

# 检查依赖
echo "检查系统依赖..."

# 检查是否安装了必要的包
check_package() {
    if ! dpkg -l | grep -q "^ii  $1 "; then
        echo "错误: 未找到包 $1"
        echo "请运行: sudo apt-get install $1"
        exit 1
    else
        echo "✓ 找到包: $1"
    fi
}

# 检查必要的开发包
check_package "libmosquitto-dev"
check_package "libjsoncpp-dev"
check_package "cmake"
check_package "build-essential"

# 检查MQTT代理是否运行
echo "检查MQTT代理状态..."
if systemctl is-active --quiet mosquitto; then
    echo "✓ MQTT代理 (mosquitto) 正在运行"
else
    echo "⚠ MQTT代理未运行，尝试启动..."
    if sudo systemctl start mosquitto; then
        echo "✓ MQTT代理启动成功"
    else
        echo "⚠ 无法启动MQTT代理，请手动检查"
    fi
fi

# 创建构建目录
echo "创建构建目录..."
if [ -d "build" ]; then
    echo "清理现有构建目录..."
    rm -rf build
fi
mkdir build
cd build

# 运行CMake配置
echo "运行CMake配置..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译项目
echo "编译项目..."
make -j$(nproc)

echo "=== 构建完成 ==="
echo ""
echo "可执行文件位置:"
echo "  - MQTT示例程序: $(pwd)/mqtt_example"
echo ""
echo "运行示例:"
echo "  cd $(pwd)"
echo "  ./mqtt_example"
echo ""
echo "测试MQTT通信:"
echo "  # 订阅MPC发布的消息"
echo "  mosquitto_sub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 -t 'mpc/#' -v"
echo ""
echo "  # 发布GCS配置消息"
echo "  mosquitto_pub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 -t 'gcs/mission_config' \\"
echo "    -m '{\"mission_id\":\"TEST_001\",\"boat_count\":3,\"formation_type\":\"LINE\",\"max_speed\":3.0}'"
