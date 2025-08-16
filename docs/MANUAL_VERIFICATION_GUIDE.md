# 无人船作业安全预测系统 - 手动验证指南

## 验证准备

### 1. 环境检查
```bash
# 检查系统环境
uname -a
lsb_release -a

# 检查依赖包
dpkg -l | grep -E "(mosquitto|jsoncpp|cmake)"

# 检查MQTT服务状态
systemctl status mosquitto
```

### 2. 编译项目
```bash
cd /home/qzx/code/cpp/boat_pro
mkdir -p build && cd build
cmake ..
make -j$(nproc)

# 检查编译结果
ls -la mqtt_example simulation/boat_simulator
```

## 功能验证步骤

### 第一步：基础MQTT通信验证

#### 1.1 启动MQTT代理
```bash
# 启动mosquitto服务
sudo systemctl start mosquitto
sudo systemctl status mosquitto

# 检查端口监听
netstat -tlnp | grep 1883
```

#### 1.2 测试基本MQTT功能
**终端1 - 订阅消息**：
```bash
mosquitto_sub -h localhost -t "test/basic" -v
```

**终端2 - 发布消息**：
```bash
mosquitto_pub -h localhost -t "test/basic" -m "Hello MQTT"
```

**预期结果**：终端1应该显示 `test/basic Hello MQTT`

#### 1.3 测试JSON消息
**终端1 - 订阅JSON消息**：
```bash
mosquitto_sub -h localhost -t "boat_safety/test/json" -v
```

**终端2 - 发布JSON消息**：
```bash
mosquitto_pub -h localhost -t "boat_safety/test/json" \
  -m '{"test":"manual_verification","timestamp":'$(date +%s)',"status":"ok"}'
```

**预期结果**：终端1应该显示完整的JSON消息

### 第二步：MQTT接口功能验证

#### 2.1 启动系统监控
**终端1 - 监控系统状态**：
```bash
mosquitto_sub -h localhost -t "boat_safety/output/system_status" -v
```

**终端2 - 监控碰撞告警**：
```bash
mosquitto_sub -h localhost -t "boat_safety/output/collision_alert/+" -v
```

**终端3 - 监控避碰建议**：
```bash
mosquitto_sub -h localhost -t "boat_safety/output/avoidance_advice/+" -v
```

#### 2.2 启动MQTT示例程序
**终端4 - 运行示例程序**：
```bash
cd /home/qzx/code/cpp/boat_pro/build
./mqtt_example
```

**预期结果**：
- 终端4显示MQTT连接成功和订阅确认
- 终端1收到系统状态消息
- 程序运行10秒后自动退出

#### 2.3 手动发送测试数据
**发送船只状态数据**：
```bash
# 发送正常速度船只数据
mosquitto_pub -h localhost -t "boat_safety/input/boat_state/1" \
  -m '{
    "sysid": 1,
    "timestamp": '$(date +%s)',
    "lat": 30.549832,
    "lng": 114.342922,
    "heading": 90.0,
    "speed": 2.5,
    "status": 2,
    "route_direction": 1
  }'

# 发送高速船只数据（应触发告警）
mosquitto_pub -h localhost -t "boat_safety/input/boat_state/2" \
  -m '{
    "sysid": 2,
    "timestamp": '$(date +%s)',
    "lat": 30.549900,
    "lng": 114.342900,
    "heading": 180.0,
    "speed": 3.5,
    "status": 2,
    "route_direction": 1
  }'
```

**预期结果**：
- 示例程序日志显示"Updated boat state"
- 高速船只应触发碰撞告警（终端2收到告警消息）

**发送船坞信息**：
```bash
mosquitto_pub -h localhost -t "boat_safety/input/dock_info" \
  -m '{
    "dock_id": 1,
    "lat": 30.549100,
    "lng": 114.343000
  }'
```

**发送航线信息**：
```bash
mosquitto_pub -h localhost -t "boat_safety/input/route_info" \
  -m '{
    "route_id": 1,
    "direction": 1,
    "points": [
      {"lat": 30.549500, "lng": 114.342800},
      {"lat": 30.549800, "lng": 114.343300},
      {"lat": 30.550100, "lng": 114.343800}
    ]
  }'
```

### 第三步：船坞管理功能验证

#### 3.1 测试船坞锁定请求
**终端1 - 监控船坞响应**：
```bash
mosquitto_sub -h localhost -t "boat_safety/dock_management/+/status_response" -v
```

**终端2 - 发送锁定请求**：
```bash
mosquitto_pub -h localhost -t "boat_safety/dock_management/1/lock_request" \
  -m '{
    "dock_id": 1,
    "boat_id": 3,
    "operation": "lock",
    "timestamp": '$(date +%s)'
  }'
```

**预期结果**：终端1应收到船坞状态响应

#### 3.2 测试船坞解锁请求
```bash
mosquitto_pub -h localhost -t "boat_safety/dock_management/1/unlock_request" \
  -m '{
    "dock_id": 1,
    "boat_id": 3,
    "operation": "unlock",
    "timestamp": '$(date +%s)'
  }'
```

### 第四步：真实场景仿真验证

#### 4.1 启动完整监控
**启动多个监控终端**：
```bash
# 终端1 - 碰撞告警
mosquitto_sub -h localhost -t "boat_safety/output/collision_alert/+" > collision_alerts.log &

# 终端2 - 系统状态
mosquitto_sub -h localhost -t "boat_safety/output/system_status" > system_status.log &

# 终端3 - 船只状态（可选，数据量大）
# mosquitto_sub -h localhost -t "boat_safety/input/boat_state/+" > boat_states.log &
```

#### 4.2 启动安全预测系统
```bash
cd /home/qzx/code/cpp/boat_pro/build
# 修改示例程序运行时间或使用Ctrl+C手动停止
./mqtt_example &
SAFETY_PID=$!
echo "安全系统PID: $SAFETY_PID"
```

#### 4.3 启动船只仿真器
```bash
cd /home/qzx/code/cpp/boat_pro/build/simulation
./boat_simulator &
SIMULATOR_PID=$!
echo "仿真器PID: $SIMULATOR_PID"
```

#### 4.4 观察仿真运行
**实时监控进程状态**：
```bash
# 检查进程是否运行
ps -p $SAFETY_PID $SIMULATOR_PID

# 查看实时日志
tail -f collision_alerts.log
tail -f system_status.log
```

**手动停止仿真**：
```bash
# 停止仿真器和安全系统
kill $SIMULATOR_PID $SAFETY_PID

# 停止监控进程
pkill -f mosquitto_sub
```

### 第五步：数据验证和分析

#### 5.1 检查日志文件
```bash
# 查看碰撞告警
echo "=== 碰撞告警 ==="
cat collision_alerts.log | head -5

# 查看系统状态
echo "=== 系统状态 ==="
cat system_status.log | head -3

# 统计消息数量
echo "告警数量: $(wc -l < collision_alerts.log)"
echo "状态更新数量: $(wc -l < system_status.log)"
```

#### 5.2 验证JSON格式
```bash
# 验证JSON格式是否正确
echo "验证JSON格式..."
head -1 collision_alerts.log | python3 -m json.tool
head -1 system_status.log | python3 -m json.tool
```

#### 5.3 分析告警内容
```bash
# 分析告警级别分布
echo "=== 告警级别分析 ==="
grep -o '"alert_level":"[^"]*"' collision_alerts.log | sort | uniq -c

# 分析涉及的船只
echo "=== 涉及船只分析 ==="
grep -o '"boat_id":[0-9]*' collision_alerts.log | sort | uniq -c
```

## 验证检查清单

### ✅ 基础功能检查
- [ ] MQTT服务正常启动
- [ ] 基本消息发布/订阅正常
- [ ] JSON消息格式正确
- [ ] 程序编译无错误
- [ ] 程序启动无异常

### ✅ MQTT接口检查
- [ ] 成功连接到MQTT代理
- [ ] 正确订阅所有必要主题
- [ ] 能够接收和处理船只状态消息
- [ ] 能够接收和处理船坞信息消息
- [ ] 能够接收和处理航线信息消息
- [ ] 能够发布系统状态消息
- [ ] 能够发布碰撞告警消息

### ✅ 业务逻辑检查
- [ ] 船只状态数据格式验证正确
- [ ] 高速船只触发告警
- [ ] 船坞管理请求得到响应
- [ ] 航线信息正确处理
- [ ] 消息时间戳格式正确

### ✅ 仿真系统检查
- [ ] 仿真器成功启动
- [ ] 船只能够出坞
- [ ] 船只按航线航行
- [ ] 实时位置更新
- [ ] 速度和航向变化合理
- [ ] 系统运行稳定

### ✅ 性能检查
- [ ] 消息延迟在可接受范围内
- [ ] 系统资源使用合理
- [ ] 无内存泄漏
- [ ] 长时间运行稳定

## 常见问题排查

### 问题1：MQTT连接失败
**症状**：程序显示"Failed to connect to MQTT broker"
**排查步骤**：
```bash
# 检查mosquitto服务
sudo systemctl status mosquitto

# 检查端口占用
netstat -tlnp | grep 1883

# 重启服务
sudo systemctl restart mosquitto
```

### 问题2：消息接收不到
**症状**：发布消息后订阅端收不到
**排查步骤**：
```bash
# 检查主题名称是否正确
mosquitto_sub -h localhost -t "#" -v  # 订阅所有主题

# 检查消息格式
mosquitto_pub -h localhost -t "test" -m "test" -d  # 开启调试模式
```

### 问题3：JSON解析错误
**症状**：程序日志显示JSON解析失败
**排查步骤**：
```bash
# 验证JSON格式
echo '你的JSON消息' | python3 -m json.tool

# 检查特殊字符和编码
hexdump -C <<< '你的JSON消息'
```

### 问题4：程序异常退出
**症状**：程序运行一段时间后崩溃
**排查步骤**：
```bash
# 使用gdb调试
gdb ./mqtt_example
(gdb) run
# 程序崩溃后
(gdb) bt  # 查看调用栈

# 检查系统日志
journalctl -u mosquitto -f
```

### 问题5：仿真数据不真实
**症状**：船只行为不符合预期
**排查步骤**：
```bash
# 检查仿真参数
grep -E "(SPEED|DISTANCE)" simulation/boat_simulator.cpp

# 调整仿真速度
# 修改 SIMULATION_SPEED 参数
```

## 验证报告模板

### 验证记录表
```
验证日期：____年__月__日
验证人员：________________
验证环境：________________

基础功能验证：
□ MQTT连接：正常/异常
□ 消息订阅：正常/异常  
□ 消息发布：正常/异常
□ JSON处理：正常/异常

业务功能验证：
□ 船只状态处理：正常/异常
□ 船坞管理：正常/异常
□ 航线管理：正常/异常
□ 碰撞告警：正常/异常

性能验证：
□ 响应时间：____ms
□ 内存使用：____MB
□ CPU使用：____%
□ 运行稳定性：正常/异常

发现问题：
1. ________________________
2. ________________________
3. ________________________

改进建议：
1. ________________________
2. ________________________
3. ________________________

总体评价：优秀/良好/一般/需改进
```

## 自动化验证脚本

如果您希望简化验证过程，可以使用我们提供的自动化脚本：

```bash
# 运行完整验证
./scripts/test_real_scenario.sh

# 运行MQTT功能验证
./scripts/test_mqtt.sh

# 运行基础功能验证
./mqtt_verify

# 运行分步手动验证
./scripts/manual_verify_step_by_step.sh
```

通过以上手动验证步骤，您可以全面了解和验证系统的各项功能，确保项目的正确性和可靠性。
