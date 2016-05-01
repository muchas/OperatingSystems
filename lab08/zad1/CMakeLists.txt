cmake_minimum_required(VERSION 2.6)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
set(ENV{LD_LIBRARY_PATH} ${CMAKE_BINARY_DIR}/lib)

enable_testing()

add_executable (main_async main_async.c)
add_executable (main_sync main_sync.c)
add_executable (main_detached main_detached.c)
add_executable (generator generator.c)

target_link_libraries (main_async LINK_PUBLIC pthread)
target_link_libraries (main_sync LINK_PUBLIC pthread)
target_link_libraries (main_detached LINK_PUBLIC pthread)
