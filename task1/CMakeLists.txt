cmake_minimum_required(VERSION 3.10)
project(SinArraySum)

# Получаем аргумент через флаг, передаваемый в командной строке
if(NOT DEFINED TYPE)
    message(FATAL_ERROR "Please specify 'TYPE=float' or 'TYPE=double' as an argument.")
elseif(TYPE STREQUAL "float")
    set(USE_FLOAT ON)
elseif(TYPE STREQUAL "double")
    set(USE_FLOAT OFF)
else()
    message(FATAL_ERROR "Invalid value for TYPE. Please specify 'float' or 'double'.")
endif()

set(CMAKE_CXX_STANDARD 17)

# Включаем макросы в зависимости от выбора
if(USE_FLOAT)
    add_definitions(-DUSE_FLOAT)
    set(EXECUTABLE_NAME "sin_float")
else()
    add_definitions(-DUSE_DOUBLE)
    set(EXECUTABLE_NAME "sin_double")
endif()

add_executable(${EXECUTABLE_NAME} src/main.cpp)
