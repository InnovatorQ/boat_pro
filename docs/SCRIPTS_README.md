# Scripts 文件夹说明

本文件夹包含无人船作业安全预测系统的所有脚本文件。

## 📁 脚本文件列表

### 🔧 构建和测试脚本
- **`build.sh`** - 项目构建脚本
  ```bash
  ./scripts/build.sh
  ```
  功能：检查依赖、编译项目、生成可执行文件

- **`run_tests.sh`** - 运行测试脚本
  ```bash
  ./scripts/run_tests.sh
  ```
  功能：运行项目的单元测试

### 🧪 验证测试脚本
- **`test_mqtt.sh`** - MQTT功能验证脚本
  ```bash
  ./scripts/test_mqtt.sh
  ```
  功能：验证MQTT接口的基本功能

- **`test_real_scenario.sh`** - 真实场景测试脚本
  ```bash
  ./scripts/test_real_scenario.sh
  ```
  功能：运行完整的真实场景仿真测试

- **`manual_verify_step_by_step.sh`** - 分步手动验证脚本
  ```bash
  ./scripts/manual_verify_step_by_step.sh
  ```
  功能：交互式分步验证系统各项功能

### 🎯 验收演示脚本
- **`acceptance_demo.sh`** - 验收演示脚本
  ```bash
  ./scripts/acceptance_demo.sh
  ```
  功能：为甲方验收提供完整的系统演示

## 🚀 使用建议

### 新手用户
推荐按以下顺序使用脚本：
1. `./scripts/build.sh` - 构建项目
2. `./scripts/manual_verify_step_by_step.sh` - 分步验证
3. `./scripts/test_mqtt.sh` - MQTT功能测试

### 开发测试
开发过程中推荐使用：
1. `./scripts/build.sh` - 构建项目
2. `./scripts/run_tests.sh` - 运行单元测试
3. `./scripts/test_real_scenario.sh` - 完整场景测试

### 验收演示
验收阶段推荐使用：
1. `./scripts/build.sh` - 确保项目构建正常
2. `./scripts/acceptance_demo.sh` - 运行验收演示
3. `./scripts/test_real_scenario.sh` - 展示真实场景

## 📋 脚本功能对比

| 脚本名称 | 用途 | 交互性 | 运行时间 | 适用场景 |
|----------|------|--------|----------|----------|
| build.sh | 构建项目 | 低 | 1-2分钟 | 开发/部署 |
| run_tests.sh | 单元测试 | 低 | < 1分钟 | 开发测试 |
| test_mqtt.sh | MQTT测试 | 低 | 2-3分钟 | 功能验证 |
| test_real_scenario.sh | 场景测试 | 低 | 2-3分钟 | 完整测试 |
| manual_verify_step_by_step.sh | 手动验证 | 高 | 10-15分钟 | 学习/调试 |
| acceptance_demo.sh | 验收演示 | 高 | 15-20分钟 | 验收展示 |

## ⚠️ 注意事项

### 运行前准备
1. 确保MQTT服务运行：`sudo systemctl start mosquitto`
2. 确保项目已编译：`./scripts/build.sh`
3. 确保有执行权限：`chmod +x scripts/*.sh`

### 环境要求
- Ubuntu 22.04 LTS 或兼容系统
- 已安装 libmosquitto-dev 和 libjsoncpp-dev
- 已安装 cmake 和 build-essential
- 网络连接正常

### 故障排除
如果脚本运行失败，请检查：
1. 系统依赖是否完整安装
2. MQTT服务是否正常运行
3. 网络连接是否正常
4. 文件权限是否正确设置

## 📞 技术支持

如果在使用脚本过程中遇到问题，请：
1. 查看脚本输出的错误信息
2. 检查系统日志：`journalctl -u mosquitto`
3. 参考项目文档中的故障排除部分
4. 联系技术支持团队

