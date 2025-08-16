# 脚本使用快速指南

## 🚀 快速开始

所有脚本文件已统一放置在 `scripts/` 文件夹中，使用前请确保脚本有执行权限：

```bash
chmod +x scripts/*.sh
```

## 📋 常用脚本命令

### 🔧 项目构建
```bash
# 构建项目
./scripts/build.sh
```

### 🧪 功能验证
```bash
# 快速MQTT功能验证
./scripts/test_mqtt.sh

# 完整真实场景测试
./scripts/test_real_scenario.sh

# 交互式分步验证（推荐新手）
./scripts/manual_verify_step_by_step.sh
```

### 🎯 验收演示
```bash
# 运行验收演示（适合甲方验收）
./scripts/acceptance_demo.sh
```

### 🔍 单元测试
```bash
# 运行单元测试
./scripts/run_tests.sh
```

## 📊 脚本选择建议

| 使用场景 | 推荐脚本 | 说明 |
|----------|----------|------|
| 首次使用 | `manual_verify_step_by_step.sh` | 交互式引导，适合学习 |
| 快速验证 | `test_mqtt.sh` | 2-3分钟快速功能验证 |
| 完整测试 | `test_real_scenario.sh` | 完整的真实场景测试 |
| 甲方验收 | `acceptance_demo.sh` | 专业的验收演示 |
| 开发调试 | `build.sh` + `run_tests.sh` | 构建和单元测试 |

## 🔗 相关文档

- 详细脚本说明：[SCRIPTS_README.md](SCRIPTS_README.md)
- 手动验证指南：[MANUAL_VERIFICATION_GUIDE.md](MANUAL_VERIFICATION_GUIDE.md)
- 验证命令速查：[VERIFICATION_COMMANDS.md](VERIFICATION_COMMANDS.md)
- 甲方数据接入：[CLIENT_DATA_INTEGRATION_GUIDE.md](CLIENT_DATA_INTEGRATION_GUIDE.md)

## ⚡ 一键验证

如果您想要最快速的验证体验：

```bash
# 一键完整验证（推荐）
./scripts/build.sh && ./scripts/test_real_scenario.sh

# 一键交互验证（适合新手）
./scripts/build.sh && ./scripts/manual_verify_step_by_step.sh
```
