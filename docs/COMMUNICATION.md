# 通信系统文档

## 概述

无人船作业安全预测系统集成了基于UDP的通信功能，支持Drone ID标准和NMEA 2000协议，用于船只间的数据交换和状态同步。

## 支持的协议

### 1. Drone ID 标准
- **协议标识**: 0xDD
- **支持消息类型**:
  - BASIC_ID (0x00): 基本标识信息
  - LOCATION (0x01): 位置和运动数据
- **数据格式**: 符合Drone ID标准规范
- **用途**: 提供船只身份识别和位置广播

### 2. NMEA 2000 协议
- **协议标识**: 0x4E32 (ASCII "N2")
- **支持PGN**:
  - 129025: 位置快速更新
  - 129026: 航向和航速快速更新
- **数据格式**: 符合NMEA 2000标准
- **用途**: 与现有海事设备兼容

## 系统架构

```
FleetManager
    ↓
UDPCommunicator ←→ UDP Network
    ↓
ProtocolConverter
    ↓
DroneID/NMEA2000 Messages
```

## 使用方法

### 初始化通信系统

```cpp
// 配置UDP通信参数
communication::UDPConfig config;
config.local_port = 8888;
config.remote_port = 8889;
config.enable_broadcast = true;

// 初始化
FleetManager fleet_manager;
fleet_manager.initializeCommunication(config);
fleet_manager.startCommunication();
```

### 发送船只状态

```cpp
BoatState boat;
// ... 设置船只状态 ...

// 广播状态(支持两种协议)
fleet_manager.broadcastBoatState(boat, true, true);
```

### 接收消息处理

系统自动处理接收到的消息，并转换为内部船只状态格式，无需手动处理。

## 协议详细说明

### Drone ID Location Message (位置消息)
- **大小**: 35字节
- **包含信息**: 位置、速度、航向、时间戳、精度信息
- **更新频率**: 建议1-2秒

### NMEA 2000 Position Message (位置消息)
- **PGN**: 129025
- **大小**: 8字节
- **包含信息**: 纬度、经度

### NMEA 2000 COG/SOG Message (航向速度消息)  
- **PGN**: 129026
- **大小**: 8字节
- **包含信息**: 对地航向、对地速度

## 网络配置

### 默认端口分配
- **接收端口**: 8888
- **发送端口**: 8889
- **协议**: UDP
- **广播**: 支持

### 防火墙配置
确保以下端口开放:
```bash
sudo ufw allow 8888/udp
sudo ufw allow 8889/udp
```

## 错误处理

系统提供完整的错误统计和处理机制:
- 发送失败重试
- 接收超时处理
- 协议解析错误处理
- 网络异常恢复

## 性能特性

- **消息处理能力**: >1000 messages/sec
- **延迟**: <10ms (局域网)
- **带宽占用**: ~500 bytes/boat/second
- **可靠性**: UDP + 应用层重传

## 扩展性

系统设计支持:
- 新协议类型添加
- 自定义消息格式
- 多播和单播模式
- 加密和认证(预留接口)