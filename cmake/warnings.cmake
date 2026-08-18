# tomko_enable_warnings(<target>)
#
# Enables -Wall -Wextra (GCC/Clang) or /W4 (MSVC) on <target>, and
# additionally treats warnings as errors when TOMKO_WARNINGS_AS_ERRORS is
# ON (set in the "ci" CMake preset — see CMakePresets.json). Warnings stay
# non-fatal for ordinary local dev builds so this doesn't suddenly break
# someone's incremental build; CI is where it's enforced as a real gate.
#
# Call this per-target, not project-wide via add_compile_options() — a
# blanket directory-scoped flag would also apply to FetchContent'd
# external dependencies (GTest) that we don't control and shouldn't be
# held to our own warning bar.
function(tomko_enable_warnings target)
    if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang" OR CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra)
        if(TOMKO_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    elseif(CMAKE_C_COMPILER_ID STREQUAL "MSVC" OR CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        target_compile_options(${target} PRIVATE /W4)
        if(TOMKO_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    endif()
endfunction()
