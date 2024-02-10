cmake_minimum_required(VERSION 3.27)

project(jedis CXX)

add_executable(jedis-server src/server.cpp)

add_executable(jedis-client src/client.cpp)
