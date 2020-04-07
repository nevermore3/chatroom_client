cmake_minimum_required(VERSION 3.15)
project(client)

#set(CMAKE_CXX_STANDARD 14)

# 设置编译选项
# 允许c++14标准、O3优化、多线程。match选项可避免一些cpu上的问题
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -march=native -O3 -pthread" )

add_executable(client main.cpp client.cpp client.h)