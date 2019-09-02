cmake_minimum_required(VERSION 3.10)

project(thread_pool)

if (NOT WIN32)
        message(FATAL_ERROR "This project is supported only on Windows!")
endif (NOT WIN32)

set(CMAKE_CXX_STANDARD 11)

include_directories(include)

add_executable(${PROJECT_NAME} 
        include/winapi_thread_pool.h
        src/winapi_thread_pool.cpp
        src/main.cpp
        )
