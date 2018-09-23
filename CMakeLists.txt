cmake_minimum_required(VERSION 3.12)
project(ConcurrentTreeTraverals)

set(CMAKE_CXX_STANDARD 14)

add_subdirectory(src/serial)
add_subdirectory(lib/googletest-master)

include_directories(lib/googletest-master/googletest/include)
include_directories(lib/googletest-master/googlemock/include)
include_directories(include)
include_directories(test)

add_executable(ConcurrentTreeTraverals main.cpp src/serial/Node.cc test/TestNode.cpp)

target_link_libraries(ConcurrentTreeTraverals gtest gtest_main)