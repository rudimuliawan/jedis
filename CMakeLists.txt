cmake_minimum_required(VERSION 4.0)

project(jedis)

set(CMAKE_CXX_STANDARD 17)

add_executable(client src/client.cpp)
add_executable(server src/server.cpp)

target_include_directories(client
    PUBLIC
        inc
)

target_include_directories(server
    PUBLIC
        inc
)
