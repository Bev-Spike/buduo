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
#set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")

# 输出目录
set(OUT_DIR "${CMAKE_SOURCE_DIR}/out")

#目标源码目录
set(NET_SRC_DIR "${CMAKE_SOURCE_DIR}/src")

#头文件目录
set(NET_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/src/include")

#子项目目录


#编译目标名
set(SERVER_TARGET server)
set(CILENT_TARGET cilent)
set(TEST_TARGET ttest)


#编译子目录
add_subdirectory(${NET_SRC_DIR}) 

# 添加头文件查找路径
INCLUDE_DIRECTORIES(${NET_INCLUDE_DIR})

set(NET_LIB netlib)

# 设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${OUT_DIR})


#设置编译目标
add_executable(${SERVER_TARGET} ./test/server.cpp)
add_executable(${CILENT_TARGET} ./test/cilent.cpp)
add_executable(${TEST_TARGET} ./test/ttest.cpp)
add_executable(mutiCilent ./test/mutiCilent.cpp)
# 添加链接库

target_link_libraries(${SERVER_TARGET} ${NET_LIB})
target_link_libraries(${CILENT_TARGET} ${NET_LIB})
target_link_libraries(${TEST_TARGET} ${NET_LIB})
target_link_libraries(mutiCilent ${NET_LIB})