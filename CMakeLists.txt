#最低要求的CMake版本为3.10
cmake_minimum_required (VERSION 3.10) 
# 设置项目名
project(buduo LANGUAGES C CXX)


#如果没有指定编译类型，自动设置为Debug
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# C++17
set(CMAKE_CXX_STANDARD 17)
# 设置编译器是否一定要支持该标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 设置是否启用GCC的拓展功能
set(CMAKE_CXX_EXTENSIONS OFF)
# 设置编译选项
# set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")

# 输出目录
set(OUT_DIR "${CMAKE_SOURCE_DIR}/out")

#目标源码目录
set(NET_SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(TEST_SRC_DIR "${CMAKE_SOURCE_DIR}/test")
#头文件目录
set(NET_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/include")


set(NET_LIB netlib)


#编译子目录
add_subdirectory(${NET_SRC_DIR}) 
add_subdirectory(${TEST_SRC_DIR})
# 添加头文件查找路径
INCLUDE_DIRECTORIES(${NET_INCLUDE_DIR})

