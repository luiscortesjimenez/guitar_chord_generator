cmake_minimum_required(VERSION 3.14)
project(ChordArchitect VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

# --- Main executable ---
file(GLOB SOURCES "src/*.cpp")
add_executable(guitar_chord_generator ${SOURCES})

# --- Engine library (shared between main and tests) ---
add_library(chord_engine_lib
    src/Fretboard.cpp
    src/Chord.cpp
    src/ChordHashTable.cpp
)
target_include_directories(chord_engine_lib PUBLIC include)

# --- Catch2 test framework ---
include(FetchContent)
FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.5.2
)
FetchContent_MakeAvailable(Catch2)

# --- Test executable ---
add_executable(tests tests/main_tests.cpp)
target_link_libraries(tests PRIVATE chord_engine_lib Catch2::Catch2WithMain)
