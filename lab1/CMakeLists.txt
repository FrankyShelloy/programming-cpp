cmake_minimum_required(VERSION 3.10)
project(lab1)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)

find_package(GTest REQUIRED)

add_executable(lab1_tests
        tests/test_priority_queue.cpp
        src/PriorityQueue.cpp
)

target_link_libraries(lab1_tests GTest::gtest GTest::gtest_main)