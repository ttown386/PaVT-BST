cmake_minimum_required(VERSION 3.12)
project(serial)

set(CMAKE_CXX_STANDARD 14)

add_executable(serial AVLTree.cc ../../include/AVLTree.h Node.cc ../../include/Node.h InternalBST.cc ../../include/InternalBST.h ExternalBST.cc ../../include/ExternalBST.h)

