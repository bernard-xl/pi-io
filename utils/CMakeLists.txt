project(pi)
cmake_minimum_required(VERSION 2.8)

include_directories(${CMAKE_SOURCE_DIR})

add_library(utils storage.c)

#add_executable(storage-test storage-test.c)
#add_executable(storage-perf storage-perf.c)
#target_link_libraries(storage-test utils)

