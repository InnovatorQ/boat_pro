cmake_minimum_required(VERSION 3.10)
project(boat_pro)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# 查找依赖包
find_package(PkgConfig REQUIRED)

# 查找线程库用于UDP通信
find_package(Threads REQUIRED)

# 查找mosquitto库用于MQTT通信
find_library(MOSQUITTO_LIB mosquitto)
if(NOT MOSQUITTO_LIB)
    message(FATAL_ERROR "libmosquitto not found. Please install mosquitto development library.")
endif()

# 查找jsoncpp库
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

# 如果找不到jsoncpp，尝试其他方式
if(NOT JSONCPP_FOUND)
    find_library(JSONCPP_LIBRARIES jsoncpp)
    find_path(JSONCPP_INCLUDE_DIRS json/json.h)
endif()

# 设置包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${JSONCPP_INCLUDE_DIRS}
)

# 收集源文件
file(GLOB_RECURSE SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# 创建静态库
add_library(boat_pro_lib STATIC ${SOURCES})
target_link_libraries(boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

# 创建主程序
add_executable(boat_pro src/main.cpp)
target_link_libraries(boat_pro boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

# 创建独立的测试程序
add_executable(test_collision_detector tests/test_collision_detector.cpp)
target_link_libraries(test_collision_detector boat_pro_lib ${JSONCPP_LIBRARIES} Threads::Threads)

add_executable(test_communication tests/test_communication.cpp)
target_link_libraries(test_communication boat_pro_lib ${JSONCPP_LIBRARIES} Threads::Threads)

# 创建碰撞决策测试程序
add_executable(test_collision_decision tests/test_collision_decision.cpp)
target_link_libraries(test_collision_decision boat_pro_lib ${JSONCPP_LIBRARIES} Threads::Threads)

# 创建简化碰撞测试程序
add_executable(test_simple_collision tests/test_simple_collision.cpp)
target_link_libraries(test_simple_collision boat_pro_lib ${JSONCPP_LIBRARIES} Threads::Threads)

# 创建90度碰撞测试程序
add_executable(test_90degree_collision tests/test_90degree_collision.cpp)
target_link_libraries(test_90degree_collision boat_pro_lib ${JSONCPP_LIBRARIES} Threads::Threads)

# 创建主程序
add_executable(boat_pro src/main.cpp)
target_link_libraries(boat_pro boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

# 创建核心测试程序
add_executable(test_collision_detector tests/test_collision_detector.cpp)
target_link_libraries(test_collision_detector boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

add_executable(test_communication tests/test_communication.cpp)
target_link_libraries(test_communication boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

add_executable(mqtt_test tests/test_mqtt.cpp)
target_link_libraries(mqtt_test boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

# 创建功能演示程序
add_executable(mqtt_example examples/mqtt_example.cpp)
target_link_libraries(mqtt_example boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

add_executable(collision_angle_test examples/collision_angle_test.cpp)
target_link_libraries(collision_angle_test boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

add_executable(avoidance_decision_test examples/avoidance_decision_test.cpp)
target_link_libraries(avoidance_decision_test boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

# 安装规则
install(TARGETS boat_pro_lib
    ARCHIVE DESTINATION lib
)

install(DIRECTORY include/
    DESTINATION include
    FILES_MATCHING PATTERN "*.h"
)

install(FILES config/mqtt_config.json
    DESTINATION etc/boat_pro
)

# 创建测试目标（可选）
enable_testing()

# 打印配置信息
message(STATUS "Mosquitto libraries: ${MOSQUITTO_LIB}")
message(STATUS "JsonCpp libraries: ${JSONCPP_LIBRARIES}")
message(STATUS "JsonCpp include dirs: ${JSONCPP_INCLUDE_DIRS}")

# 创建配置文件
configure_file(
    "${CMAKE_SOURCE_DIR}/config/mqtt_config.json"
    "${CMAKE_BINARY_DIR}/mqtt_config.json"
    COPYONLY
)
