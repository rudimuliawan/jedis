cmake_minimum_required(VERSION 3.29)
project(jedis)

set(CMAKE_CXX_STANDARD 20)

add_executable(client src/client.cpp)
add_executable(server src/server.cpp)
