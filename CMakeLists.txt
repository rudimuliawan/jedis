cmake_minimum_required(VERSION 4.0)

project(jedis)

set(CMAKE_CXX_STANDARD 17)

# static library
set(LIB_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jedis/tcp.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jedis/message.cpp"
)
add_library(jedis-lib STATIC ${LIB_SOURCES})
target_compile_features(jedis-lib
    PUBLIC
        cxx_std_17
)
target_include_directories(jedis-lib
    PUBLIC
        inc
)

# binary server
add_executable(server src/server.cpp)
target_include_directories(server
    PUBLIC
        inc
)
target_link_libraries(server
    PUBLIC
        jedis-lib
)

# binary client
add_executable(client src/client.cpp)
target_include_directories(client
    PUBLIC
        inc
)
target_link_libraries(client
    PUBLIC
        jedis-lib
)
