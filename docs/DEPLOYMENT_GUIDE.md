# 部署指南

## 系统要求

### 操作系统
- **推荐**: Ubuntu 20.04 LTS 或更高版本
- **支持**: Debian 10+ 或其他基于Debian的发行版
- **架构**: x86_64 (AMD64)

### 硬件要求
- **CPU**: 双核1GHz以上处理器
- **内存**: 512MB以上可用内存
- **存储**: 100MB可用磁盘空间
- **网络**: 支持TCP/IP通信，稳定的网络连接

### 软件依赖

#### 必需依赖
```bash
# 构建工具
sudo apt-get update
sudo apt-get install build-essential cmake

# 核心库
sudo apt-get install libjsoncpp-dev libmosquitto-dev

# MQTT服务
sudo apt-get install mosquitto mosquitto-clients
```

#### 可选依赖
```bash
# 开发工具
sudo apt-get install git vim

# 调试工具
sudo apt-get install gdb valgrind
```

## 快速部署

### 1. 获取源码
```bash
git clone <repository-url>
cd boat_pro
```

### 2. 一键构建
```bash
# 使用构建脚本（推荐）
./scripts/build.sh

# 或手动构建
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### 3. 配置MQTT服务
```bash
# 启动MQTT服务
sudo systemctl start mosquitto
sudo systemctl enable mosquitto

# 验证服务状态
sudo systemctl status mosquitto
```

### 4. 运行系统
```bash
cd build
./boat_pro
```

## 详细部署步骤

### 步骤1: 环境准备

#### 1.1 系统更新
```bash
sudo apt-get update
sudo apt-get upgrade -y
```

#### 1.2 安装构建工具
```bash
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config
```

#### 1.3 安装核心依赖
```bash
# JsonCpp库 - JSON数据处理
sudo apt-get install -y libjsoncpp-dev

# Mosquitto库 - MQTT通信
sudo apt-get install -y libmosquitto-dev

# MQTT Broker和客户端工具
sudo apt-get install -y mosquitto mosquitto-clients
```

### 步骤2: MQTT服务配置

#### 2.1 配置MQTT Broker
```bash
# 创建配置文件
sudo tee /etc/mosquitto/conf.d/boat_pro.conf << EOF
# 监听端口
port 2000

# 允许匿名连接（生产环境建议使用认证）
allow_anonymous false

# 密码文件
password_file /etc/mosquitto/passwd

# 访问控制列表
acl_file /etc/mosquitto/acl
EOF
```

#### 2.2 创建用户认证
```bash
# 创建用户
sudo mosquitto_passwd -c /etc/mosquitto/passwd vEagles

# 设置密码为 123456
# 输入密码时会提示输入
```

#### 2.3 配置访问控制
```bash
sudo tee /etc/mosquitto/acl << EOF
# vEagles用户权限
user vEagles
topic readwrite mpc/#
topic readwrite gcs/#
EOF
```

#### 2.4 启动MQTT服务
```bash
# 重启服务以应用配置
sudo systemctl restart mosquitto

# 设置开机自启
sudo systemctl enable mosquitto

# 验证服务状态
sudo systemctl status mosquitto
```

### 步骤3: 编译项目

#### 3.1 获取源码
```bash
# 克隆项目（如果还没有）
git clone <repository-url>
cd boat_pro

# 或更新现有代码
git pull origin main
```

#### 3.2 构建项目
```bash
# 使用构建脚本（推荐）
chmod +x scripts/build.sh
./scripts/build.sh

# 验证构建结果
ls -la build/
```

#### 3.3 验证构建
```bash
cd build

# 检查主程序
./boat_pro --version

# 运行测试程序
./collision_angle_test
./avoidance_decision_test
```

### 步骤4: 配置文件设置

#### 4.1 MQTT配置
编辑 `config/mqtt_config.json`:
```json
{
    "broker": {
        "host": "127.0.0.1",
        "port": 2000,
        "username": "vEagles",
        "password": "123456",
        "client_id": "MPC_CLIENT_001"
    },
    "topics": {
        "boat_state": "mpc/BoatState",
        "dock_info": "mpc/DockInfo",
        "route_info": "mpc/RouteInfo",
        "config": "mpc/Config",
        "collision_alert": "mpc/CollisionAlert",
        "safety_status": "mpc/SafetyStatus",
        "system_status": "mpc/SystemStatus"
    }
}
```

#### 4.2 系统配置
编辑 `config/system_config.json`:
```json
{
    "boat": {
        "length": 0.75,
        "width": 0.47
    },
    "emergency_threshold_s": 5,
    "warning_threshold_s": 30,
    "max_boats": 30,
    "min_route_gap_m": 10
}
```

### 步骤5: 系统测试

#### 5.1 MQTT连接测试
```bash
# 测试MQTT连接
./scripts/mqtt_quick_check.sh

# 完整MQTT功能测试
./scripts/mqtt_full_test.sh
```

#### 5.2 功能测试
```bash
# 碰撞角度计算测试
./scripts/test_collision_angle.sh

# 运行所有测试
./scripts/run_tests.sh
```

#### 5.3 实时通信演示
```bash
# MQTT实时通信演示
./scripts/mqtt_demo.sh
```

## 生产环境部署

### 安全配置

#### 1. MQTT安全加强
```bash
# 禁用匿名连接
sudo sed -i 's/allow_anonymous true/allow_anonymous false/' /etc/mosquitto/mosquitto.conf

# 启用TLS加密（可选）
sudo tee -a /etc/mosquitto/conf.d/boat_pro.conf << EOF
# TLS配置
cafile /etc/mosquitto/ca_certificates/ca.crt
certfile /etc/mosquitto/certs/server.crt
keyfile /etc/mosquitto/certs/server.key
tls_version tlsv1.2
EOF
```

#### 2. 防火墙配置
```bash
# 开放MQTT端口
sudo ufw allow 2000/tcp

# 限制SSH访问（可选）
sudo ufw limit ssh

# 启用防火墙
sudo ufw enable
```

#### 3. 系统服务配置
```bash
# 创建系统服务文件
sudo tee /etc/systemd/system/boat-pro.service << EOF
[Unit]
Description=Boat Pro Safety Prediction System
After=network.target mosquitto.service
Requires=mosquitto.service

[Service]
Type=simple
User=boat-pro
Group=boat-pro
WorkingDirectory=/opt/boat-pro
ExecStart=/opt/boat-pro/boat_pro
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
EOF

# 创建专用用户
sudo useradd -r -s /bin/false boat-pro

# 部署到系统目录
sudo mkdir -p /opt/boat-pro
sudo cp -r build/* /opt/boat-pro/
sudo cp -r config /opt/boat-pro/
sudo chown -R boat-pro:boat-pro /opt/boat-pro

# 启用服务
sudo systemctl daemon-reload
sudo systemctl enable boat-pro.service
sudo systemctl start boat-pro.service
```

### 监控和日志

#### 1. 日志配置
```bash
# 创建日志目录
sudo mkdir -p /var/log/boat-pro
sudo chown boat-pro:boat-pro /var/log/boat-pro

# 配置logrotate
sudo tee /etc/logrotate.d/boat-pro << EOF
/var/log/boat-pro/*.log {
    daily
    missingok
    rotate 30
    compress
    delaycompress
    notifempty
    create 644 boat-pro boat-pro
}
EOF
```

#### 2. 系统监控
```bash
# 查看服务状态
sudo systemctl status boat-pro.service

# 查看实时日志
sudo journalctl -u boat-pro.service -f

# 查看MQTT服务状态
sudo systemctl status mosquitto.service
```

## 故障排除

### 常见问题

#### 1. 编译错误
```bash
# 缺少依赖库
sudo apt-get install libjsoncpp-dev libmosquitto-dev

# CMake版本过低
sudo apt-get install cmake

# 权限问题
chmod +x scripts/build.sh
```

#### 2. MQTT连接问题
```bash
# 检查MQTT服务状态
sudo systemctl status mosquitto

# 检查端口占用
sudo netstat -tlnp | grep 2000

# 测试MQTT连接
mosquitto_pub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 -t test -m "hello"
```

#### 3. 运行时错误
```bash
# 检查配置文件
cat config/mqtt_config.json
cat config/system_config.json

# 检查权限
ls -la build/boat_pro

# 查看详细错误
./build/boat_pro --verbose
```

### 性能优化

#### 1. 系统调优
```bash
# 增加文件描述符限制
echo "* soft nofile 65536" | sudo tee -a /etc/security/limits.conf
echo "* hard nofile 65536" | sudo tee -a /etc/security/limits.conf

# 优化网络参数
echo "net.core.rmem_max = 16777216" | sudo tee -a /etc/sysctl.conf
echo "net.core.wmem_max = 16777216" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

#### 2. MQTT优化
```bash
# 调整MQTT配置
sudo tee -a /etc/mosquitto/conf.d/boat_pro.conf << EOF
# 性能优化
max_connections 1000
max_inflight_messages 100
max_queued_messages 1000
message_size_limit 1048576
EOF

sudo systemctl restart mosquitto
```

## 维护指南

### 定期维护

#### 1. 系统更新
```bash
# 每月系统更新
sudo apt-get update && sudo apt-get upgrade

# 重启服务
sudo systemctl restart boat-pro.service
```

#### 2. 日志清理
```bash
# 清理旧日志
sudo find /var/log/boat-pro -name "*.log" -mtime +30 -delete

# 清理系统日志
sudo journalctl --vacuum-time=30d
```

#### 3. 性能监控
```bash
# 检查系统资源使用
htop
df -h
free -h

# 检查网络连接
ss -tlnp | grep 2000
```

### 备份和恢复

#### 1. 配置备份
```bash
# 备份配置文件
sudo tar -czf /backup/boat-pro-config-$(date +%Y%m%d).tar.gz \
    /opt/boat-pro/config \
    /etc/mosquitto/conf.d/boat_pro.conf \
    /etc/mosquitto/passwd \
    /etc/mosquitto/acl
```

#### 2. 系统恢复
```bash
# 恢复配置
sudo tar -xzf /backup/boat-pro-config-YYYYMMDD.tar.gz -C /

# 重启服务
sudo systemctl restart mosquitto.service
sudo systemctl restart boat-pro.service
```

## 技术支持

### 联系信息
- **项目文档**: 参见 `docs/` 目录
- **API参考**: `docs/API_REFERENCE.md`
- **系统概述**: `docs/SYSTEM_OVERVIEW.md`

### 调试工具
- **构建脚本**: `./scripts/build.sh`
- **测试脚本**: `./scripts/run_tests.sh`
- **MQTT测试**: `./scripts/mqtt_quick_check.sh`
- **角度测试**: `./scripts/test_collision_angle.sh`
