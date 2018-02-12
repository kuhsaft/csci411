cmake_minimum_required(VERSION 3.5)
project(cooperating_processes)

set(CMAKE_CXX_STANDARD 11)

add_executable(client client.cpp messages.h)
target_link_libraries(client rt)

add_executable(server server.cpp messages.h)
target_link_libraries(server rt)
