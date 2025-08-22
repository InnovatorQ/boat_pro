# 部署指南

## 概述

本文档提供了无人船作业安全预测系统的完整部署指南，包括环境准备、系统安装、配置设置和运行维护。

## 系统要求

### 硬件要求

#### 最低配置
- **CPU**: 双核 2.0GHz
- **内存**: 4GB RAM
- **存储**: 10GB 可用空间
- **网络**: 100Mbps 以太网

#### 推荐配置
- **CPU**: 四核 3.0GHz
- **内存**: 8GB RAM
- **存储**: 50GB SSD
- **网络**: 1Gbps 以太网

### 软件要求

#### 操作系统
- **Ubuntu**: 18.04 LTS 或更高版本
- **CentOS**: 7.0 或更高版本
- **Debian**: 9.0 或更高版本

#### 编译环境
- **GCC**: 7.0+ (支持C++17)
- **CMake**: 3.10+
- **Make**: 4.0+

#### 依赖库
- **jsoncpp**: JSON处理库
- **mosquitto**: MQTT客户端库
- **pthread**: 多线程支持

## 环境准备

### Ubuntu/Debian 系统

```bash
# 更新系统包
sudo apt-get update
sudo apt-get upgrade -y

# 安装编译工具
sudo apt-get install -y build-essential cmake git

# 安装依赖库
sudo apt-get install -y libjsoncpp-dev libmosquitto-dev

# 安装MQTT服务器
sudo apt-get install -y mosquitto mosquitto-clients

# 启动MQTT服务
sudo systemctl start mosquitto
sudo systemctl enable mosquitto
```

### CentOS/RHEL 系统

```bash
# 更新系统包
sudo yum update -y

# 安装编译工具
sudo yum groupinstall -y "Development Tools"
sudo yum install -y cmake3 git

# 安装EPEL仓库
sudo yum install -y epel-release

# 安装依赖库
sudo yum install -y jsoncpp-devel libmosquitto-devel

# 安装MQTT服务器
sudo yum install -y mosquitto mosquitto-clients

# 启动MQTT服务
sudo systemctl start mosquitto
sudo systemctl enable mosquitto
```

## 源码获取与编译

### 1. 获取源码

```bash
# 克隆仓库
git clone <repository-url> boat_pro
cd boat_pro

# 检查分支
git branch -a
git checkout main
```

### 2. 编译项目

```bash
# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 编译项目
make -j$(nproc)

# 检查编译结果
ls -la
```

### 3. 验证编译

```bash
# 运行测试程序
./simple_mqtt_test

# 检查依赖库
ldd boat_pro
```

## 配置设置

### 1. 系统配置

创建系统配置文件 `config/system_config.json`:

```json
{
    "boat": {
        "length": 0.75,
        "width": 0.47
    },
    "emergency_threshold_s": 5,
    "warning_threshold_s": 30,
    "max_boats": 30,
    "min_route_gap_m": 10,
    "collision_detection": {
        "update_interval_ms": 100,
        "prediction_horizon_s": 60
    },
    "logging": {
        "level": "INFO",
        "file": "/var/log/boat_pro/system.log"
    }
}
```

### 2. MQTT配置

配置MQTT连接 `config/mqtt_config.json`:

```json
{
    "broker": {
        "host": "127.0.0.1",
        "port": 1883,
        "username": "",
        "password": "",
        "keep_alive": 60,
        "clean_session": true
    },
    "mpc_client": {
        "client_id": "MPC_CLIENT_001",
        "publish_topics": {
            "boat_state": "mpc/boat_state/",
            "collision_alert": "mpc/collision_alert/",
            "safety_status": "mpc/safety_status/",
            "system_status": "mpc/system_status",
            "heartbeat": "mpc/heartbeat/"
        },
        "subscribe_topics": {
            "mission_config": "gcs/mission_config",
            "route_plan": "gcs/route_plan/+",
            "safety_params": "gcs/safety_params",
            "emergency_override": "gcs/emergency_override",
            "system_command": "gcs/system_command"
        }
    },
    "qos_settings": {
        "boat_state": 0,
        "collision_alert": 1,
        "safety_status": 1,
        "system_status": 1,
        "mission_config": 2,
        "emergency_override": 2
    }
}
```

### 3. MQTT服务器配置

配置Mosquitto服务器 `/etc/mosquitto/conf.d/boat_pro.conf`:

```conf
# 监听端口
listener 1883 0.0.0.0

# 允许匿名连接（开发环境）
allow_anonymous true

# 日志配置
log_dest file /var/log/mosquitto/mosquitto.log
log_type error
log_type warning
log_type notice
log_type information

# 持久化配置
persistence true
persistence_location /var/lib/mosquitto/

# 连接限制
max_connections 1000
max_inflight_messages 100
max_queued_messages 1000

# 消息大小限制
message_size_limit 1048576
```

### 4. 生产环境安全配置

生产环境MQTT安全配置:

```conf
# 禁用匿名连接
allow_anonymous false

# 密码文件
password_file /etc/mosquitto/passwd

# ACL访问控制
acl_file /etc/mosquitto/acl

# TLS加密
listener 8883 0.0.0.0
cafile /etc/mosquitto/ca.crt
certfile /etc/mosquitto/server.crt
keyfile /etc/mosquitto/server.key
require_certificate false
```

创建用户和权限:

```bash
# 创建用户
sudo mosquitto_passwd -c /etc/mosquitto/passwd mpc_user
sudo mosquitto_passwd /etc/mosquitto/passwd gcs_user

# 配置ACL权限
sudo tee /etc/mosquitto/acl << EOF
# MPC用户权限
user mpc_user
topic write mpc/#
topic read gcs/#

# GCS用户权限
user gcs_user
topic write gcs/#
topic read mpc/#
EOF
```

## 服务部署

### 1. 创建系统服务

创建systemd服务文件 `/etc/systemd/system/boat-pro.service`:

```ini
[Unit]
Description=Boat Pro Safety Prediction System
After=network.target mosquitto.service
Requires=mosquitto.service

[Service]
Type=simple
User=boat-pro
Group=boat-pro
WorkingDirectory=/opt/boat_pro
ExecStart=/opt/boat_pro/bin/boat_pro
ExecReload=/bin/kill -HUP $MAINPID
Restart=always
RestartSec=5
StandardOutput=journal
StandardError=journal

# 环境变量
Environment=BOAT_PRO_CONFIG=/opt/boat_pro/config
Environment=BOAT_PRO_LOG_LEVEL=INFO

# 资源限制
LimitNOFILE=65536
LimitNPROC=32768

[Install]
WantedBy=multi-user.target
```

### 2. 创建用户和目录

```bash
# 创建系统用户
sudo useradd -r -s /bin/false boat-pro

# 创建目录结构
sudo mkdir -p /opt/boat_pro/{bin,config,logs,data}
sudo mkdir -p /var/log/boat_pro

# 复制文件
sudo cp build/boat_pro /opt/boat_pro/bin/
sudo cp -r config/* /opt/boat_pro/config/
sudo cp -r scripts /opt/boat_pro/

# 设置权限
sudo chown -R boat-pro:boat-pro /opt/boat_pro
sudo chown -R boat-pro:boat-pro /var/log/boat_pro
sudo chmod +x /opt/boat_pro/bin/boat_pro
```

### 3. 启动服务

```bash
# 重新加载systemd配置
sudo systemctl daemon-reload

# 启动服务
sudo systemctl start boat-pro

# 设置开机自启
sudo systemctl enable boat-pro

# 检查服务状态
sudo systemctl status boat-pro
```

## 网络配置

### 1. 防火墙设置

```bash
# Ubuntu/Debian (ufw)
sudo ufw allow 1883/tcp comment "MQTT"
sudo ufw allow 8883/tcp comment "MQTT TLS"

# CentOS/RHEL (firewalld)
sudo firewall-cmd --permanent --add-port=1883/tcp
sudo firewall-cmd --permanent --add-port=8883/tcp
sudo firewall-cmd --reload
```

### 2. 网络优化

配置网络参数 `/etc/sysctl.d/99-boat-pro.conf`:

```conf
# TCP优化
net.core.rmem_max = 16777216
net.core.wmem_max = 16777216
net.ipv4.tcp_rmem = 4096 87380 16777216
net.ipv4.tcp_wmem = 4096 65536 16777216

# 连接数优化
net.core.somaxconn = 65535
net.ipv4.tcp_max_syn_backlog = 65535
net.core.netdev_max_backlog = 5000

# 时间等待优化
net.ipv4.tcp_fin_timeout = 30
net.ipv4.tcp_tw_reuse = 1
```

应用配置:

```bash
sudo sysctl -p /etc/sysctl.d/99-boat-pro.conf
```

## 监控和日志

### 1. 日志配置

配置logrotate `/etc/logrotate.d/boat-pro`:

```conf
/var/log/boat_pro/*.log {
    daily
    missingok
    rotate 30
    compress
    delaycompress
    notifempty
    create 644 boat-pro boat-pro
    postrotate
        systemctl reload boat-pro
    endscript
}

/var/log/mosquitto/*.log {
    daily
    missingok
    rotate 30
    compress
    delaycompress
    notifempty
    create 644 mosquitto mosquitto
    postrotate
        systemctl reload mosquitto
    endscript
}
```

### 2. 监控脚本

创建健康检查脚本 `/opt/boat_pro/scripts/health_check.sh`:

```bash
#!/bin/bash

# 健康检查脚本
LOG_FILE="/var/log/boat_pro/health_check.log"
MQTT_HOST="127.0.0.1"
MQTT_PORT="1883"

echo "$(date): 开始健康检查" >> $LOG_FILE

# 检查服务状态
if ! systemctl is-active --quiet boat-pro; then
    echo "$(date): ERROR - boat-pro服务未运行" >> $LOG_FILE
    exit 1
fi

if ! systemctl is-active --quiet mosquitto; then
    echo "$(date): ERROR - mosquitto服务未运行" >> $LOG_FILE
    exit 1
fi

# 检查MQTT连接
if ! timeout 5 mosquitto_pub -h $MQTT_HOST -p $MQTT_PORT -t "health/check" -m "$(date)" 2>/dev/null; then
    echo "$(date): ERROR - MQTT连接失败" >> $LOG_FILE
    exit 1
fi

# 检查进程资源
CPU_USAGE=$(ps -o %cpu -p $(pgrep boat_pro) | tail -1 | tr -d ' ')
MEM_USAGE=$(ps -o %mem -p $(pgrep boat_pro) | tail -1 | tr -d ' ')

if (( $(echo "$CPU_USAGE > 80" | bc -l) )); then
    echo "$(date): WARNING - CPU使用率过高: $CPU_USAGE%" >> $LOG_FILE
fi

if (( $(echo "$MEM_USAGE > 80" | bc -l) )); then
    echo "$(date): WARNING - 内存使用率过高: $MEM_USAGE%" >> $LOG_FILE
fi

echo "$(date): 健康检查完成 - CPU: $CPU_USAGE%, MEM: $MEM_USAGE%" >> $LOG_FILE
```

设置定时任务:

```bash
# 添加到crontab
sudo crontab -e

# 每5分钟执行一次健康检查
*/5 * * * * /opt/boat_pro/scripts/health_check.sh
```

## 测试验证

### 1. 功能测试

```bash
# 进入项目目录
cd /opt/boat_pro

# 快速功能检查
./scripts/mqtt_quick_check.sh

# 完整功能测试
./scripts/mqtt_full_test.sh

# 实时通信演示
./scripts/mqtt_demo.sh
```

### 2. 性能测试

```bash
# MQTT性能测试
./scripts/mqtt_performance_test.sh

# 系统负载测试
./scripts/system_load_test.sh
```

### 3. 压力测试

```bash
# 并发连接测试
for i in {1..100}; do
    mosquitto_sub -h 127.0.0.1 -p 1883 -t "test/$i" -C 1 &
done

# 消息吞吐量测试
for i in {1..1000}; do
    mosquitto_pub -h 127.0.0.1 -p 1883 -t "test/performance" -m "message_$i"
done
```

## 故障排除

### 1. 常见问题

#### 服务启动失败
```bash
# 检查服务状态
sudo systemctl status boat-pro

# 查看日志
sudo journalctl -u boat-pro -f

# 检查配置文件
sudo -u boat-pro /opt/boat_pro/bin/boat_pro --check-config
```

#### MQTT连接失败
```bash
# 检查MQTT服务
sudo systemctl status mosquitto

# 测试连接
mosquitto_pub -h 127.0.0.1 -p 1883 -t "test" -m "hello"

# 查看MQTT日志
sudo tail -f /var/log/mosquitto/mosquitto.log
```

#### 性能问题
```bash
# 检查系统资源
top -p $(pgrep boat_pro)
iostat -x 1
netstat -i

# 检查网络连接
ss -tuln | grep 1883
```

### 2. 调试工具

```bash
# 实时监控MQTT消息
mosquitto_sub -h 127.0.0.1 -p 1883 -t "#" -v

# 系统性能监控
htop
iotop
nethogs

# 网络连接监控
watch -n 1 'ss -tuln | grep 1883'
```

## 升级和维护

### 1. 系统升级

```bash
# 停止服务
sudo systemctl stop boat-pro

# 备份配置
sudo cp -r /opt/boat_pro/config /opt/boat_pro/config.backup.$(date +%Y%m%d)

# 更新代码
cd /path/to/source
git pull origin main

# 重新编译
cd build
make clean
make -j$(nproc)

# 部署新版本
sudo cp boat_pro /opt/boat_pro/bin/

# 启动服务
sudo systemctl start boat-pro
```

### 2. 配置更新

```bash
# 修改配置文件
sudo nano /opt/boat_pro/config/system_config.json

# 重新加载配置
sudo systemctl reload boat-pro

# 验证配置
sudo systemctl status boat-pro
```

### 3. 数据备份

```bash
# 创建备份脚本
#!/bin/bash
BACKUP_DIR="/backup/boat_pro/$(date +%Y%m%d)"
mkdir -p $BACKUP_DIR

# 备份配置文件
cp -r /opt/boat_pro/config $BACKUP_DIR/

# 备份日志文件
cp -r /var/log/boat_pro $BACKUP_DIR/

# 备份数据文件
cp -r /opt/boat_pro/data $BACKUP_DIR/

# 压缩备份
tar -czf $BACKUP_DIR.tar.gz $BACKUP_DIR
rm -rf $BACKUP_DIR
```

