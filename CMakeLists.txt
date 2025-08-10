# ==================== CMakeLists.txt ====================
cmake_minimum_required(VERSION 3.16)
project(boat_pro)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)

include_directories(include)
include_directories(${JSONCPP_INCLUDE_DIRS})

# 创建静态库（排除main.cpp）
file(GLOB_RECURSE SOURCES "src/*.cpp")
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")
add_library(boat_pro_lib STATIC ${SOURCES})
target_link_libraries(boat_pro_lib ${JSONCPP_LIBRARIES})

# 创建主程序
add_executable(boat_pro src/main.cpp)
target_link_libraries(boat_pro boat_pro_lib ${JSONCPP_LIBRARIES})

# 创建测试程序
file(GLOB_RECURSE TEST_SOURCES "tests/*.cpp")
add_executable(boat_pro_test ${TEST_SOURCES})
target_link_libraries(boat_pro_test boat_pro_lib ${JSONCPP_LIBRARIES})

