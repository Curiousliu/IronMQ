cmake_minimum_required(VERSION 3.6)
project(MyMQ)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

add_executable(MyMQ main.cpp Common/IpNode.h)