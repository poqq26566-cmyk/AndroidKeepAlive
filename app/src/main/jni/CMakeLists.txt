cmake_minimum_required(VERSION 3.10)
project("keepalive")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -march=armv8-a")

add_library(keepalive SHARED
    KeepAlive.c
    KeepAlive.cpp
)

target_include_directories(keepalive PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(keepalive
    log
    c
)
