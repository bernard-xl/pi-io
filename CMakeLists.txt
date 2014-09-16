project(pi)
cmake_minimum_required(VERSION 2.8)

#set(CMAKE_C_FLAGS "-O3")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake-modules)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/utils)
include_directories(${CMAKE_SOURCE_DIR}/io)

add_subdirectory(io)
add_subdirectory(utils)

add_executable(reactive reactive.c)
target_link_libraries(reactive io utils)
