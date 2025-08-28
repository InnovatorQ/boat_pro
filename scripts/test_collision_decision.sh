#!/bin/bash

# 碰撞决策系统测试脚本
# 演示优化后的智能决策建议功能

echo "=== 碰撞决策系统测试 ==="
echo "测试优化后的智能避碰决策建议系统"
echo ""

# 检查构建目录
if [ ! -d "build" ]; then
    echo "构建目录不存在，正在创建..."
    mkdir build
fi

cd build

# 检查是否需要重新构建
if [ ! -f "test_collision_decision" ] || [ "../src/collision_decision_engine.cpp" -nt "test_collision_decision" ]; then
    echo "正在编译碰撞决策测试程序..."
    cmake .. && make test_collision_decision
    
    if [ $? -ne 0 ]; then
        echo "编译失败！"
        exit 1
    fi
    echo "编译完成！"
    echo ""
fi

echo "开始运行碰撞决策测试..."
echo "========================================"
echo ""

# 运行测试程序
./test_collision_decision

echo ""
echo "========================================"
echo "测试完成！"
echo ""
echo "优化后的决策系统特点："
echo "1. 根据不同碰撞类型给出具体建议"
echo "2. 考虑船只优先级和海上避碰规则"
echo "3. 提供详细的速度和转向指令"
echo "4. 处理多船冲突和紧急情况"
echo "5. 包含安全距离和时间计算"
echo ""
echo "决策建议包含："
echo "- 具体的速度调整建议"
echo "- 明确的转向指令"
echo "- 优先级判断"
echo "- 紧急情况处理"
echo "- 多威胁协调避让"
