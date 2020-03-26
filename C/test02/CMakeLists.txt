cmake_minimum_required(VERSION 3.12)
project(test02 C)

set(CMAKE_C_STANDARD 99)

# 添加第三方的库的头文件
set(INC_DIR "D:/Cobject/clion/testlib02/include")
include_directories(${INC_DIR})

#先把第三方的库拷贝到本程序lib目录中现加本程序环景变量（PATH=D:\Cobject\clion\test02\lib）
link_directories(lib)


add_executable(test02 astest03.c)
target_link_libraries(test02 paho-mqtt3a.dll)