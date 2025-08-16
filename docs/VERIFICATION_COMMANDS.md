# 验证命令速查表

## 🚀 一键验证命令

### 自动化验证
```bash
# 完整自动验证
./scripts/test_real_scenario.sh

# MQTT功能验证
./scripts/test_mqtt.sh

# 分步手动验证
./scripts/manual_verify_step_by_step.sh

# 简化验证程序
./mqtt_verify

# 验收演示
./scripts/acceptance_demo.sh
```

## 📡 MQTT基础命令

### 服务管理
```bash
# 启动MQTT服务
sudo systemctl start mosquitto

# 检查服务状态
systemctl status mosquitto

# 重启服务
sudo systemctl restart mosquitto

# 开机自启
sudo systemctl enable mosquitto
```

### 基础通信测试
```bash
# 订阅消息 (终端1)
mosquitto_sub -h localhost -t "test/topic" -v

# 发布消息 (终端2)
mosquitto_pub -h localhost -t "test/topic" -m "Hello World"

# 订阅所有主题 (调试用)
mosquitto_sub -h localhost -t "#" -v

# 带时间戳订阅
mosquitto_sub -h localhost -t "#" -v --pretty
```

## 🔧 项目编译命令

### 编译项目
```bash
# 创建构建目录
mkdir -p build && cd build

# 配置项目
cmake ..

# 编译项目
make -j$(nproc)

# 清理重编译
make clean && make

# 查看编译结果
ls -la mqtt_example simulation/boat_simulator
```

### 依赖安装
```bash
# 安装基础依赖
sudo apt-get update
sudo apt-get install libmosquitto-dev libjsoncpp-dev cmake build-essential

# 安装MQTT服务和客户端
sudo apt-get install mosquitto mosquitto-clients

# 检查安装结果
dpkg -l | grep -E "(mosquitto|jsoncpp|cmake)"
```

## 📊 系统监控命令

### 实时监控
```bash
# 监控系统状态
mosquitto_sub -h localhost -t "boat_safety/output/system_status" -v

# 监控碰撞告警
mosquitto_sub -h localhost -t "boat_safety/output/collision_alert/+" -v

# 监控避碰建议
mosquitto_sub -h localhost -t "boat_safety/output/avoidance_advice/+" -v

# 监控所有输出
mosquitto_sub -h localhost -t "boat_safety/output/+" -v

# 监控所有消息
mosquitto_sub -h localhost -t "boat_safety/#" -v
```

### 日志监控
```bash
# 查看MQTT服务日志
journalctl -u mosquitto -f

# 查看系统日志
tail -f /var/log/syslog | grep mosquitto

# 监控网络连接
netstat -tlnp | grep 1883

# 监控进程状态
ps aux | grep -E "(mqtt|boat)"
```

## 🚢 测试数据发送命令

### 船只状态数据
```bash
# 正常速度船只
mosquitto_pub -h localhost -t "boat_safety/input/boat_state/1" \
  -m '{"sysid":1,"timestamp":'$(date +%s)',"lat":30.549832,"lng":114.342922,"heading":90.0,"speed":2.5,"status":2,"route_direction":1}'

# 高速船只 (触发告警)
mosquitto_pub -h localhost -t "boat_safety/input/boat_state/2" \
  -m '{"sysid":2,"timestamp":'$(date +%s)',"lat":30.549900,"lng":114.342900,"heading":180.0,"speed":3.5,"status":2,"route_direction":1}'

# 出坞船只
mosquitto_pub -h localhost -t "boat_safety/input/boat_state/3" \
  -m '{"sysid":3,"timestamp":'$(date +%s)',"lat":30.549100,"lng":114.343000,"heading":45.0,"speed":1.0,"status":1,"route_direction":1}'

# 入坞船只
mosquitto_pub -h localhost -t "boat_safety/input/boat_state/4" \
  -m '{"sysid":4,"timestamp":'$(date +%s)',"lat":30.549200,"lng":114.343100,"heading":270.0,"speed":0.8,"status":3,"route_direction":2}'
```

### 船坞和航线数据
```bash
# 船坞信息
mosquitto_pub -h localhost -t "boat_safety/input/dock_info" \
  -m '{"dock_id":1,"lat":30.549100,"lng":114.343000}'

# 航线信息
mosquitto_pub -h localhost -t "boat_safety/input/route_info" \
  -m '{"route_id":1,"direction":1,"points":[{"lat":30.549500,"lng":114.342800},{"lat":30.549800,"lng":114.343300},{"lat":30.550100,"lng":114.343800}]}'

# 系统配置
mosquitto_pub -h localhost -t "boat_safety/input/config" \
  -m '{"boat":{"length":0.75,"width":0.47},"emergency_threshold_s":5,"warning_threshold_s":30,"max_boats":30}'
```

### 控制命令
```bash
# 启动监控命令
mosquitto_pub -h localhost -t "boat_safety/input/control" \
  -m '{"command":"start_monitoring","timestamp":'$(date +%s)'}'

# 停止监控命令
mosquitto_pub -h localhost -t "boat_safety/input/control" \
  -m '{"command":"stop_monitoring","timestamp":'$(date +%s)'}'

# 船坞锁定请求
mosquitto_pub -h localhost -t "boat_safety/dock_management/1/lock_request" \
  -m '{"dock_id":1,"boat_id":3,"operation":"lock","timestamp":'$(date +%s)'}'

# 船坞解锁请求
mosquitto_pub -h localhost -t "boat_safety/dock_management/1/unlock_request" \
  -m '{"dock_id":1,"boat_id":3,"operation":"unlock","timestamp":'$(date +%s)'}'
```

## 🏃 程序运行命令

### 示例程序
```bash
# 运行MQTT示例程序
cd build && ./mqtt_example

# 后台运行示例程序
cd build && ./mqtt_example > mqtt_example.log 2>&1 &

# 限时运行示例程序
cd build && timeout 30s ./mqtt_example

# 调试模式运行
cd build && gdb ./mqtt_example
```

### 仿真程序
```bash
# 运行船只仿真器
cd build/simulation && ./boat_simulator

# 后台运行仿真器
cd build/simulation && ./boat_simulator > boat_simulator.log 2>&1 &

# 限时运行仿真器
cd build/simulation && timeout 60s ./boat_simulator
```

### 验证程序
```bash
# 运行简化验证程序
./mqtt_verify

# 运行分步验证
./scripts/manual_verify_step_by_step.sh

# 运行完整场景测试
./scripts/test_real_scenario.sh

# 运行验收演示
./scripts/acceptance_demo.sh
```

## 🔍 调试和诊断命令

### 网络诊断
```bash
# 检查MQTT端口
telnet localhost 1883

# 检查网络连接
ss -tlnp | grep 1883

# 测试DNS解析
nslookup localhost

# 检查防火墙
sudo ufw status
```

### 进程诊断
```bash
# 查找相关进程
pgrep -f mqtt
pgrep -f boat

# 查看进程详情
ps aux | grep mqtt

# 监控资源使用
top -p $(pgrep -f mqtt | tr '\n' ',' | sed 's/,$//')

# 查看进程树
pstree -p $(pgrep -f mqtt)
```

### 日志分析
```bash
# 分析MQTT连接
grep -i "connect" /var/log/mosquitto/mosquitto.log

# 统计消息数量
grep -c "Updated boat state" mqtt_example.log

# 查找错误信息
grep -i "error\|fail\|exception" *.log

# 分析告警级别
grep -o '"alert_level":"[^"]*"' collision_alerts.log | sort | uniq -c
```

## 📈 性能测试命令

### 压力测试
```bash
# 批量发送消息
for i in {1..100}; do
  mosquitto_pub -h localhost -t "boat_safety/input/boat_state/$i" \
    -m '{"sysid":'$i',"timestamp":'$(date +%s)',"lat":30.549832,"lng":114.342922,"heading":90.0,"speed":2.5,"status":2,"route_direction":1}'
  sleep 0.1
done

# 并发连接测试
for i in {1..10}; do
  mosquitto_sub -h localhost -t "boat_safety/output/+" > test_$i.log &
done

# 消息延迟测试
time mosquitto_pub -h localhost -t "test/latency" -m "$(date +%s.%N)"
```

### 资源监控
```bash
# 监控内存使用
watch -n 1 'ps aux | grep mqtt | grep -v grep'

# 监控CPU使用
pidstat -p $(pgrep -f mqtt) 1

# 监控网络流量
iftop -i lo

# 监控磁盘IO
iotop -p $(pgrep -f mqtt)
```

## 🧹 清理命令

### 停止进程
```bash
# 停止所有相关进程
pkill -f mqtt
pkill -f boat

# 优雅停止
kill -TERM $(pgrep -f mqtt)

# 强制停止
kill -KILL $(pgrep -f mqtt)
```

### 清理文件
```bash
# 清理日志文件
rm -f *.log test_logs/*.log /tmp/*mqtt*.log

# 清理编译文件
cd build && make clean

# 重置环境
rm -rf build && mkdir build
```

### 重置服务
```bash
# 重启MQTT服务
sudo systemctl restart mosquitto

# 清理MQTT持久化数据
sudo rm -rf /var/lib/mosquitto/*

# 重置配置
sudo cp /etc/mosquitto/mosquitto.conf.example /etc/mosquitto/mosquitto.conf
```

## 📋 快速检查命令组合

### 环境检查
```bash
systemctl is-active mosquitto && echo "MQTT服务正常" || echo "MQTT服务异常"
ls build/mqtt_example build/simulation/boat_simulator 2>/dev/null && echo "程序编译正常" || echo "程序编译异常"
```

### 功能检查
```bash
# 一键功能测试
(mosquitto_sub -h localhost -t "test" -C 1 &) && sleep 1 && mosquitto_pub -h localhost -t "test" -m "test" && echo "MQTT通信正常"
```

### 完整检查
```bash
# 系统状态一览
echo "=== 系统状态 ==="
systemctl is-active mosquitto
ls -la build/mqtt_example build/simulation/boat_simulator 2>/dev/null
ps aux | grep -E "(mqtt|boat)" | grep -v grep
netstat -tlnp | grep 1883
```
