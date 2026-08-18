find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(CANDB_YAML        ${CMAKE_CURRENT_SOURCE_DIR}/../can_messages.yaml)
set(CANDB_GEN_SCRIPT   ${CMAKE_CURRENT_SOURCE_DIR}/../tools/gen_can_database.py)
set(CANDB_CAN_FRAME_H  ${CMAKE_CURRENT_SOURCE_DIR}/../include/can_database/can_frame.h)
set(CANDB_GEN_INCLUDE_DIR ${CMAKE_CURRENT_BINARY_DIR}/../include)
set(CANDB_GEN_SRC_DIR     ${CMAKE_CURRENT_BINARY_DIR}/generated)
set(CANDB_DBC_OUT_DIR     ${CMAKE_CURRENT_BINARY_DIR}/dbc)

execute_process(
    COMMAND ${Python3_EXECUTABLE} -c "import yaml"
    RESULT_VARIABLE CANDB_PYYAML_CHECK
    OUTPUT_QUIET ERROR_QUIET
)
if(NOT CANDB_PYYAML_CHECK EQUAL 0)
    message(FATAL_ERROR
        "can_database code generation requires PyYAML. Install it with:\n"
        "    ${Python3_EXECUTABLE} -m pip install pyyaml")
endif()

set(CANDB_GEN_ARGS
    --yaml ${CANDB_YAML}
    --include-out ${CANDB_GEN_INCLUDE_DIR}
    --src-out ${CANDB_GEN_SRC_DIR}
    --dbc-out-dir ${CANDB_DBC_OUT_DIR}
    --can-frame-h ${CANDB_CAN_FRAME_H}
)

# Run once now so generated headers exist immediately — IDE/IntelliSense
# sees them right after configure, no build required first. This also
# tells us which .dbc files actually get written (one per bus known to
# can_frame.h) so add_custom_command below can declare them as OUTPUTs
# without CMake needing to hardcode the bus list itself.
execute_process(
    COMMAND ${Python3_EXECUTABLE} ${CANDB_GEN_SCRIPT} ${CANDB_GEN_ARGS}
    RESULT_VARIABLE CANDB_GEN_RESULT
)
if(NOT CANDB_GEN_RESULT EQUAL 0)
    message(FATAL_ERROR "can_database code generation failed — see output above")
endif()

set(CANDB_GENERATED_HEADERS
    ${CANDB_GEN_INCLUDE_DIR}/can_database/can_messages.h
    ${CANDB_GEN_INCLUDE_DIR}/can_database/can_ids.h
    ${CANDB_GEN_INCLUDE_DIR}/can_database/can_codec.h
    ${CANDB_GEN_INCLUDE_DIR}/can_database/can_publish.h
)
set(CANDB_GENERATED_SRCS
    ${CANDB_GEN_SRC_DIR}/can_codec.c
    ${CANDB_GEN_SRC_DIR}/can_publish.c
)
file(GLOB CANDB_GENERATED_DBCS ${CANDB_DBC_OUT_DIR}/*.dbc)

# Re-run at build time whenever can_messages.yaml, can_frame.h, or the
# generator itself changes — edit the YAML, hit Build, no manual
# reconfigure needed.
add_custom_command(
    OUTPUT ${CANDB_GENERATED_HEADERS} ${CANDB_GENERATED_SRCS} ${CANDB_GENERATED_DBCS}
    COMMAND ${Python3_EXECUTABLE} ${CANDB_GEN_SCRIPT} ${CANDB_GEN_ARGS}
    DEPENDS ${CANDB_YAML} ${CANDB_GEN_SCRIPT} ${CANDB_CAN_FRAME_H}
    COMMENT "Generating can_database from can_messages.yaml"
    VERBATIM
)

set(SRC_Include
    "../include/can_database/can_frame.h"
    "../include/can_database/can_bits.h"
    "../include/can_database/can_database.h"
    ${CANDB_GENERATED_HEADERS}
)

set(SRC_Srcs
    "can_bits.c"
    "can_database.c"
    "can_database_internal.h"
    ${CANDB_GENERATED_SRCS}
)

add_library(${PROJECT_NAME} STATIC
    ${SRC_Include}
    ${SRC_Srcs}
)

add_library(tomko::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PRIVATE c_std_11)
tomko_enable_warnings(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>
    PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# math.h's lround/lroundf (used by generated scaled signal codecs)
find_library(CANDB_MATH_LIB m)
if(CANDB_MATH_LIB)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CANDB_MATH_LIB})
endif()
