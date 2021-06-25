cmake_minimum_required(VERSION 3.5)

project(HSS LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

add_executable(HSS main.cpp)

target_link_libraries(HSS ${SDL2_LIBRARIES})
