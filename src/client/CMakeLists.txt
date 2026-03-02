aux_source_directory(. src_list)

add_executable(ChatClient ${src_list})
target_link_libraries(ChatClient pthread)