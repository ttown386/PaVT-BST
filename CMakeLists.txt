cmake_minimum_required(VERSION 3.12)
project(ConcurrentTreeTraverals)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(src/serial)

add_executable(ConcurrentTreeTraverals main.cpp)