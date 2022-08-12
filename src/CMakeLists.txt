aux_source_directory(${NET_SRC_DIR} LIB_SRCS)			# 添加当前目录下所有的源文件
add_library (netlib STATIC ${LIB_SRCS})				# 当前目录下的文件生成一个链接库
target_include_directories(netlib PUBLIC ${NET_INCLUDE_DIR} )  #允许上级继承头文件
# 添加pthread链接
find_package(Threads)
# 链接库到项目中
target_link_libraries (netlib ${CMAKE_THREAD_LIBS_INIT})
