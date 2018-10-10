cmake_minimum_required(VERSION 3.12)
project(serial)

set(CMAKE_CXX_STANDARD 14)

add_executable(serial
        Node.cc
        ../../include/Node.h
        BinarySearchTree.cc
        ../../include/BinarySearchTree.h
        SerialUtils.cc
        ../../include/SerialUtils.h)

