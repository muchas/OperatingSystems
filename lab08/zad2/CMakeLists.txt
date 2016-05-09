cmake_minimum_required(VERSION 2.6)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
set(ENV{LD_LIBRARY_PATH} ${CMAKE_BINARY_DIR}/lib)

enable_testing()

add_executable (main_a_1 main_a_1.c)
add_executable (main_a_2 main_a_2.c)
add_executable (main_a_3 main_a_3.c)
add_executable (main_a_4 main_a_4.c)
add_executable (main_a_5 main_a_5.c)
add_executable (main_b main_b.c)


target_link_libraries (main_a_1 LINK_PUBLIC pthread)
target_link_libraries (main_a_2 LINK_PUBLIC pthread)
target_link_libraries (main_a_3 LINK_PUBLIC pthread)
target_link_libraries (main_a_4 LINK_PUBLIC pthread)
target_link_libraries (main_a_5 LINK_PUBLIC pthread)
target_link_libraries (main_b LINK_PUBLIC pthread)
