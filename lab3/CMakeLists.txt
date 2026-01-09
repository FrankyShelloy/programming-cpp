cmake_minimum_required(VERSION 3.14)
project(flat_hash_map_lab)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_compile_options(-msse2 -Wall -Wextra -Wpedantic)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(flat_hash_map_tests tests.cpp)
target_link_libraries(flat_hash_map_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(flat_hash_map_tests)
