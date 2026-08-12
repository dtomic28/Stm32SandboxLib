project(CanDatabase)

set(CANDB_TARGET tomko::${PROJECT_NAME})

add_subdirectory(src)

if(TOMKO_LIB_ENABLE_EXAMPLE)
    add_subdirectory(example)
endif()

if(TOMKO_LIB_ENABLE_TESTS)
    add_subdirectory(test)
endif()
