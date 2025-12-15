cmake_minimum_required(VERSION 3.16)
project(polyclean LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(polyclean
    src/mesh.cpp
    src/main.cpp
    )

target_include_directories(polyclean PRIVATE ${CMAKE_SOURCE_DIR})