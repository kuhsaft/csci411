cmake_minimum_required(VERSION 3.5)
project(producer_consumer)

set(CMAKE_CXX_STANDARD 11)

add_executable(producer_consumer main.cpp RingBuffer.h ProducerConsumer.h)
target_link_libraries(producer_consumer pthread)
