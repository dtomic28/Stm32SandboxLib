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
│   ├── can_messages.yaml         # single source of truth for every message
│   ├── tools/gen_can_database.py # generates structs/codec/publish API/.dbc from it
│   ├── include/can_database/     # hand-written: can_frame.h, can_bits.h, can_database.h
│   ├── src/                      # hand-written: can_bits.c, can_database.c
│   ├── example/
│   └── test/
└── CMakeLists.txt
```

Each module is a self-contained subdirectory with its own `CMakeLists.txt`, exposing a `tomko::<Name>` CMake target. Projects that consume this lib link against those targets directly.

## Building

Requires CMake 3.25+, Ninja (Linux) or Visual Studio 2026 (Windows), and
Python 3 with PyYAML (`pip install pyyaml`) — used at configure and build
time to generate `can_database`'s structs, codec, publish API, and `.dbc`
file from `can_messages.yaml`.

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

Either edit `can_database/can_messages.yaml` by hand, or run the GUI editor:

```bash
py can_database/tools/can_message_editor.py
```

It's a grid: messages on top (add/remove rows, double-click a cell to
edit — Bus and signal Type/Wire Type are dropdowns sourced from
`can_frame.h` and the generator's known types, so you can't typo them),
signals for the selected message below. "Save && Validate" writes the
YAML back only if it passes the exact same schema checks the generator
uses — it does not generate C code itself, that's still CMake's job.

Either way:

1. Add a message entry to `can_database/can_messages.yaml` — name, id, bus,
   dlc, and its signals (name, type, optional `wire_type` for a
   scaled/physical firmware type, bits, scale, offset, unit).
2. Reconfigure or rebuild — `CanMsg_<Name>_t`, `CanCodec_<Name>_Encode`/
   `_Decode`, `CanDb_<Name>_Publish`, `CAN_ID_<NAME>`/`CAN_BUS_<NAME>`, and
   a `.dbc` file per bus all regenerate from the YAML automatically
   (`can_database/tools/gen_can_database.py`, wired into
   `can_database/src/CMakeLists.txt`). Do not hand-edit any generated file.

You don't need to add tests for the new message. Every message goes
through the same handful of code-generation shapes (byte-aligned store,
bit-packed, sign-extended, scaled), each already covered once in
`test/test_can_codec.cpp` and `test/test_can_bits.cpp`, plus the
generator's own schema validation and a golden-file codegen check in
`tools/test_gen_can_database.py` (`py -m unittest discover -s
can_database/tools -p "test_*.py"`, also wired into `ctest`). Only add a
new test if your message needs a genuinely new shape those don't cover.

The generator writes one `.dbc` file per bus — `build/<preset>/can_database/dbc/can_database_<bus>.dbc`
(e.g. `can_database_sensor.dbc`), one per `CAN_BUS_*` name found in
`can_frame.h` — rather than a single file mixing every bus. That matches
how real CAN tooling (Vector CANoe/CANalyzer, `cantools`) actually loads
databases: per physical bus/channel, since two different buses are free
to reuse the same CAN ID. Each file loads directly with Python's
`cantools` (`cantools.database.load_file(...)`) — no custom bindings
needed to decode messages on a PC.
