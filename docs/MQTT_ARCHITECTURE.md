# MQTT通信架构文档

## 架构概述

本文档定义了无人船作业安全预测系统中MPC（Mission Planning Computer）和GCS（Ground Control Station）之间的MQTT通信架构。

## 系统角色定义

### MPC (Mission Planning Computer)
- **职责**: 现场执行单元，负责实时决策和控制
- **功能**: 
  - 实时碰撞检测和预测
  - 自动避障决策执行
  - 船只状态监控
  - 安全告警生成
  - 数据上报到GCS

### GCS (Ground Control Station)  
- **职责**: 地面监控中心，负责监控和高级管理
- **功能**:
  - 监控船队整体状态
  - 任务规划和配置管理
  - 告警阈值设置
  - 紧急情况人工接管
  - 历史数据分析

## MQTT主题架构

### MPC发布主题 (MPC → GCS)

MPC作为现场执行单元，负责向GCS上报各种状态和告警信息。

```
mpc/
├── boat_state/{boat_id}      # 船只实时状态
├── collision_alert/{boat_id} # 碰撞预测告警  
├── safety_status/{boat_id}   # 安全状态评估
├── dock_info/{dock_id}       # 船坞状态信息
├── route_info/{route_id}     # 航线执行状态
├── system_status             # MPC系统状态
├── heartbeat/{boat_id}       # 船只心跳信号
└── emergency_event/{boat_id} # 紧急事件上报
```

### GCS发布主题 (GCS → MPC)

GCS作为监控中心，主要发布配置、规划和紧急干预指令。

```
gcs/
├── mission_config            # 任务配置下发
├── route_plan/{route_id}     # 航线规划下发
├── safety_params             # 安全参数配置
├── emergency_override        # 紧急接管指令
├── system_command            # 系统级命令
└── heartbeat                 # GCS心跳信号
```

## 消息格式定义

### MPC发布消息

#### 船只状态 (mpc/boat_state/{boat_id})
```json
{
  "boat_id": 1,
  "timestamp": 1640995200,
  "position": {
    "lat": 30.549832,
    "lng": 114.342922
  },
  "speed": 2.5,
  "heading": 90.0,
  "battery": 85,
  "status": "ACTIVE"
}
```

#### 碰撞告警 (mpc/collision_alert/{boat_id})
```json
{
  "boat_id": 1,
  "alert_level": 2,
  "collision_time": 15.5,
  "collision_position": {
    "lat": 30.549832,
    "lng": 114.342922
  },
  "involved_boats": [2, 3],
  "avoidance_action": "减速避让",
  "timestamp": 1640995200
}
```

#### 安全状态 (mpc/safety_status/{boat_id})
```json
{
  "boat_id": 1,
  "safety_level": "SAFE",
  "risk_factors": [],
  "operational_status": "NORMAL",
  "last_check": 1640995200
}
```

#### 系统状态 (mpc/system_status)
```json
{
  "system_id": "MPC_001",
  "status": "OPERATIONAL",
  "active_boats": 5,
  "cpu_usage": 45.2,
  "memory_usage": 62.1,
  "uptime": 86400,
  "timestamp": 1640995200
}
```

### GCS发布消息

#### 任务配置 (gcs/mission_config)
```json
{
  "mission_id": "MISSION_001",
  "boat_count": 5,
  "formation_type": "LINE",
  "safety_distance": 10.0,
  "max_speed": 3.0,
  "operation_area": {
    "center": {"lat": 30.55, "lng": 114.34},
    "radius": 500
  },
  "timestamp": 1640995200
}
```

#### 航线规划 (gcs/route_plan/{route_id})
```json
{
  "route_id": "ROUTE_001",
  "waypoints": [
    {"lat": 30.549832, "lng": 114.342922, "speed": 2.0},
    {"lat": 30.550832, "lng": 114.343922, "speed": 2.5}
  ],
  "route_type": "CLOCKWISE",
  "priority": 1,
  "timestamp": 1640995200
}
```

#### 安全参数 (gcs/safety_params)
```json
{
  "emergency_threshold_s": 5,
  "warning_threshold_s": 30,
  "min_safe_distance": 5.0,
  "max_collision_risk": 0.8,
  "auto_avoidance_enabled": true,
  "timestamp": 1640995200
}
```

#### 紧急接管 (gcs/emergency_override)
```json
{
  "override_id": "EMERGENCY_001",
  "boat_ids": [1, 2, 3],
  "action": "STOP_ALL",
  "reason": "人工接管",
  "operator": "OPERATOR_001",
  "timestamp": 1640995200
}
```

## 通信流程

### 正常运行流程
```
1. MPC启动 → 发布 mpc/system_status
2. GCS接收状态 → 发布 gcs/mission_config
3. MPC接收配置 → 开始执行任务
4. MPC持续发布船只状态和告警信息
5. GCS监控并记录所有数据
```

### 碰撞告警流程
```
1. MPC检测到碰撞风险
2. MPC发布 mpc/collision_alert/{boat_id}
3. MPC执行自动避障
4. GCS接收告警并记录
5. 必要时GCS发布 gcs/emergency_override
```

### 紧急接管流程
```
1. GCS操作员判断需要接管
2. GCS发布 gcs/emergency_override
3. MPC接收指令并执行
4. MPC发布执行状态确认
5. GCS确认接管成功
```

## QoS策略

| 消息类型 | QoS等级 | 原因 |
|---------|---------|------|
| 船只状态 | 0 | 高频数据，允许丢失 |
| 碰撞告警 | 1 | 安全关键，确保送达 |
| 安全状态 | 1 | 重要状态，确保送达 |
| 系统配置 | 2 | 重要配置，确保送达且不重复 |
| 紧急指令 | 2 | 关键指令，确保送达且不重复 |
| 心跳信息 | 0 | 高频信号，允许丢失 |

## 连接配置

### MQTT Broker配置
```json
{
  "broker": {
    "host": "127.0.0.1",
    "port": 1883,
    "username": "",
    "password": "",
    "keep_alive": 60,
    "clean_session": true
  }
}
```

### 客户端配置
```json
{
  "mpc_client": {
    "client_id": "MPC_CLIENT_001",
    "reconnect_interval": 5,
    "max_reconnect_attempts": 10
  },
  "gcs_client": {
    "client_id": "GCS_CLIENT_001",
    "reconnect_interval": 5,
    "max_reconnect_attempts": 10
  }
}
```

## 发布频率

| 消息类型 | 频率 | 说明 |
|---------|------|------|
| 船只状态 | 1Hz | 实时位置和状态 |
| 安全状态 | 0.2Hz | 定期安全评估 |
| 系统状态 | 0.1Hz | 系统健康检查 |
| 心跳信息 | 0.1Hz | 连接存活检测 |
| 告警信息 | 事件触发 | 检测到风险时发送 |
| 配置信息 | 按需发送 | 配置变更时发送 |

## 错误处理

### 连接失败处理
- 自动重连机制
- 指数退避策略
- 最大重试次数限制

### 消息丢失处理
- 关键消息使用QoS 1/2
- 心跳超时检测
- 状态同步机制

### 网络异常处理
- 本地缓存机制
- 离线数据存储
- 恢复后数据同步

## 安全考虑

### 生产环境建议
- 启用TLS加密传输
- 配置用户名密码认证
- 设置访问控制列表(ACL)
- 定期更新认证信息

### 监控告警
- 连接状态监控
- 消息传输监控
- 异常行为检测
- 性能指标监控

## 扩展性

### 水平扩展
- 支持多MPC实例
- 支持多GCS实例
- 负载均衡配置

### 功能扩展
- 自定义消息类型
- 插件化架构
- 第三方系统集成

## 测试验证

### 功能测试
```bash
# 快速功能检查
./scripts/mqtt_quick_check.sh

# 完整功能测试
./scripts/mqtt_full_test.sh

# 实时通信演示
./scripts/mqtt_demo.sh
```

### 性能测试
- 消息吞吐量: >700 msg/s
- 延迟: <10ms
- 并发连接: >100

## 故障排除

### 常见问题
1. **连接失败**: 检查MQTT服务状态和网络连接
2. **消息丢失**: 检查QoS设置和网络稳定性
3. **性能问题**: 检查系统资源和网络带宽

### 调试工具
```bash
# 监听所有消息
mosquitto_sub -h 127.0.0.1 -p 1883 -t "#" -v

# 发布测试消息
mosquitto_pub -h 127.0.0.1 -p 1883 -t "test/topic" -m "test message"
```

