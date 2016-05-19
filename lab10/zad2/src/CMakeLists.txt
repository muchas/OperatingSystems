add_executable (server server.c)

target_link_libraries(server LINK_PUBLIC pthread)
