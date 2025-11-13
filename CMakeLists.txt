cmake_minimum_required(VERSION 3.10)
project(boat_collision_predictor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP REQUIRED jsoncpp)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${JSONCPP_INCLUDE_DIRS}
)

# 收集所有源文件（排除main.cpp）
file(GLOB LIB_SOURCES 
    "src/types.cpp"
    "src/avoidance_decision_types.cpp"
    "src/geometry_utils.cpp"
    "src/collision_detector.cpp"
    "src/fleet_manager.cpp"
)

# 创建静态库
add_library(boat_pro_lib STATIC ${LIB_SOURCES})
target_link_libraries(boat_pro_lib ${JSONCPP_LIBRARIES})

# 创建主程序
add_executable(boat_collision_predictor src/main.cpp)
target_link_libraries(boat_collision_predictor boat_pro_lib ${JSONCPP_LIBRARIES})
