cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(jedis)

# Static Library
set(LIB_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jedis-utils.c"
)
add_library(jedis-utils STATIC ${LIB_SOURCES})
target_include_directories(jedis-utils
    PUBLIC
        inc
)

# Bin
set(JEDIS_SERVER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jedis-server.c"
)
add_executable(jedis-server ${JEDIS_SERVER_SOURCES})
target_include_directories(jedis-server
    PUBLIC
        inc
)
target_link_libraries(jedis-server
    PRIVATE
        jedis-utils
)
