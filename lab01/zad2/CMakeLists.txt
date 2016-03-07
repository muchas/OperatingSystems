
cmake_minimum_required(VERSION 2.6)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Release)
set(ENV{LD_LIBRARY_PATH} ${CMAKE_BINARY_DIR}/lib)

enable_testing()

include_directories(include)

add_subdirectory(lib)
add_subdirectory(src)


target_include_directories (contacts PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_test(run_main main)
add_test(run_main_static main_static)
add_test(run_main_dynamic main_dynamic)
