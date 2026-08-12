add_executable(can_database_example "example.c")

target_link_libraries(can_database_example PRIVATE ${CANDB_TARGET})

target_compile_features(can_database_example PRIVATE c_std_11)
