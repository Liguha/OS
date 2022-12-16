cmake_minimum_required(VERSION 3.24.2)

project(LR6-8)
find_package(Threads)

add_executable(client src/distribution_node.cpp)
target_link_libraries(client ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries(client zmq)

add_executable(server src/computing_node.cpp)
target_link_libraries(server zmq)
