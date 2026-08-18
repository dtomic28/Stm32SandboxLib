add_executable(test_can_database
    "test_can_codec.cpp"
    "test_can_bits.cpp"
)

target_link_libraries(test_can_database PRIVATE
    ${CANDB_TARGET}
    GTest::gtest
    GTest::gtest_main
)

target_compile_features(test_can_database PRIVATE cxx_std_14)

add_test(NAME test_can_database COMMAND test_can_database)

# Generator-level tests (schema validation + golden-file codegen check).
# Pure Python, no compiler involved -- see tools/test_gen_can_database.py.
find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_test(
    NAME test_gen_can_database
    COMMAND ${Python3_EXECUTABLE} -m unittest discover
            -s "${CMAKE_CURRENT_SOURCE_DIR}/../tools" -p "test_*.py" -v
)
