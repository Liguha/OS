cmake_minimum_required(VERSION 3.24.2)

project(LR4)
find_package(Threads)
add_executable(a.out src/file_mapping.c)
target_link_libraries(a.out ${CMAKE_THREAD_LIBS_INIT})
