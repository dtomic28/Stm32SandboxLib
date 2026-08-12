include(FetchContent)

macro(tomko_fetch_dependency NAME)
    FetchContent_Declare(
        ${NAME}
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/External/${NAME}
        ${ARGN}
    )
    FetchContent_MakeAvailable(${NAME})
endmacro()
