cmake_minimum_required(VERSION 3.10)
project(boat_pro)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# 查找依赖包
find_package(PkgConfig REQUIRED)

# 查找mosquitto库
pkg_check_modules(MOSQUITTO REQUIRED libmosquitto)

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
    ${MOSQUITTO_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
)

# 设置库目录
link_directories(
    ${MOSQUITTO_LIBRARY_DIRS}
    ${JSONCPP_LIBRARY_DIRS}
)

# 收集源文件
file(GLOB_RECURSE SOURCES 
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# 创建静态库
add_library(boat_pro_lib STATIC ${SOURCES})

# 链接库
target_link_libraries(boat_pro_lib
    ${MOSQUITTO_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    pthread
)

# 设置编译标志
target_compile_options(boat_pro_lib PRIVATE
    ${MOSQUITTO_CFLAGS_OTHER}
    ${JSONCPP_CFLAGS_OTHER}
)

# 创建示例程序
add_executable(mqtt_example examples/mqtt_example.cpp)
target_link_libraries(mqtt_example boat_pro_lib)

# 添加仿真器
add_subdirectory(simulation)

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

# 如果有测试文件，可以添加测试
# add_subdirectory(tests)

# 打印配置信息
message(STATUS "Mosquitto libraries: ${MOSQUITTO_LIBRARIES}")
message(STATUS "Mosquitto include dirs: ${MOSQUITTO_INCLUDE_DIRS}")
message(STATUS "JsonCpp libraries: ${JSONCPP_LIBRARIES}")
message(STATUS "JsonCpp include dirs: ${JSONCPP_INCLUDE_DIRS}")

# 创建配置文件
configure_file(
    "${CMAKE_SOURCE_DIR}/config/mqtt_config.json"
    "${CMAKE_BINARY_DIR}/mqtt_config.json"
    COPYONLY
)
