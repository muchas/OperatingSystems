cmake_minimum_required(VERSION 2.6)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
set(ENV{LD_LIBRARY_PATH} ${CMAKE_BINARY_DIR}/lib)

include_directories(include)

enable_testing()

add_executable (catcher_a catcher_a.c)
add_executable (sender_a sender_a.c)

add_executable (catcher_b catcher_b.c)
add_executable (sender_b sender_b.c)

add_executable (catcher_c catcher_c.c)
add_executable (sender_c sender_c.c)
