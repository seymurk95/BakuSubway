cmake_minimum_required(VERSION 3.10)
project(BakuSubway)

option(BUILD_SHARED_LIBS "build using shared libraries" ON)

set(CMAKE_CXX_STANDARD 20)

add_library(BakuSubway BakuSubway.cpp)
target_include_directories(BakuSubway PUBLIC ${CMAKE_SOURCE_DIR})

add_executable(BakuSubway main.cpp BakuSubway.cpp)

target_link_libraries(main PRIVATE BakuSubway)