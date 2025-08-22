# MQTT主题定义文档

## 概述

根据项目需求，MPC和GCS之间的MQTT通信采用简化的主题架构，不使用前缀路径，直接使用主题名称。

## 主题架构

### MPC (Model Predictive Control)

#### 🔵 MPC 订阅主题 (GCS → MPC)
| 主题名称 | 描述 | QoS | 数据来源 |
|---------|------|-----|---------|
| `BoatState` | 船只状态数据 | 0 | GCS发布 |
| `DockInfo` | 船坞信息数据 | 1 | GCS发布 |
| `RouteInfo` | 航线信息数据 | 1 | GCS发布 |
| `Config` | 系统配置数据 | 2 | GCS发布 |

#### 🔴 MPC 发布主题 (MPC → GCS)
| 主题名称 | 描述 | QoS | 数据接收方 |
|---------|------|-----|-----------|
| `CollisionAlert` | 碰撞告警信息 | 2 | GCS订阅 |
| `SafetyStatus` | 安全状态 | 1 | GCS订阅 |
| `FleetCommand` | 舰队命令 | 2 | GCS订阅 |
| `SystemStatus` | 系统状态 | 1 | GCS订阅 |
| `Heartbeat` | 心跳信息 | 0 | GCS订阅 |

### GCS (Ground Control Station)

#### 🔵 GCS 订阅主题 (MPC → GCS)
| 主题名称 | 描述 | QoS | 数据来源 |
|---------|------|-----|---------|
| `CollisionAlert` | 碰撞告警信息 | 2 | MPC发布 |
| `SafetyStatus` | 安全状态 | 1 | MPC发布 |
| `FleetCommand` | 舰队命令 | 2 | MPC发布 |
| `SystemStatus` | 系统状态 | 1 | MPC发布 |
| `Heartbeat` | 心跳信息 | 0 | MPC发布 |

#### 🔴 GCS 发布主题 (GCS → MPC)
| 主题名称 | 描述 | QoS | 数据接收方 |
|---------|------|-----|-----------|
| `BoatState` | 船只状态数据 | 0 | MPC订阅 |
| `DockInfo` | 船坞信息数据 | 1 | MPC订阅 |
| `RouteInfo` | 航线信息数据 | 1 | MPC订阅 |
| `Config` | 系统配置数据 | 2 | MPC订阅 |

## 重要消息格式

### CollisionAlert 碰撞告警消息

GCS订阅此主题以接收碰撞告警信息，包含以下必需字段：

```json
{
  "alert_level": 3,                    // 碰撞紧急程度 (1=正常, 2=警告, 3=紧急)
  "avoidance_decision": "立即减速并右转避让",  // 相应避碰决策建议
  "alert_boat_id": 1,                  // 告警船只ID
  "collision_position": {              // 预计发生碰撞的位置
    "lat": 30.549832,                  // 纬度
    "lng": 114.342922                  // 经度
  },
  "collision_time": 15.5,              // 预计碰撞时间(秒)
  "oncoming_collision_info": {         // 对向碰撞时的航向信息(可选)
    "boat1_heading": 90.0,             // 船只1实际航向(度,0为正北)
    "boat2_heading": 270.0             // 船只2实际航向(度,0为正北)
  },
  "timestamp": 1692691200              // 时间戳
}
```

#### 字段说明

- **alert_level**: 碰撞紧急程度
  - 1 = 正常状态
  - 2 = 警告状态  
  - 3 = 紧急状态

- **avoidance_decision**: 避碰决策建议文本

- **alert_boat_id**: 发出告警的船只ID

- **collision_position**: 预计碰撞发生的地理位置
  - lat: 纬度 (WGS84)
  - lng: 经度 (WGS84)

- **collision_time**: 预计碰撞时间，单位为秒

- **oncoming_collision_info**: 对向碰撞警告时的特殊信息
  - boat1_heading: 第一艘船的实际航向
  - boat2_heading: 第二艘船的实际航向
  - 航向单位为度，0度表示正北方向
  - 此字段仅在对向碰撞警告时包含

## MQTT连接配置

- **Broker地址**: 127.0.0.1
- **端口**: 2000
- **用户名**: vEagles
- **密码**: 123456
- **客户端ID**: 
  - MPC: MPC_CLIENT_001
  - GCS: GCS_CLIENT_001

## QoS等级说明

| QoS | 描述 | 适用主题 |
|-----|------|---------|
| 0 | 最多一次传递 | BoatState, Heartbeat |
| 1 | 至少一次传递 | DockInfo, RouteInfo, SafetyStatus, SystemStatus |
| 2 | 恰好一次传递 | Config, CollisionAlert, FleetCommand |

## 发布频率建议

| 主题 | 建议频率 | 说明 |
|------|---------|------|
| BoatState | 1.0 Hz | 每秒更新一次船只状态 |
| SafetyStatus | 0.2 Hz | 每5秒更新一次安全状态 |
| SystemStatus | 0.1 Hz | 每10秒更新一次系统状态 |
| Heartbeat | 0.1 Hz | 每10秒发送一次心跳 |
| CollisionAlert | 事件驱动 | 检测到碰撞风险时立即发送 |

## 测试命令

### 监听所有主题
```bash
mosquitto_sub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 \
  -t "CollisionAlert" -t "SafetyStatus" -t "FleetCommand" \
  -t "SystemStatus" -t "Heartbeat" -t "BoatState" \
  -t "DockInfo" -t "RouteInfo" -t "Config" -v
```

### 发布测试消息
```bash
# GCS发布船只状态
mosquitto_pub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 \
  -t "BoatState" \
  -m '{"boat_id":1,"lat":30.55,"lng":114.34,"speed":2.5,"heading":90,"status":"ACTIVE","timestamp":'$(date +%s)'}'

# MPC发布碰撞告警
mosquitto_pub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 \
  -t "CollisionAlert" \
  -m '{"alert_level":3,"avoidance_decision":"立即减速并右转避让","alert_boat_id":1,"collision_position":{"lat":30.549832,"lng":114.342922},"collision_time":15.5,"oncoming_collision_info":{"boat1_heading":90.0,"boat2_heading":270.0},"timestamp":'$(date +%s)'}'
```
