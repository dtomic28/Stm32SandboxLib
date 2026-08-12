add_executable(test_can_database
    "test_can_codec.cpp"
)

target_link_libraries(test_can_database PRIVATE
    ${CANDB_TARGET}
    GTest::gtest
    GTest::gtest_main
)

target_compile_features(test_can_database PRIVATE cxx_std_14)

add_test(NAME test_can_database COMMAND test_can_database)
