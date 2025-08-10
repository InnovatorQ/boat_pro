#!/bin/bash

# 运行测试脚本

echo "运行无人船安全预测系统测试..."

if [ ! -f "build/boat_pro_test" ]; then
    echo "测试程序不存在，请先构建项目"
    exit 1
fi

# 运行测试
./build/boat_pro_test

if [ $? -eq 0 ]; then
    echo "所有测试通过!"
else
    echo "测试失败!"
    exit 1
fi