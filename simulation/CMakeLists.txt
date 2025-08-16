cmake_minimum_required(VERSION 3.10)

# 设置包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    /usr/include/jsoncpp
)

# 查找依赖
find_package(PkgConfig REQUIRED)
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# 创建仿真器可执行文件
add_executable(boat_simulator boat_simulator.cpp)

# 链接库
target_link_libraries(boat_simulator
    ${MOSQUITTO_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    pthread
)

# 设置编译标志
target_compile_options(boat_simulator PRIVATE
    ${MOSQUITTO_CFLAGS_OTHER}
    ${JSONCPP_CFLAGS_OTHER}
)
