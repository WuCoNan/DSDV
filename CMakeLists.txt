cmake_minimum_required(VERSION 3.10)
project(DSDV)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含头文件目录（关键！）
include_directories(${PROJECT_SOURCE_DIR}/include)

# 调试配置
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")

# 生成可执行文件
add_executable(dsdv_test src/test.cpp src/DSDV.cpp src/NetworkLayer.cpp src/EthernetLayer.cpp src/Application.cpp src/Logger.cpp)