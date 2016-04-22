
add_library(utils SHARED utils.c)

add_executable (host host.c)
add_executable (consumer consumer.c)
add_executable (producer producer.c)

target_link_libraries (host LINK_PUBLIC utils)
target_link_libraries (consumer LINK_PUBLIC utils)
target_link_libraries (producer LINK_PUBLIC utils)
