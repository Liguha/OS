cmake_minimum_required(VERSION 3.24.2)

project(LR5)
include_directories(include)

if (NOT (DEFINED LIB_PRINT_TYPE))
	set(LIB_PRINT_TYPE DEFAULT)
endif ()

if (LIB_PRINT_TYPE STREQUAL EXTENDED)
	add_compile_definitions(SYSTEM="${CMAKE_SYSTEM}")
elseif (NOT (LIB_PRINT_TYPE STREQUAL DEFAULT))
	message(FATAL_ERROR "Unexpected LIB_PRINT_TYPE: ${LIB_PRINT_TYPE}")
endif ()

add_library(dynamic1 SHARED src/functions1.c)
add_library(dynamic2 SHARED src/functions2.c)

add_executable(program11.out src/program1.c include/functions.h)
add_executable(program12.out src/program1.c include/functions.h)
target_link_libraries(program11.out dynamic1)
target_link_libraries(program12.out dynamic2)

add_executable(program2.out src/program2.c)
target_link_libraries(program2.out ${CMAKE_DL_LIBS})
