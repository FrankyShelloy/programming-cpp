cmake_minimum_required(VERSION 3.10)

project(lab0)

set(CMAKE_CXX_STANDARD 17)
include_directories(include)

add_executable(lab0
        src/main.cpp
        src/text_processor.cpp
        src/word_counter.cpp
        src/file_handler.cpp
)