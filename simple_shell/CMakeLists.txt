cmake_minimum_required(VERSION 3.5)
project(simple_shell)

set(CMAKE_CXX_STANDARD 11)

add_executable(simple_shell main.cpp redirect.cpp redirect.h string_util.cpp string_util.h command.cpp command.h)