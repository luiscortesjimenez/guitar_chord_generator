# Building ChordArchitect

## Prerequisites

- A C++20-compatible compiler (GCC 10+, Clang 12+, Apple Clang 14+, MSVC 2019+)
- CMake 3.14 or later
- Git (for CMake's FetchContent to download Catch2)

## Clone

```bash
git clone <repository-url>
cd guitar_chord_generator
```

## Configure and Build

```bash
cmake -B build
cmake --build build
```

This builds two targets:

| Target | Path | Description |
|---|---|---|
| `guitar_chord_generator` | `build/guitar_chord_generator` | The main CLI executable |
| `tests` | `build/tests` | Catch2 unit test executable |

### Release build

For optimized output:

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Rebuild after changes

CMake tracks file dependencies, so after editing source files you only need:

```bash
cmake --build build
```

## Running the CLI

```bash
# Argument mode
./build/guitar_chord_generator Am G7 Cmaj7

# Interactive mode
./build/guitar_chord_generator

# With options
./build/guitar_chord_generator --tuning dropd --max-voicings 5 Em
```

See [CLI Usage](cli-usage.md) for the full reference.

## Running Tests

```bash
./build/tests
```

Expected output:

```
===============================================================================
All tests passed (2043 assertions in 25 test cases)
```

### Running specific test groups

Catch2 supports tag-based filtering:

```bash
./build/tests [fretboard]      # Fretboard tests only
./build/tests [chord]          # Chord tests only
./build/tests [hashtable]      # ChordHashTable tests only
```

### Verbose output

```bash
./build/tests --success        # Show passing assertions too
```

## Dependencies

**Catch2** (v3.5.2) is the only external dependency. It is fetched automatically by CMake via `FetchContent` during the configure step — no manual installation needed. The first build will take longer as it downloads and compiles Catch2; subsequent builds reuse the cached version.

## Project Structure

```
guitar_chord_generator/
├── CMakeLists.txt              Build configuration
├── README.md                   Project overview
├── docs/                       Documentation (you are here)
├── include/ChordEngine/        Public headers
│   ├── Fretboard.h
│   ├── Chord.h
│   └── ChordHashTable.h
├── src/                        Implementation files
│   ├── main.cpp                CLI entry point
│   ├── Fretboard.cpp
│   ├── Chord.cpp
│   └── ChordHashTable.cpp
└── tests/
    └── main_tests.cpp          Catch2 unit tests
```
