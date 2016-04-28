add_library(utils SHARED utils.c)

add_executable (host host.c)
add_executable (reader reader.c)
add_executable (writer writer.c)

target_link_libraries(host LINK_PUBLIC utils rt pthread)
target_link_libraries (reader LINK_PUBLIC utils rt pthread)
target_link_libraries (writer LINK_PUBLIC utils rt pthread)
