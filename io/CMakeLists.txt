project(pi)
cmake_minimum_required(VERSION 2.8)

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake-modules)

find_package (Threads)
find_package (BlueZ)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/utils)

add_library(io loop.c handle.c operation.c file.c tcp.c rfcomm.c serial.c inotify.c)

#add_executable(io-tcp-test io-tcp-test.c)
#add_executable(io-splice-test io-splice-test.c)
#add_executable(io-rfcomm-test io-rfcomm-test.c)
#add_executable(reactive mdp.c)
#add_executable(macro-test macro-test.c)

target_link_libraries(io ${CMAKE_THREAD_LIBS_INIT} ${BLUEZ_LIBRARIES})
#target_link_libraries(io-tcp-test io) 
#target_link_libraries(io-splice-test io)
#target_link_libraries(io-rfcomm-test io)
#target_link_libraries(reactive io)

