# MQTT主题定义文档
---

## 主题架构

### MPC (Model Predictive Control) - 无人船作业安全预测系统

#### 🔵 MPC 订阅主题 (GCS → MPC)
| 主题名称 | 描述 | QoS | 数据来源 | 用途 |
|---------|------|-----|---------|------|
| `BoatState` | 船只状态数据 | 0 | GCS发布 | 实时船只位置、速度、航向等状态信息 |
| `DockInfo` | 船坞信息数据 | 1 | GCS发布 | 船坞位置信息，用于出入坞碰撞预测 |
| `RouteInfo` | 航线信息数据 | 1 | GCS发布 | 规划航线信息，用于航线碰撞预测 |
| `Config` | 系统配置数据 | 2 | GCS发布 | 船只尺寸、安全阈值等配置参数 |

#### 🔴 MPC 发布主题 (MPC → GCS) - 安全预测输出
| 主题名称 | 描述 | QoS | 数据接收方 | 输出内容 |
|---------|------|-----|-----------|---------|
| `CollisionAlert` | 碰撞告警信息 | 2 | GCS订阅 | 碰撞预测结果、告警等级、避碰建议 |
| `SafetyStatus` | 安全状态 | 1 | GCS订阅 | 系统整体安全状态监控 |
| `SystemStatus` | 系统状态 | 1 | GCS订阅 | 预测系统运行状态 |

### GCS (Ground Control Station) - 地面控制站

#### 🔵 GCS 订阅主题 (MPC → GCS) - 接收安全预测结果
| 主题名称 | 描述 | QoS | 数据来源 | 接收内容 |
|---------|------|-----|---------|---------|
| `CollisionAlert` | 碰撞告警信息 | 2 | MPC发布 | 安全预测系统的碰撞告警 |
| `SafetyStatus` | 安全状态 | 1 | MPC发布 | 整体安全监控状态 |
| `SystemStatus` | 系统状态 | 1 | MPC发布 | 预测系统健康状态 |

#### 🔴 GCS 发布主题 (GCS → MPC) - 提供预测所需数据
| 主题名称 | 描述 | QoS | 数据接收方 | 提供内容 |
|---------|------|-----|-----------|---------|
| `BoatState` | 船只状态数据 | 0 | MPC订阅 | 实时船只动态信息 |
| `DockInfo` | 船坞信息数据 | 1 | MPC订阅 | 船坞静态位置信息 |
| `RouteInfo` | 航线信息数据 | 1 | MPC订阅 | 规划航线路径信息 |
| `Config` | 系统配置数据 | 2 | MPC订阅 | 预测系统配置参数 |

## 重要消息格式

## GCS发布主题数据格式 (GCS → MPC)

### BoatState 无人船动态数据

GCS发布此主题向MPC提供实时船只状态信息，用于碰撞预测分析：

```json
{
  "sysid": 1,                        // 船只ID
  "timestamp": 1722325256.530,       // 接收时间戳（UTC或本机时间，单位：秒）
  "lat": 30.549832,                  // 纬度（WGS84）
  "lng": 114.342922,                 // 经度（WGS84）
  "heading": 90.0,                   // 航向角（0°为正北，顺时针增加）
  "speed": 2.5,                      // 船速，单位：米/秒
  "status": 2                        // 航行状态（1-出坞，2-正常航行，3-入坞）
}
```

#### 字段说明
- **sysid**: 船只唯一标识符
- **timestamp**: 数据采集时间戳，用于数据时效性判断
- **lat/lng**: WGS84坐标系统的地理位置
- **heading**: 船只朝向，用于对向碰撞预测
- **speed**: 当前航行速度，用于碰撞时间计算
- **status**: 航行状态，影响优先级判断
  - 1 = 出坞（最低优先级）
  - 2 = 正常航行（中等优先级）
  - 3 = 入坞（最高优先级）

### DockInfo 船坞静态数据

GCS发布此主题向MPC提供船坞位置信息，用于出入坞碰撞预测：

```json
{
  "dock_id": 1,                      // 船坞ID（与停靠boat sysid相同）
  "lat": 30.549100,                  // 纬度（WGS84）
  "lng": 114.343000                  // 经度（WGS84）
}
```

#### 字段说明
- **dock_id**: 船坞唯一标识符，与对应船只ID保持一致
- **lat/lng**: 船坞的固定地理位置，用于出入坞路径预测

### RouteInfo 航线定义数据

GCS发布此主题向MPC提供船只规划航线信息，用于航线碰撞预测：

```json
{
  "sysid": 1,                        // 船只ID
  "points": [                        // 航线点列表（可包含任意个数的点，max 500）
    {"lat": 30.549500, "lng": 114.342800},
    {"lat": 30.549800, "lng": 114.343300},
    {"lat": 30.550100, "lng": 114.343800}
  ]
}
```

#### 字段说明
- **sysid**: 对应船只的唯一标识符
- **points**: 航线路径点数组
  - 每个点包含lat（纬度）和lng（经度）
  - 最多支持500个航点
  - 用于前后船碰撞预测和航线交叉分析

#### 发布时机
- GCS加载boat的规划航线后发布一次
- 修改航线后重新发布
- 总共支持10条船的航线信息

### Config 系统配置数据

GCS发布此主题向MPC提供系统配置参数，用于碰撞预测算法调优：

```json
{
  "boat": {                          // 船只尺寸信息
    "length": 0.75,                  // 船只长度（单位：米）
    "width": 0.47                    // 船只宽度（单位：米）
  },
  "emergency_threshold_s": 5,        // 紧急判断时间阈值（秒）
  "warning_threshold_s": 30,         // 警告判断时间阈值（秒）
  "max_boats": 30,                   // 最大船只数量
  "min_route_gap_m": 10              // 最小航线横向间距（米）
}
```

#### 字段说明
- **boat**: 船只物理尺寸参数
  - **length**: 船只长度，用于碰撞距离计算
  - **width**: 船只宽度，用于安全间距计算
- **emergency_threshold_s**: 紧急告警时间阈值（碰撞距离 < 速度×5秒）
- **warning_threshold_s**: 警告告警时间阈值（碰撞距离 < 速度×30秒）
- **max_boats**: 系统支持的最大船只数量限制
- **min_route_gap_m**: 航线间最小安全距离

#### 配置用途
- 船只尺寸用于精确的碰撞边界计算
- 时间阈值用于告警等级判断
- 数量和距离限制用于系统容量规划

## 数据结构对应关系

### C# 数据结构映射

本文档中的JSON格式与以下C#数据结构完全对应：

#### BoatState 对应关系
```csharp
[DataContract]
public class mqtt_mpc_BoatState
{
    [DataMember(Name = "sysid")]
    public int SysId { get; set; }
    
    [DataMember(Name = "timestamp")]
    public long Timestamp { get; set; }
    
    [DataMember(Name = "lat")]
    public double Latitude { get; set; }
    
    [DataMember(Name = "lng")]
    public double Longitude { get; set; }
    
    [DataMember(Name = "heading")]
    public double Heading { get; set; }
    
    [DataMember(Name = "speed")]
    public double Speed { get; set; }
    
    [DataMember(Name = "status")]
    public int NavigationStatus { get; set; }
}
```

#### DockInfo 对应关系
```csharp
[DataContract]
public class mqtt_mpc_DockInfo
{
    [DataMember(Name = "dock_id")]
    public int DockId { get; set; }
    
    [DataMember(Name = "lat")]
    public double Latitude { get; set; }
    
    [DataMember(Name = "lng")]
    public double Longitude { get; set; }
}
```

#### RouteInfo 对应关系
```csharp
[DataContract]
public class mqtt_mpc_RouteInfo
{
    [DataMember(Name = "sysid")]
    public int SysId { get; set; }
    
    [DataMember(Name = "points")]
    public List<Position> Points { get; set; } = new List<Position>();
}

[DataContract]
public class Position
{
    [DataMember(Name = "lat")]
    public double Latitude { get; set; }
    
    [DataMember(Name = "lng")]
    public double Longitude { get; set; }
}
```

#### Config 对应关系
```csharp
[DataContract]
public class mqtt_mpc_SystemConfiguration
{
    [DataMember(Name = "boat")]
    public BoatDimensions Boat { get; set; } = new BoatDimensions();
    
    [DataMember(Name = "emergency_threshold_s")]
    public int EmergencyThresholdSeconds { get; set; }
    
    [DataMember(Name = "warning_threshold_s")]
    public int WarningThresholdSeconds { get; set; }
    
    [DataMember(Name = "max_boats")]
    public int MaxBoats { get; set; }
    
    [DataMember(Name = "min_route_gap_m")]
    public int MinRouteGapMeters { get; set; }
}

[DataContract]
public class BoatDimensions
{
    [DataMember(Name = "length")]
    public double Length { get; set; }
    
    [DataMember(Name = "width")]
    public double Width { get; set; }
}
```
---

## MPC发布主题数据格式 (MPC → GCS)

### CollisionAlert 碰撞告警消息 (核心输出)

根据需求文档，MPC发布此主题向GCS输出碰撞预测结果，包含以下必需字段：

```json
{
  "alert_level": 3,                    // 碰撞紧急程度 (1=正常, 2=警告, 3=紧急)
  "avoidance_decision": 80,            // 避碰决策建议代码 (整数枚举)
  "current_boat_id": 1,                // 当前告警船只ID
  "collision_angle": 90.0,             // 碰撞角度 (度，0-180°范围)
  "collision_type": "crossing",        // 碰撞类型 (overtaking/oblique/crossing/oblique_head_on/head_on)
  "front_collision_boat_id": 2,        // 前向被碰撞船只ID (仅距离最近的一个)
  "oncoming_collision_boat_ids": [3, 4], // 对向被碰撞船只ID列表 (所有对向船只)
  "collision_position": {              // 预计发生碰撞的位置 (WGS84经纬度)
    "lat": 30.549832,                  // 纬度
    "lng": 114.342922                  // 经度
  },
  "collision_time": 15.5,              // 预计发生碰撞的时间 (秒)
  "oncoming_collision_info": {         // 对向碰撞时两船实际航向 (可选)
    "current_boat_heading": 90.0,      // 当前船只实际航向 (度，0为正北)
    "oncoming_boats_heading": [270.0, 275.0] // 对向船只实际航向列表
  },
  "timestamp": 1692691200              // 时间戳
}
```

#### 字段说明 (严格按照需求文档)

**1. 碰撞告警紧急程度 (3级)**
- **alert_level**: 
  - 1 = 正常：碰撞距离 > 速度×30秒
  - 2 = 警告：碰撞距离 < 速度×30秒，预留30秒反应时间
  - 3 = 紧急：碰撞距离 < 速度×5秒，预留5秒停船时间

**2. 避碰决策建议**
- **avoidance_decision**: 整数枚举代码，表示相应的避碰决策建议

**3. 碰撞告警船ID**
- **current_boat_id**: 当前船ID
- **front_collision_boat_id**: 前向被碰撞船ID (仅上报距离最近船ID)
- **oncoming_collision_boat_ids**: 对向被碰撞船ID (需要上报所有船ID)

**4. 预计发生碰撞的位置**
- **collision_position**: 经纬度 (WGS84坐标系统)

**5. 预计发生碰撞的时间**
- **collision_time**: 秒为单位

**6. 对向碰撞告警时两船的实际航向**
- **oncoming_collision_info**: 仅在对向碰撞时包含
- **current_boat_heading**: 当前船实际航向 (度，0为正北)
- **oncoming_boats_heading**: 对向船只实际航向列表
- 航向单位为度，0度表示正北方向
- 此字段仅在对向碰撞警告时包含

**7. 碰撞角度和类型**
- **collision_angle**: 两船航向线之间的夹角（度）
  - 取值范围：0-180°
  - 计算方法：|boat1_heading - boat2_heading|，如果结果>180°则取360°-结果
  - 用于确定碰撞类型和选择避让策略
- **collision_type**: 基于碰撞角度的碰撞类型分类
  - "overtaking" = 追尾碰撞 (0-30°)
  - "oblique" = 斜向碰撞 (30-60°)
  - "crossing" = 交叉碰撞 (60-120°)
  - "oblique_head_on" = 斜向对撞 (120-150°)
  - "head_on" = 正面对撞 (150-180°)

## 碰撞角度计算规则

### 角度定义和计算方法

**碰撞角度定义**：两船航向线之间的夹角，用于判断碰撞类型和选择避让策略。

**计算公式**：
```javascript
function calculateCollisionAngle(heading1, heading2) {
    let angle = Math.abs(heading1 - heading2);
    if (angle > 180) {
        angle = 360 - angle;  // 取较小的角度
    }
    return angle;
}
```

**示例计算**：
- 船A航向90°，船B航向270° → 碰撞角度 = |90-270| = 180° → 正面对撞
- 船A航向45°，船B航向135° → 碰撞角度 = |45-135| = 90° → 交叉碰撞
- 船A航向0°，船B航向15° → 碰撞角度 = |0-15| = 15° → 追尾碰撞

### 碰撞类型分类表

| 角度范围 | 碰撞类型 | collision_type | 航海术语 | 典型场景 |
|---------|---------|---------------|---------|---------|
| 0-30° | 追尾碰撞 | "overtaking" | Overtaking | 后船追前船，几乎同向 |
| 30-60° | 斜向碰撞 | "oblique" | Oblique collision | 两船斜向相遇 |
| 60-120° | 交叉碰撞 | "crossing" | Crossing collision | 两船垂直或近垂直交叉 |
| 120-150° | 斜向对撞 | "oblique_head_on" | Oblique head-on | 两船大角度相向而行 |
| 150-180° | 正面对撞 | "head_on" | Head-on collision | 两船正面相向 |

### 基于碰撞角度的决策矩阵

| 碰撞角度 | 碰撞类型 | 标准决策代码 | 紧急决策代码 | 决策说明 |
|---------|---------|-------------|-------------|---------|
| 0-30° | 追尾碰撞 | 23 (减速并右转) | 41 (紧急右转) | 后船避让前船 |
| 30-60° | 斜向碰撞 | 21 (向右转向) | 41 (紧急右转) | 根据优先级决定 |
| 60-120° | 交叉碰撞 | 82/83 (让路/保持) | 40 (紧急停船) | 严格按优先级规则 |
| 120-150° | 斜向对撞 | 23 (减速并右转) | 41 (紧急右转) | 双方协调避让 |
| 150-180° | 正面对撞 | 80 (正面相遇右转) | 80 (正面相遇右转) | 双方向右转向 |
## 双方协调避让的含义

### 1. "双方协调避让"的具体含义

#### **协调原则**
• **同步性**: 两船需要同时开始避让动作
• **互补性**: 避让方向要互相配合，不能冲突
• **通信性**: 需要通过信号或系统协调避让策略

### 协调避让的实施步骤

#### **第一阶段: 检测和通知** (碰撞时间 > 30秒)
1. 系统检测到120-150°斜向对撞
2. 向两船同时发送协调避让指令
3. 指定各自的避让方向和速度调整

#### **第二阶段: 执行协调** (碰撞时间 5-30秒)
1. 两船按照指定方向开始转向
2. 同时适当减速以增加反应时间
3. 系统持续监控避让效果

#### **第三阶段: 紧急处理** (碰撞时间 < 5秒)
1. 如果协调失败，启用紧急协议
2. 默认规则: 双方都向右转(类似正面对撞)
3. 必要时紧急停船



### 实际场景示例

#### **场景1: 两船正常航行的斜向对撞**
• 船A: 航向45°, 速度2.5m/s
• 船B: 航向180°, 速度3.0m/s  
• 碰撞角度: 135°
• **协调方案**: A右转+减速, B左转+减速

#### **场景2: 出坞船与正常航行船的斜向对撞**
• 出坞船: 航向30°, 速度2.0m/s
• 正常船: 航向150°, 速度2.8m/s
• 碰撞角度: 120°
• **处理方案**: 优先级规则覆盖，出坞船单方避让

### 系统实现要点

#### **检测逻辑**
```cpp
bool requiresCoordination(double angle, BoatStatus status1, BoatStatus status2) {
    // 角度在120-150°范围
    bool isObliqueHeadOn = (angle >= 120.0 && angle <= 150.0);
    
    // 两船优先级相同时需要协调
    bool samePriority = (status1 == status2);
    
    return isObliqueHeadOn && samePriority;
}
```

#### **决策生成**
```cpp
AvoidanceDecision generateCoordinatedDecision(int boatId, double heading) {
    // 基于船只ID的奇偶性分配避让方向
    if (boatId % 2 == 0) {
        return AvoidanceDecision::REDUCE_SPEED_AND_TURN_RIGHT;  // 23
    } else {
```
### 角度计算的数据来源

**前向碰撞角度计算**：
- 当前船航向：通过BoatState消息获取
- 前向船航向：通过BoatState消息获取对应船只的heading

**对向碰撞角度计算**：
- 当前船航向：`oncoming_collision_info.current_boat_heading`
- 对向船航向：`oncoming_collision_info.oncoming_boats_heading[i]`

**多船碰撞处理**：
- 对于多个对向船只，分别计算与每艘船的碰撞角度
- collision_angle字段记录最危险（时间最短）的碰撞角度
- collision_type基于最危险的碰撞确定

## 避碰决策枚举定义

### 设计原则

本枚举系统专注于**两船避让的核心场景**.

### 决策代码分类 (专注于两船避让场景)

####  正常状态 (1-9)
| 代码 | 枚举名称 | 中文描述 | 英文描述 | 使用场景 |
|------|---------|---------|---------|---------|
| 1 | CONTINUE_NORMAL | 保持当前航向和速度，继续正常航行 | Continue normal navigation | 无碰撞风险，正常航行 |
| 2 | MONITOR_OTHER_VESSEL | 监控对方船只，保持警戒 | Monitor other vessel, maintain vigilance | 发现其他船只，保持观察 |

####  预防性避让 (10-19)
| 代码 | 枚举名称 | 中文描述 | 英文描述 | 使用场景 |
|------|---------|---------|---------|---------|
| 10 | REDUCE_SPEED_SLIGHTLY | 轻微减速，保持安全距离 | Reduce speed slightly, maintain safe distance | 发现潜在风险 |
| 11 | PREPARE_AVOIDANCE | 准备避让动作 | Prepare for avoidance maneuver | 碰撞风险增加 |
| 12 | INCREASE_DISTANCE | 增加与对方船只的距离 | Increase distance from other vessel | 距离过近时 |

####  主动避让 (20-39)
| 代码 | 枚举名称 | 中文描述 | 英文描述 | 使用场景 |
|------|---------|---------|---------|---------|
| 20 | REDUCE_SPEED | 减速避让 | Reduce speed for avoidance | 前方有船只阻挡 |
| 21 | TURN_RIGHT | 向右转向 | Turn right | 标准右转避让 |
| 22 | TURN_LEFT | 向左转向 | Turn left | 标准左转避让 |
| 23 | REDUCE_SPEED_AND_TURN_RIGHT | 减速并右转 | Reduce speed and turn right | 标准避让动作 |
| 24 | REDUCE_SPEED_AND_TURN_LEFT | 减速并左转 | Reduce speed and turn left | 标准避让动作 |
| 25 | STOP_AND_WAIT | 停船等待对方通过 | Stop and wait for other vessel to pass | 让行等待 |

####  紧急避让 (40-59)
| 代码 | 枚举名称 | 中文描述 | 英文描述 | 使用场景 |
|------|---------|---------|---------|---------|
| 40 | EMERGENCY_STOP | 紧急停船 | Emergency stop | 即将发生碰撞 |
| 41 | EMERGENCY_TURN_RIGHT | 紧急右转 | Emergency turn right | 紧急右转避让 |
| 42 | EMERGENCY_TURN_LEFT | 紧急左转 | Emergency turn left | 紧急左转避让 |
| 43 | EMERGENCY_REVERSE | 紧急倒车 | Emergency reverse | 无法前进避让 |
| 44 | EMERGENCY_FULL_STOP | 立即全停 | Immediate full stop | 极度危险情况 |

####  优先级避让 (60-79)
| 代码 | 枚举名称 | 中文描述 | 英文描述 | 使用场景 |
|------|---------|---------|---------|---------|
| 60 | YIELD_TO_PRIORITY_VESSEL | 给优先船只让行 | Yield to priority vessel | 遇到高优先级船只 |
| 61 | MAINTAIN_PRIORITY_RIGHT | 保持优先通行权 | Maintain priority right of way | 本船具有优先权 |
| 62 | UNDOCKING_MUST_YIELD | 出坞船只必须避让 | Undocking vessel must yield | 出坞时遇到其他船 |
| 63 | DOCKING_HAS_PRIORITY | 入坞船只具有优先权 | Docking vessel has priority | 入坞船只保持航行 |
| 64 | NORMAL_SAILING_PRIORITY | 正常航行船只优先权 | Normal sailing vessel priority | 正常航行船只 |

####  特定角度避让 (80-99)
| 代码 | 枚举名称 | 中文描述 | 英文描述 | 使用场景 |
|------|---------|---------|---------|---------|
| 80 | HEAD_ON_TURN_RIGHT | 正面相遇向右转 | Head-on encounter turn right | 正面对撞避让 |
| 81 | OVERTAKING_KEEP_CLEAR | 追越时保持清晰航道 | Overtaking keep clear | 追越场景 |
| 82 | CROSSING_GIVE_WAY | 交叉相遇时让路 | Crossing give way | 交叉相遇让行 |
| 83 | CROSSING_STAND_ON | 交叉相遇时保持航向 | Crossing stand on | 交叉相遇保持 |

####  未知情况 (99)
| 代码 | 枚举名称 | 中文描述 | 英文描述 | 使用场景 |
|------|---------|---------|---------|---------|
| 99 | UNKNOWN_SITUATION | 未知情况，需要人工判断 | Unknown situation, manual intervention required | 系统无法识别的情况 |


### 两船避让决策

#### 基于船只优先级的决策
| 当前船只状态 | 对方船只状态 | 推荐决策代码 | 决策含义 | 适用场景 |
|-------------|-------------|-------------|---------|---------|
| 出坞 (UNDOCKING) | 入坞 (DOCKING) | 62 | 出坞船只必须避让 | 出坞船遇到入坞船 |
| 出坞 (UNDOCKING) | 正常航行 (NORMAL_SAILING) | 62 | 出坞船只必须避让 | 出坞船遇到正常航行船 |
| 正常航行 (NORMAL_SAILING) | 入坞 (DOCKING) | 60 | 给优先船只让行 | 正常航行船遇到入坞船 |
| 正常航行 (NORMAL_SAILING) | 出坞 (UNDOCKING) | 64 | 正常航行船只优先权 | 正常航行船遇到出坞船 |
| 入坞 (DOCKING) | 任何状态 | 63 | 入坞船只具有优先权 | 入坞船具有最高优先权 |

#### 基于碰撞角度的决策
| 碰撞角度范围 | 碰撞类型 | 标准决策代码 | 紧急决策代码 | 决策说明 |
|-------------|---------|-------------|-------------|---------|
| 0-30° | 追尾碰撞 | 23 (减速并右转) | 41 (紧急右转) | 后船避让前船 |
| 30-60° | 斜向碰撞 | 21 (向右转向) | 41 (紧急右转) | 根据优先级决定 |
| 60-120° | 交叉碰撞 | 82/83 (让路/保持) | 40 (紧急停船) | 严格按优先级规则 |
| 120-150° | 斜向对撞 | 23 (减速并右转) | 41 (紧急右转) | 双方协调避让 |
| 150-180° | 正面对撞 | 80 (正面相遇右转) | 80 (正面相遇右转) | 双方向右转向 |

#### 基于告警等级的决策
| 告警等级 | 碰撞时间 | 决策代码范围 | 典型决策 | 响应要求 |
|---------|---------|-------------|---------|---------|
| 1 (正常) | >30秒 | 1-12 | 1 (继续正常航行) | 保持观察 |
| 2 (警告) | 5-30秒 | 20-25, 60-64, 82-83 | 23 (减速并右转) | 主动避让 |
| 3 (紧急) | <5秒 | 40-44, 80-81 | 40 (紧急停船) | 立即响应 |

#### 字段说明

- **alert_level**: 碰撞紧急程度
  - 1 = 正常状态
  - 2 = 警告状态  
  - 3 = 紧急状态

- **avoidance_decision**: 避碰决策代码 (整数枚举)
  - 使用预定义的整数代码表示具体的避碰决策
  - 代码范围: 1-99，专注于两船避让场景
  - 详细代码定义参见上方"避碰决策枚举定义"表格
  - 示例: 80 = 正面相遇向右转, 23 = 减速并右转, 62 = 出坞船只必须避让

- **current_boat_id**: 当前发出告警的船只ID

- **front_collision_boat_id**: 前向被碰撞船只ID
  - 当存在多个前向碰撞船只时，仅上报距离最近的船只ID
  - 如果没有前向碰撞风险，此字段为null或不包含

- **oncoming_collision_boat_ids**: 对向被碰撞船只ID列表
  - 当存在多个对向碰撞船只时，需要上报所有船只ID
  - 如果没有对向碰撞风险，此字段为空数组[]

- **collision_position**: 预计碰撞发生的地理位置
  - lat: 纬度 (WGS84)
  - lng: 经度 (WGS84)

- **collision_time**: 预计碰撞时间，单位为秒

- **oncoming_collision_info**: 对向碰撞警告时的特殊信息(可选)
  - current_boat_heading: 当前船只的实际航向
  - oncoming_boats_heading: 对向船只实际航向列表，与oncoming_collision_boat_ids对应
  - 航向单位为度，0度表示正北方向
  - 此字段仅在对向碰撞警告时包含

## 决策代码使用指南

### 代码选择原则

1. **按紧急程度选择范围**:
   - 正常情况: 1-199
   - 需要避让: 200-299  
   - 紧急情况: 300-399
   - 特殊场景: 400-999

2. **按船只优先级选择**:
   - 入坞船只: 优先使用 403 (保持入坞优先权)
   - 正常航行: 优先使用 404 (保持航行优先权)  
   - 出坞船只: 优先使用 405 (出坞船只避让)

3. **按碰撞角度选择**:
   - 正面对撞 (150-180°): 302/303 (紧急左右转)
   - 交叉相遇 (60-120°): 根据优先级选择
   - 追尾情况 (0-30°): 201/204 (减速避让)

### 常用决策代码组合

| 场景类型 | 告警等级 | 推荐决策代码 | 代码含义 |
|---------|---------|-------------|---------|
| **正面对撞** | 3 | 302 | 紧急右转 |
| **出坞冲突** | 3 | 405 | 出坞船只避让 |
| **入坞优先** | 2 | 403 | 保持入坞优先权 |
| **追尾风险** | 2 | 204 | 减速并右转 |
| **正常航行** | 1 | 1 | 继续正常航行 |

#### 碰撞类型示例

**仅前向碰撞:**
```json
{
  "alert_level": 2,
  "avoidance_decision": 23,                     // 减速并右转
  "current_boat_id": 1,
  "collision_angle": 15.0,                      // 追尾碰撞角度
  "collision_type": "overtaking",               // 追尾碰撞类型
  "front_collision_boat_id": 2,
  "oncoming_collision_boat_ids": [],
  "collision_time": 20.0,
  "collision_position": {"lat": 30.549832, "lng": 114.342922},
  "timestamp": 1692691200
}
```

**仅对向碰撞:**
```json
{
  "alert_level": 3,
  "avoidance_decision": 80,                     // 正面相遇右转
  "current_boat_id": 1,
  "collision_angle": 175.0,                     // 正面对撞角度
  "collision_type": "head_on",                  // 正面对撞类型
  "front_collision_boat_id": null,
  "oncoming_collision_boat_ids": [3, 4],
  "collision_time": 10.5,
  "collision_position": {"lat": 30.549832, "lng": 114.342922},
  "oncoming_collision_info": {
    "current_boat_heading": 90.0,
    "oncoming_boats_heading": [270.0, 275.0]
  },
  "timestamp": 1692691200
}
```

**前向和对向同时碰撞:**
```json
{
  "alert_level": 3,
  "avoidance_decision": 40,                     // 紧急停船
  "current_boat_id": 1,
  "collision_angle": 90.0,                      // 交叉碰撞角度
  "collision_type": "crossing",                 // 交叉碰撞类型
  "front_collision_boat_id": 2,
  "oncoming_collision_boat_ids": [3, 4],
  "collision_time": 8.0,
  "collision_position": {"lat": 30.549832, "lng": 114.342922},
  "oncoming_collision_info": {
    "current_boat_heading": 90.0,
    "oncoming_boats_heading": [270.0, 275.0]
  },
  "timestamp": 1692691200
}
```

**出坞冲突 (90度交叉):**
```json
{
  "alert_level": 3,
  "avoidance_decision": 62,                     // 出坞船只必须避让
  "current_boat_id": 1,
  "collision_angle": 90.0,                      // 交叉碰撞角度
  "collision_type": "crossing",                 // 交叉碰撞类型
  "front_collision_boat_id": null,
  "oncoming_collision_boat_ids": [2],
  "collision_time": 12.0,
  "collision_position": {"lat": 30.549832, "lng": 114.342922},
  "oncoming_collision_info": {
    "current_boat_heading": 45.0,               // 出坞船航向
    "oncoming_boats_heading": [135.0]           // 正常航行船航向
  },
  "timestamp": 1692691200
}
```

**入坞优先通行:**
```json
{
  "alert_level": 2,
  "avoidance_decision": 403,                    // 保持入坞优先权
  "current_boat_id": 1,
  "front_collision_boat_id": 3,
  "oncoming_collision_boat_ids": [],
  "collision_time": 25.0,
  "collision_position": {"lat": 30.549832, "lng": 114.342922},
  "boat_priorities": {
    "current_boat": 1,                          // 入坞船只 (最高优先级)
    "other_boat": 2                             // 正常航行 (中等优先级)
  },
  "timestamp": 1692691200
}
```

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
| 0 | 最多一次传递 | BoatState |
| 1 | 至少一次传递 | DockInfo, RouteInfo, SafetyStatus, SystemStatus |
| 2 | 恰好一次传递 | Config, CollisionAlert |

## 发布频率建议

| 主题 | 建议频率 | 说明 |
|------|---------|------|
| BoatState | 1.0 Hz | 每秒更新一次船只状态 |
| SafetyStatus | 1.0 Hz | 每1秒更新一次安全状态 |
| SystemStatus | 1.0 Hz | 每1秒更新一次系统状态 |
| CollisionAlert | 事件驱动 | 检测到碰撞风险时立即发送 |

### SafetyStatus 安全状态消息

MPC发布此主题向GCS报告系统安全状态，包含以下字段：

```json
{
  "timestamp": 1692691200,           // 时间戳
  "status": "monitoring",            // 系统状态 ("monitoring", "idle", "error")
  "active_boats": 5,                 // 当前活跃船只数量
  "alert_level": "normal",           // 整体告警等级 ("normal", "warning", "emergency")
  "last_collision_check": 1692691200, // 最后一次碰撞检测时间戳
  "total_alerts": 2,                 // 当前总告警数量(可选)
  "system_health": "healthy"         // 系统健康状态(可选)
}
```

#### 字段说明

- **timestamp**: 消息生成时间戳
- **status**: 系统运行状态
  - "monitoring" = 正常监控中
  - "idle" = 空闲状态
  - "error" = 系统错误
- **active_boats**: 当前系统监控的活跃船只数量
- **alert_level**: 系统整体告警等级
  - "normal" = 正常状态
  - "warning" = 警告状态
  - "emergency" = 紧急状态
- **last_collision_check**: 最后一次执行碰撞检测的时间戳
- **total_alerts**: 当前系统中的告警总数(可选字段)
- **system_health**: 系统健康状态描述(可选字段)

### SystemStatus 系统状态消息

MPC发布此主题向GCS报告系统运行状态，包含以下字段：

```json
{
  "timestamp": 1692691200,           // 时间戳
  "status": "running",               // 系统运行状态
  "version": "1.0.0",                // 系统版本
  "uptime": 86400,                   // 系统运行时间(秒)
  "memory_usage": "normal",          // 内存使用状态
  "cpu_usage": "normal",             // CPU使用状态
  "mqtt_connected": true,            // MQTT连接状态
  "messages_published": 1250,        // 已发布消息数量
  "messages_received": 890,          // 已接收消息数量
  "connection_errors": 2,            // 连接错误次数
  "last_error": "",                  // 最后一次错误信息(可选)
  "modules": {                       // 各模块状态(可选)
    "collision_detector": "active",
    "fleet_manager": "active",
    "mqtt_communicator": "active"
  }
}
```

#### 字段说明

- **timestamp**: 状态报告生成时间戳
- **status**: 系统整体运行状态
  - "running" = 正常运行
  - "starting" = 启动中
  - "stopping" = 停止中
  - "error" = 错误状态
- **version**: 系统软件版本号
- **uptime**: 系统连续运行时间，单位为秒
- **memory_usage**: 内存使用状态描述
  - "low" = 低使用率
  - "normal" = 正常使用率
  - "high" = 高使用率
  - "critical" = 临界使用率
- **cpu_usage**: CPU使用状态描述(同memory_usage)
- **mqtt_connected**: MQTT连接状态布尔值
- **messages_published**: 累计发布的MQTT消息数量
- **messages_received**: 累计接收的MQTT消息数量
- **connection_errors**: 累计连接错误次数
- **last_error**: 最后一次发生的错误信息(可选)
- **modules**: 各个系统模块的运行状态(可选)



### Q1: 数据结构有2种信息（前向碰撞和对向碰撞），怎么区分是哪一种？

**A1**: 通过检查CollisionAlert消息中的以下字段来区分碰撞类型：

| 碰撞类型 | front_collision_boat_id | oncoming_collision_boat_ids | 示例 |
|---------|------------------------|----------------------------|------|
| **仅前向碰撞** | 有值(船只ID) | 空数组 [] | `{"front_collision_boat_id": 2, "oncoming_collision_boat_ids": []}` |
| **仅对向碰撞** | null 或不存在 | 有值(船只ID列表) | `{"front_collision_boat_id": null, "oncoming_collision_boat_ids": [3, 4]}` |
| **复合碰撞** | 有值(船只ID) | 有值(船只ID列表) | `{"front_collision_boat_id": 2, "oncoming_collision_boat_ids": [3, 4]}` |
| **无碰撞风险** | null 或不存在 | 空数组 [] | `{"front_collision_boat_id": null, "oncoming_collision_boat_ids": []}` |

```

### Q2: 从以上数据怎么判断从告警状态恢复到安全状态？是否需要新增一个主题？

**A2**: **不需要新增主题**。可以通过现有主题判断安全状态恢复：

#### 方法1: 通过CollisionAlert主题判断
- **告警状态**: `alert_level` = 2 (警告) 或 3 (紧急)
- **安全状态**: `alert_level` = 1 (正常)
- **状态转换**: 当同一船只的alert_level从2/3变为1时，表示该船只恢复安全

#### 方法2: 通过SafetyStatus主题判断
- **告警状态**: `alert_level` = "warning" 或 "emergency"
- **安全状态**: `alert_level` = "normal"
- **无告警**: `total_alerts` = 0

#### 方法3: 消息频率判断
- **告警期间**: CollisionAlert消息持续发布
- **安全恢复**: CollisionAlert消息停止发布，或只发布alert_level=1的消息

**状态恢复判断示例**：
```json
// 告警状态
{"alert_level": 3, "current_boat_id": 1, "collision_time": 5.0}

// 恢复安全状态
{"alert_level": 1, "current_boat_id": 1, "collision_time": -1}
```

### Q3: 被碰撞船只的ID没有？

**A3**: **有提供被碰撞船只ID**，通过以下字段：

| 字段名称 | 类型 | 描述 | 示例 |
|---------|------|------|------|
| `front_collision_boat_id` | Integer/null | 前向被碰撞船只ID（单个，最近的） | `2` |
| `oncoming_collision_boat_ids` | Array | 对向被碰撞船只ID列表（所有对向船只） | `[3, 4, 5]` |

**设计原理**：
- **前向碰撞**：通常只关心最近的一艘船，所以使用单个ID
- **对向碰撞**：可能同时面临多艘对向船只，所以使用ID列表
- **获取所有相关船只**：`front_collision_boat_id` + `oncoming_collision_boat_ids`

**使用示例**：
```javascript
function getAllInvolvedBoats(alert) {
    let boats = [];
    
    // 添加前向碰撞船只
    if (alert.front_collision_boat_id) {
        boats.push(alert.front_collision_boat_id);
    }
    
    // 添加对向碰撞船只
    if (alert.oncoming_collision_boat_ids) {
        boats = boats.concat(alert.oncoming_collision_boat_ids);
    }
    
    return boats; // 返回所有相关船只ID
}
```

### Q4: "立即减速并右转避让"：在出坞时，可能与其它正常行驶的船发生碰撞，碰撞的角度可能是90°，这种情况的避碰决策建议是什么？

**A4**: 对于90度交叉碰撞（出坞船只与正常航行船只），避碰决策使用**枚举代码**，遵循优先级规则：

#### 优先级规则（从高到低）
1. **入坞船只** (status=3) - 最高优先级
2. **正常航行船只** (status=2) - 中等优先级  
3. **出坞船只** (status=1) - 最低优先级

#### 90度碰撞决策枚举代码

**场景**: 出坞船只与正常航行船只形成90度交叉碰撞

| 船只类型 | 优先级 | 决策枚举代码 | 代码含义 |
|---------|-------|-------------|---------|
| **出坞船只** | 低 | `62` | 出坞船只必须避让 |
| **正常航行船只** | 高 | `64` | 正常航行船只优先权 |

#### 具体决策代码示例

**出坞船只收到的告警消息**：
```json
{
  "alert_level": 3,
  "avoidance_decision": 62,                    // 出坞船只必须避让
  "current_boat_id": 1,
  "front_collision_boat_id": null,
  "oncoming_collision_boat_ids": [2],
  "collision_time": 12.0,
  "collision_position": {"lat": 30.549832, "lng": 114.342922},
  "timestamp": 1692691200
}
```

**正常航行船只收到的告警消息**：
```json
{
  "alert_level": 2,
  "avoidance_decision": 64,                    // 正常航行船只优先权
  "current_boat_id": 2,
  "front_collision_boat_id": null,
  "oncoming_collision_boat_ids": [1],
  "collision_time": 12.0,
  "collision_position": {"lat": 30.549832, "lng": 114.342922},
  "timestamp": 1692691200
}
```

#### 特殊情况的决策代码

**紧急情况**（碰撞时间<5秒）：
- 出坞船只: `avoidance_decision: 40` (紧急停船)
- 正常航行船只: `avoidance_decision: 41` (紧急右转)

**多船冲突**（同时存在多个90度交叉）：
- 出坞船只: `avoidance_decision: 25` (停船等待对方通过)
- 正常航行船只: `avoidance_decision: 61` (保持优先通行权)

#### 决策代码对应表

| 代码 | 枚举名称 | 中文描述 | 适用场景 |
|------|---------|---------|---------|
| 25 | STOP_AND_WAIT | 停船等待对方通过 | 出坞船只让行 |
| 40 | EMERGENCY_STOP | 紧急停船 | 紧急避让 |
| 41 | EMERGENCY_TURN_RIGHT | 紧急右转 | 紧急转向避让 |
| 61 | MAINTAIN_PRIORITY_RIGHT | 保持优先通行权 | 正常航行船只 |
| 62 | UNDOCKING_MUST_YIELD | 出坞船只必须避让 | 出坞冲突标准处理 |
| 64 | NORMAL_SAILING_PRIORITY | 正常航行船只优先权 | 正常航行优先 |


### 监听所有主题
```bash
mosquitto_sub -h 127.0.0.1 -p 2000 -u vEagles -P 123456 \
  -t "CollisionAlert" -t "SafetyStatus" \
  -t "SystemStatus" -t "BoatState" \
  -t "DockInfo" -t "RouteInfo" -t "Config" -v
```

---

