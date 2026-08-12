set(SRC_Include
    "../include/can_database/can_frame.h"
    "../include/can_database/can_ids.h"
    "../include/can_database/can_messages.h"
    "../include/can_database/can_codec.h"
    "../include/can_database/can_database.h"
)

set(SRC_Srcs
    "can_codec.c"
    "can_database.c"
)

add_library(${PROJECT_NAME} STATIC
    ${SRC_Include}
    ${SRC_Srcs}
)

add_library(tomko::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PRIVATE c_std_11)

target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/../include>
)
