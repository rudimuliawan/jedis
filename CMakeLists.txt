cmake_minimum_required(VERSION 3.27)

project(jedis CXX)

# Add the local CMake modules folder to the CMake search path.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Static library
set(LIB_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/message_protocol.cpp"
        src/util.cpp
        inc/util.h
)
add_library(jedis STATIC ${LIB_SOURCES})
target_compile_features(jedis
    PUBLIC
        cxx_std_17
)
target_include_directories(jedis
    PUBLIC
        inc
)

# jedis-server
set(BINARY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/server.cpp"
)
add_executable(jedis-server ${BINARY_SOURCES})
target_link_libraries(jedis-server
    PRIVATE
        jedis
)

# executable jedis-client
set(BINARY_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp"
)
add_executable(jedis-client ${BINARY_SOURCES})
target_link_libraries(jedis-client
    PRIVATE
        jedis
)
