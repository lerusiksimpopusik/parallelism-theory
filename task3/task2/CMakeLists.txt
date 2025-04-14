cmake_minimum_required(VERSION 3.22.1)
project(task2)

set(CMAKE_CXX_STANDARD 20)

add_executable(task2 main.cpp)

target_include_directories(task2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/functions
    ${CMAKE_CURRENT_SOURCE_DIR}/server
    ${CMAKE_CURRENT_SOURCE_DIR}/client
)

target_sources(task2 PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/functions/functions.h
    ${CMAKE_CURRENT_SOURCE_DIR}/server/server.h
    ${CMAKE_CURRENT_SOURCE_DIR}/server/server.tpp
    ${CMAKE_CURRENT_SOURCE_DIR}/client/client.h
    ${CMAKE_CURRENT_SOURCE_DIR}/client/client.tpp
)
