cmake_minimum_required(VERSION 3.5)
project(shell_program)

set(CMAKE_CXX_STANDARD 11)

add_executable(shell_program main.cpp redirect.cpp redirect.h string_util.cpp string_util.h command.cpp command.h)