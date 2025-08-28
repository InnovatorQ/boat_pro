#!/bin/bash

# ==================== scripts/test_collision_angle.sh ====================
# 碰撞角度计算功能测试脚本

set -e

echo "========================================"
echo "碰撞角度计算功能测试"
echo "========================================"

# 检查构建目录
if [ ! -d "build" ]; then
    echo "错误: 构建目录不存在，请先运行 ./scripts/build.sh"
    exit 1
fi

cd build

# 检查可执行文件
if [ ! -f "collision_angle_test" ]; then
    echo "错误: collision_angle_test 程序不存在，请先编译项目"
    exit 1
fi

echo "运行碰撞角度计算测试..."
echo "----------------------------------------"

# 运行测试程序
./collision_angle_test

echo ""
echo "========================================"
echo "测试完成！"
echo "========================================"
echo ""
echo "功能验证项目："
echo "✓ 碰撞角度计算算法"
echo "✓ 碰撞类型分类逻辑"
echo "✓ JSON序列化新字段"
echo "✓ 实际场景演示"
echo ""
echo "新增字段说明："
echo "- collision_angle: 两船航向夹角 (0-180°)"
echo "- collision_type: 碰撞类型分类"
echo "  * overtaking: 追尾碰撞 (0-30°)"
echo "  * oblique: 斜向碰撞 (30-60°)"
echo "  * crossing: 交叉碰撞 (60-120°)"
echo "  * oblique_head_on: 斜向对撞 (120-150°)"
echo "  * head_on: 正面对撞 (150-180°)"
