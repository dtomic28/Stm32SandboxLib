cmake_minimum_required(VERSION 3.25)

project(tomko_lib C)

include(cmake/fetch_dependency.cmake)

# --- Output directories ---
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# --- Options ---
set(TOMKO_LIB_ENABLE_TESTS   OFF CACHE BOOL "Enable tests for all sublibs")
set(TOMKO_LIB_ENABLE_EXAMPLE OFF CACHE BOOL "Build examples for all sublibs")

# --- External dependencies ---
if(TOMKO_LIB_ENABLE_TESTS)
    add_subdirectory(external/gtest)
    enable_testing()
endif()

# --- Sublibs ---
set(TOMKO_ENABLE_CAN_DATABASE OFF CACHE BOOL "Build CanDatabase sublib")

if(TOMKO_ENABLE_CAN_DATABASE)
    add_subdirectory(can_database)
endif()
