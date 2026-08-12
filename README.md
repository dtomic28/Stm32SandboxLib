# tomko_lib

A sandbox and reference implementation for a shared embedded C library. The goal is to establish a clean, modular CMake architecture that can be reused across multiple embedded projects — common drivers, protocols, and utilities live here and get pulled into target projects via CMake FetchContent.

## Structure

```
tomko_lib/
├── cmake/
│   └── fetch_dependency.cmake   # tomko_fetch_dependency() helper
├── external/
│   └── gtest/                   # GTest FetchContent recipe
├── can_database/                 # CAN message codec and runtime dispatcher
│   ├── include/can_database/
│   ├── src/
│   ├── example/
│   └── test/
└── CMakeLists.txt
```

Each module is a self-contained subdirectory with its own `CMakeLists.txt`, exposing a `tomko::<Name>` CMake target. Projects that consume this lib link against those targets directly.

## Building

Requires CMake 3.25+ and Ninja (Linux) or Visual Studio 2026 (Windows).

```bash
# Configure
cmake --preset ci        # all sublibs + tests + examples enabled
cmake --preset debug     # bare debug build
cmake --preset release   # bare release build

# Build
cmake --build --preset ci

# Test
ctest --preset ci
```

Binaries land in `build/<preset>/bin/`, libraries in `build/<preset>/lib/`.

## Adding a new sublib

1. Create a directory: `my_module/`
2. Add `my_module/CMakeLists.txt` with `project(MyModule)` and `add_subdirectory(src)`
3. Add `my_module/src/CMakeLists.txt` defining the library target and `tomko::MyModule` alias
4. In the root `CMakeLists.txt`, add:
   ```cmake
   set(TOMKO_ENABLE_MY_MODULE OFF CACHE BOOL "Build MyModule")
   if(TOMKO_ENABLE_MY_MODULE)
       add_subdirectory(my_module)
   endif()
   ```
5. Enable it in the `ci` preset in `CMakePresets.json`

## Adding a new CAN message

1. Define the ID and bus in `can_database/include/can_database/can_ids.h`
2. Add the packed struct + `static_assert` to `can_messages.h`
3. Add encode/decode declarations to `can_codec.h` and implement in `can_codec.c`
4. Add a `_Publish` function to `can_database.h` and implement in `can_database.c`
5. Add roundtrip and edge case tests to `test/test_can_codec.cpp`
