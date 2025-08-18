# ==================== CMakeLists.txt ====================
cmake_minimum_required(VERSION 3.16)
project(boat_pro)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

# 查找线程库用于UDP通信
find_package(Threads REQUIRED)

# 查找mosquitto库用于MQTT通信
find_library(MOSQUITTO_LIB mosquitto)
if(NOT MOSQUITTO_LIB)
    message(FATAL_ERROR "libmosquitto not found. Please install mosquitto development library.")
endif()

include_directories(include)
include_directories(${JSONCPP_INCLUDE_DIRS})

# 创建静态库（排除main.cpp）
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
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

# 创建MQTT示例程序
add_executable(mqtt_example examples/mqtt_example.cpp)
target_link_libraries(mqtt_example boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

# 创建MQTT数据接收器程序
add_executable(mqtt_data_receiver examples/mqtt_data_receiver.cpp)
target_link_libraries(mqtt_data_receiver boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

# 创建简单MQTT测试程序
add_executable(simple_mqtt_test examples/simple_mqtt_test.cpp)
target_link_libraries(simple_mqtt_test boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

# 创建MQTT测试程序
add_executable(mqtt_test tests/test_mqtt.cpp)
target_link_libraries(mqtt_test boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

# 创建MQTT客户端配置测试程序
add_executable(mqtt_client_test examples/mqtt_client_test.cpp)
target_link_libraries(mqtt_client_test boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

# 创建简单MQTT连接测试程序
add_executable(simple_mqtt_connection_test examples/simple_mqtt_connection_test.cpp)
target_link_libraries(simple_mqtt_connection_test boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

# 创建心跳监控程序
add_executable(heartbeat_monitor examples/heartbeat_monitor.cpp)
target_link_libraries(heartbeat_monitor boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

# 创建MPC客户端测试程序
add_executable(mpc_client_test examples/mpc_client_test.cpp)
target_link_libraries(mpc_client_test boat_pro_lib ${JSONCPP_LIBRARIES} ${MOSQUITTO_LIB} Threads::Threads)

