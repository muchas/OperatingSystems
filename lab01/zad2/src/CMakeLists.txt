
add_executable (main main.c)
add_executable (main_static main.c)
add_executable (main_dynamic main_dynamic.c)

target_link_libraries (main LINK_PUBLIC contacts)
target_link_libraries (main_static LINK_PUBLIC contacts_static)
target_link_libraries (main_dynamic LINK_PUBLIC dl)