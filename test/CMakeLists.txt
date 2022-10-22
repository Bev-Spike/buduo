
# # 设置输出路径
# set(EXECUTABLE_OUTPUT_PATH ${OUT_DIR})


# 设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${OUT_DIR})


#设置编译目标
add_executable(server server.cpp)
target_link_libraries(server ${NET_LIB})

add_executable(chatCilent chatCilent.cpp)
target_link_libraries(chatCilent ${NET_LIB})
add_executable(chatServer chatServer.cpp)
target_link_libraries(chatServer ${NET_LIB})



add_executable(mutiCilent mutiCilent.cpp)
target_link_libraries(mutiCilent ${NET_LIB})

add_executable(logtest LogTest.cpp)
target_link_libraries(logtest ${NET_LIB})