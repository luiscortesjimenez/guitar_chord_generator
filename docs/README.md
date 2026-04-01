# ChordArchitect Documentation

ChordArchitect is a C++ guitar chord generator that models a fretboard, computes playable voicings for any chord and stores them in a custom hash table for fast lookup.

## Contents

| Document | Description |
|---|---|
| [Architecture](architecture.md) | System design, module relationships, and data flow |
| [CLI Usage](cli-usage.md) | Command-line interface reference with examples |
| [API: Fretboard](api-fretboard.md) | `Note`, `Pitch`, and `Fretboard` class reference |
| [API: Chord](api-chord.md) | `Voicing`, `PlayabilityConfig`, and `Chord` class reference |
| [API: ChordHashTable](api-chordhashtable.md) | `ChordHashTable`, `ChordEntry`, and `Iterator` reference |
| [Music Theory Primer](music-theory.md) | Background on notes, intervals, chords, and voicings |
| [Building](building.md) | Prerequisites, build steps, and running tests |

## Quick Start

```bash
cmake -B build
cmake --build build
./build/guitar_chord_generator Am G7 Cmaj7
```

```
Am (389 voicings, showing first 10):
  1. x x 7 5 5 5
  2. x x 7 5 5 8
  ...
```

See [CLI Usage](cli-usage.md) for the full set of options, or [Building](building.md) if you need help getting set up.
