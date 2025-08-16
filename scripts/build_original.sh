#!/bin/bash

# 无人船作业安全预测系统构建脚本

echo "开始构建无人船安全预测系统..."

# 创建构建目录
mkdir -p build
cd build

# 运行CMake配置
cmake ..

# 编译项目
make -j4

if [ $? -eq 0 ]; then
    echo "构建成功!"
    echo "可执行文件位于: build/boat_pro"
    echo "测试程序位于: build/boat_pro_test"
else
    echo "构建失败!"
    exit 1
fi
