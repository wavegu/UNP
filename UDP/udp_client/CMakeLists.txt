cmake_minimum_required(VERSION 3.1)
project(udp_client)

link_libraries(libunp.a)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(udp_client ${SOURCE_FILES} query.cpp query.h util.h util.cpp rtn_code.h)