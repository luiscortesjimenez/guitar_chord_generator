# ChordArchitect

A C++ guitar chord generator that models a fretboard, computes chord voicings and stores them in a custom hash table for fast lookup. Given a chord name like "Am", "G7", "Dmaj7", ChordArchitect finds playable fingerings across the neck and presents them to the user.

## Features

- **Fretboard modeling** — Represents a standard 6 string guitar with configurable tuning and fret count. NOTE: String count configuration has been added.
- **Chord voicing generation** — Generates valid fingerings for any given chord. Takes into account physical playability constraints such as stretch, muted strings and barre chords.
- **Hash-table storage** — Stores and retrieves chord voicings efficiently using a purpose built hash table.
- **Command-line interface** — Query chords interactively or via arguments.

## Project Structure

```
ChordArchitect/
├── CMakeLists.txt
├── README.md
├── docs/
├── include/
│   └── ChordEngine/
│       ├── Fretboard.h
│       ├── Chord.h
│       └── ChordHashTable.h
├── src/
│   ├── main.cpp
│   ├── Fretboard.cpp
│   ├── Chord.cpp
│   └── ChordHashTable.cpp
└── tests/
    └── main_tests.cpp
```

## Architecture Overview

ChordArchitect is organized around three core modules under the `ChordEngine` namespace:

### Fretboard (implemented)

Models the guitar neck as a 2D grid of strings and frets. Built around two supporting types:

- **`enum class Note`** — The 12 chromatic pitch classes (C through B) with semitone arithmetic via `operator+`
- **`struct Pitch`** — Pairs a `Note` with an octave (e.g. E2, A4) and supports MIDI conversion for easy comparison

The `Fretboard` class itself provides:

- Configurable tuning and fret count (any number of strings, any number of frets)
- Factory methods for common tunings: `standard()` (EADGBE) and `dropD()` (DADGBE)
- `getPitchAt(string, fret)` / `getNoteAt(string, fret)` for looking up any position on the neck
- `findNotePositions(Note)` to locate every occurrence of a given note across all strings and frets
- `printGrid()` for a formatted fretboard chart

### Chord

Defines chords in terms of a root note and a set of intervals (e.g. major = root, major third, perfect fifth). Responsible for:

- Translating a chord symbol (like "Cmaj7") into its constituent notes
- Generating candidate voicings on a given `Fretboard`
- Filtering voicings by playability (max fret span, minimum strings sounded)

### ChordHashTable

A custom hash table that maps chord names to their computed voicings. Responsible for:

- Caching generated voicings so repeated lookups are O(1)
- Handling collisions and resizing
- Providing iteration over all stored chords

## Roadmap

- [x] Project scaffold and build system
- [x] Implement `Fretboard` (tuning, note mapping)
- [x] Implement `Chord` (interval logic, voicing generation)
- [x] Implement `ChordHashTable` (storage and retrieval)
- [x] CLI for querying chords
- [x] Unit tests
- [x] Documentation in `docs/`

## Documentation

Full documentation lives in the [`docs/`](docs/) directory:

- [Architecture](docs/architecture.md) — system design, module relationships, data flow
- [CLI Usage](docs/cli-usage.md) — command-line reference with examples
- [API: Fretboard](docs/api-fretboard.md) — Note, Pitch, and Fretboard reference
- [API: Chord](docs/api-chord.md) — Voicing, PlayabilityConfig, and Chord reference
- [API: ChordHashTable](docs/api-chordhashtable.md) — hash table and iterator reference
- [Music Theory Primer](docs/music-theory.md) — background on notes, intervals, chords, and voicings
- [Building](docs/building.md) — prerequisites, build steps, and running tests
