# MQTT客户端配置文档

## 概述

本文档描述了boat_pro系统与客户端MQTT配置的集成方案，包括连接参数、主题映射和消息格式。

## 客户端配置要求

### 连接参数
- **IP地址**: 127.0.0.1
- **端口**: 2000 (可修改)
- **账号**: vEagles
- **密码**: 123456
- **客户端ID**: 1000

### 认证方式
- 用户名/密码认证
- 清理会话: true
- 保活时间: 60秒
- 连接超时: 30秒

## 主题映射

### 订阅主题 (boat_pro系统订阅，客户端发布)

| 主题名称 | 描述 | 数据类型 |
|---------|------|----------|
| `dock/BoatState` | 无人船动态数据 | BoatState JSON |
| `dock/DockInfo` | 船坞静态数据 | DockInfo JSON |
| `dock/RouteInfo` | 船线定义数据 | RouteInfo JSON |
| `dock/Config` | 系统配置文件 | SystemConfig JSON |

### 发布主题 (boat_pro系统发布，客户端订阅)

| 主题名称 | 描述 | 数据类型 |
|---------|------|----------|
| `dock/CollisionAlert` | 碰撞告警 | CollisionAlert JSON |
| `dock/SafetyStatus` | 安全状态 | SafetyStatus JSON |
| `dock/FleetCommand` | 舰队命令 | FleetCommand JSON |
| `dock/SystemStatus` | 系统状态 | SystemStatus JSON |
| `dock/Heartbeat` | 心跳消息 | Heartbeat JSON |

## 消息格式

### 1. 船只状态 (dock/BoatState)

```json
{
    "sysid": 1,
    "lat": 30.549832,
    "lng": 114.342922,
    "speed": 2.5,
    "heading": 45.0,
    "timestamp": 1692168000,
    "status": "normal"
}
```

### 2. 船坞信息 (dock/DockInfo)

```json
{
    "dock_id": 1,
    "position": {
        "lat": 30.550000,
        "lng": 114.343000
    },
    "capacity": 10,
    "status": "available",
    "occupied_slots": 3
}
```

### 3. 航线信息 (dock/RouteInfo)

```json
{
    "route_id": 1,
    "name": "主航线",
    "direction": "clockwise",
    "waypoints": [
        {"lat": 30.549000, "lng": 114.342000},
        {"lat": 30.550000, "lng": 114.343000}
    ],
    "min_gap_m": 10
}
```

### 4. 系统配置 (dock/Config)

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

### 5. 碰撞告警 (dock/CollisionAlert)

```json
{
    "current_boat_id": 1,
    "level": 2,
    "collision_time": 15.5,
    "collision_position": {
        "lat": 30.549832,
        "lng": 114.342922
    },
    "front_boat_ids": [2],
    "oncoming_boat_ids": [3, 4],
    "decision_advice": "减速避让"
}
```

### 6. 安全状态 (dock/SafetyStatus)

```json
{
    "boat_id": 1,
    "status": "safe",
    "risk_level": 0,
    "last_check": 1692168000,
    "alerts_count": 0
}
```

### 7. 舰队命令 (dock/FleetCommand)

```json
{
    "command": "stop",
    "target_boats": [1, 2, 3],
    "priority": "high",
    "timestamp": 1692168000,
    "reason": "emergency_stop"
}
```

### 8. 系统状态 (dock/SystemStatus)

```json
{
    "status": "running",
    "active_boats": 3,
    "alerts": 1,
    "timestamp": 1692168000,
    "uptime": 3600
}
```

### 9. 心跳消息 (dock/Heartbeat)

```json
{
    "boat_id": 1,
    "timestamp": 1692168000,
    "status": "alive",
    "sequence": 123
}
```

## 配置文件

### MQTT配置文件 (config/mqtt_config.json)

```json
{
    "broker": {
        "host": "127.0.0.1",
        "port": 2000,
        "client_id": "1000",
        "username": "vEagles",
        "password": "123456",
        "clean_session": true,
        "keep_alive": 60,
        "connect_timeout": 30
    },
    "ssl": {
        "enable": false,
        "ca_cert_file": "",
        "cert_file": "",
        "key_file": ""
    },
    "topics": {
        "subscribe": {
            "boat_state": "dock/BoatState",
            "dock_info": "dock/DockInfo", 
            "route_info": "dock/RouteInfo",
            "system_config": "dock/Config"
        },
        "publish": {
            "collision_alert": "dock/CollisionAlert",
            "safety_status": "dock/SafetyStatus",
            "fleet_command": "dock/FleetCommand",
            "system_status": "dock/SystemStatus",
            "heartbeat": "dock/Heartbeat"
        }
    },
    "publishing": {
        "default_qos": 1,
        "retain_messages": false,
        "publish_interval_ms": 1000
    }
}
```

## 使用示例

### 1. 编译和运行

```bash
# 编译项目
cd boat_pro
mkdir -p build && cd build
cmake ..
make mqtt_client_test

# 运行测试程序
./mqtt_client_test
```

### 2. 测试MQTT连接

```bash
# 运行测试脚本
./scripts/test_mqtt_client.sh
```

### 3. 手动测试订阅

```bash
# 订阅所有boat_pro发布的主题
mosquitto_sub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 -i 1001 -t "dock/#" -v
```

### 4. 手动测试发布

```bash
# 发布船只状态数据
mosquitto_pub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 -i 1002 \
  -t "dock/BoatState" \
  -m '{"sysid":1,"lat":30.5,"lng":114.3,"speed":2.5,"heading":45.0}'
```

## 集成步骤

### 1. 客户端集成

1. 配置MQTT客户端连接参数
2. 订阅boat_pro发布的主题
3. 发布船只和系统数据到指定主题
4. 处理接收到的告警和命令消息

### 2. boat_pro系统配置

1. 更新`config/mqtt_config.json`配置文件
2. 重新编译boat_pro系统
3. 启动系统并验证MQTT连接
4. 测试消息收发功能

## 故障排除

### 常见问题

1. **连接被拒绝**
   - 检查MQTT代理是否在指定端口运行
   - 验证用户名和密码是否正确
   - 确认防火墙设置

2. **消息未收到**
   - 检查主题名称是否正确
   - 验证QoS设置
   - 确认订阅是否成功

3. **认证失败**
   - 验证用户名: vEagles
   - 验证密码: 123456
   - 检查MQTT代理的用户配置

### 调试命令

```bash
# 检查MQTT代理状态
netstat -an | grep 2000

# 测试基本连接
mosquitto_pub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 -t "test" -m "hello"

# 监听所有消息
mosquitto_sub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 -t "#" -v
```

## 性能考虑

- **消息频率**: 建议船只状态更新频率不超过1Hz
- **消息大小**: 单个消息建议不超过1KB
- **连接数**: 支持多个客户端同时连接
- **QoS级别**: 建议使用QoS 1确保消息传递

## 安全建议

- 在生产环境中使用强密码
- 考虑启用SSL/TLS加密
- 限制客户端IP访问范围
- 定期更新认证凭据

---

**版本**: v1.0  
**更新时间**: 2024年8月16日  
**状态**: ✅ 已验证
