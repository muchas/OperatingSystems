cmake_minimum_required(VERSION 2.6)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
set(ENV{LD_LIBRARY_PATH} ${CMAKE_BINARY_DIR}/lib)

enable_testing()

add_executable (main_fork main_fork.c)
add_executable (main_vfork main_vfork.c)
add_executable (main_clone_fork main_clone_fork.c)
add_executable (main_clone_vfork main_clone_vfork.c)