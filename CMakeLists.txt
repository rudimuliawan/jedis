cmake_minimum_required(VERSION 3.17 FATAL_ERROR)

project(jedis)

# Static Library
set(LIB_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jedis-utils.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jedis-message.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jedis-connection.cpp"
)
add_library(jedis-utils STATIC ${LIB_SOURCES})
target_compile_features(jedis-utils
    PUBLIC
        cxx_std_20
)
target_include_directories(jedis-utils
    PUBLIC
        inc
)

# Jedis Server
set(JEDIS_SERVER_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jedis-server.cpp"
)
add_executable(jedis-server ${JEDIS_SERVER_SOURCES})
target_include_directories(jedis-server
    PUBLIC
        inc
)
target_link_libraries(jedis-server
    PUBLIC
        jedis-utils
)

# Jedis Client
set(JEDIS_CLIENT_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/jedis-client.cpp"
)
add_executable(jedis-client ${JEDIS_CLIENT_SOURCES})
target_compile_features(jedis-client
    PUBLIC
        cxx_std_20
)
target_include_directories(jedis-client
    PUBLIC
        inc
)
target_link_libraries(jedis-client
    PUBLIC
        jedis-utils
)
