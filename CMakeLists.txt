cmake_minimum_required(VERSION 3.0)
project(interceptkb)
add_executable(interceptkb main.c)

if(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    message(FATAL_ERROR "Only Linux is supported.")
endif(NOT ${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if(CMAKE_COMPILER_IS_GNUCC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Werror -pedantic -Wall \
        -Wextra -Wcast-align -Wcast-qual -Wdisabled-optimization -Wformat=2 \
        -Winit-self -Wlogical-op -Wmissing-declarations \
        -Wmissing-include-dirs -Wredundant-decls -Wshadow -Wsign-conversion \
        -Wstrict-overflow=5 -Wswitch-default -Wundef")
    set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -fwhole-program \
        -funroll-loops")
endif()
