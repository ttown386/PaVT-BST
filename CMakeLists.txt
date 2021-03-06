cmake_minimum_required(VERSION 3.1.3)

# Set project name, version and laguages here.
# Version numbers are available by including "BingoCpp/version.h" in 
# the source. See version.h.in for some more details.
project(ttree VERSION 0.1.1.0 LANGUAGES C CXX)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-std=c++11 -pthread")

# ---------- version ----------
configure_file (
  "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/version.h.in"
  "${PROJECT_SOURCE_DIR}/include/${PROJECT_NAME}/version.h")

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
include(ConfigSafeGuards)
include(Colors)


# ------------------------------------------------------------------------------
#                          Compile flags
# ------------------------------------------------------------------------------
# Set the C++ standard you wish to use (will apply to all files).
# If you do not use any features that limits the standard required,
# you could omit this line.
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")  
SET(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# ------------------------------------------------------------------------------
#                         Locate files (no change needed).
# ------------------------------------------------------------------------------
# We make sure that CMake sees all the files.
include_directories(${PROJECT_SOURCE_DIR}/include)

# Make variables referring to all the sources and test files.
file(GLOB SRC_FILES 
    "src/base/*.c*"
    "src/sequential/*.c*"
    "src/pavt/*.c*")
file(GLOB TEST_FILES "tests/*.cpp")
list(REMOVE_ITEM TEST_FILES ${PROJECT_SOURCE_DIR}/tests/performance_tests.cpp)

add_library(pavt_lib ${SRC_FILES})

# ------------------------------------------------------------------------------
#                        Performance Benchmarks 
# ------------------------------------------------------------------------------
find_package(Threads)
add_executable(performance_benchmarks ${PROJECT_SOURCE_DIR}/tests/performance_tests.cpp)
target_link_libraries(performance_benchmarks pavt_lib)
target_link_libraries(performance_benchmarks ${CMAKE_THREAD_LIB_INIT})

find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
# Update submodules as needed
  option(GIT_SUBMODULE "Check submodules during build" ON)
  if(GIT_SUBMODULE)
    message(STATUS "Submodule update")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
      message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
    endif()
  endif()
endif()

if(NOT EXISTS "${PROJECT_SOURCE_DIR}/external/googletest/CMakeLists.txt")
  message(FATAL_ERROR "The submodules were not downloaded! GIT_SUBMODULE was turned off or failed. Please update submodules and try again.")
endif()

add_subdirectory(${PROJECT_SOURCE_DIR}/external/googletest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

set(TEST_MAIN unit_tests.x)
add_executable(${TEST_MAIN} ${TEST_FILES})
target_link_libraries(${TEST_MAIN} gtest gmock gtest_main)
target_link_libraries(${TEST_MAIN} pavt_lib)

# ------------------------------------------------------------------------------
#                         Code Coverage
# ------------------------------------------------------------------------------
# If CMake aborts due to missing dependencies for code coverage 
# (gcov, lcov, genhtml and supported compiler), comment this line.
include(CodeCoverage)
