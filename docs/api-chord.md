# API Reference: Chord

Header: `include/ChordEngine/Chord.h`
Implementation: `src/Chord.cpp`
Namespace: `ChordEngine`

---

## `using Voicing = std::vector<int>`

A voicing is one way to play a chord on the guitar. Each element corresponds to a string (index 0 = lowest-pitched string). The value is a fret number, or `-1` if the string is muted.

```cpp
Voicing classicAm = {-1, 0, 2, 2, 1, 0};
// x 0 2 2 1 0 — the common open Am shape
```

---

## `struct PlayabilityConfig`

Controls which generated voicings are kept. All fields have defaults matching typical human hand constraints.

| Field | Type | Default | Description |
|---|---|---|---|
| `maxFretSpan` | `int` | `4` | Maximum distance between lowest and highest fretted notes. Open strings (fret 0) don't count toward span. |
| `minStringsSounded` | `int` | `4` | Minimum number of strings that must be sounded (not muted). |
| `requireRootInBass` | `bool` | `true` | The lowest sounded note must be the chord's root note. |

```cpp
PlayabilityConfig config;
config.maxFretSpan = 3;         // tighter fingering
config.requireRootInBass = false; // allow inversions
```

---

## `class Chord`

Represents a chord as a root note plus a quality suffix. Knows how to parse chord symbols, compute constituent notes from an interval formula, and generate playable voicings on a `Fretboard`.

### Constructor

```cpp
Chord(Note root, const std::string& quality);
```

Constructs a chord from an explicit root note and quality string. The quality must exist in the formula registry (see table below).

**Throws:** `std::invalid_argument` if the quality is unknown.

```cpp
Chord amChord(Note::A, "m");      // A minor
Chord cMajor(Note::C, "");        // C major (empty string = major)
```

### `static fromSymbol(const std::string& symbol) -> Chord`

Parses a chord symbol string into a `Chord` object. Extracts the root note (letter + optional `#`) and treats the remainder as the quality suffix.

**Throws:** `std::invalid_argument` if the root note letter is invalid or the quality suffix is unrecognized.

```cpp
auto chord = Chord::fromSymbol("F#m7");
// root = Note::Fs, quality = "m7"
```

### `getRoot() -> Note`

Returns the root note.

### `getQuality() -> const std::string&`

Returns the quality suffix string (e.g. `"m"`, `"7"`, `"maj7"`, `""` for major).

### `getIntervals() -> const std::vector<int>&`

Returns the semitone interval vector from the formula registry.

```cpp
auto chord = Chord::fromSymbol("Am");
auto intervals = chord.getIntervals(); // {0, 3, 7}
```

### `getNotes() -> std::vector<Note>`

Applies each interval to the root note to produce the chord's pitch classes.

```cpp
auto chord = Chord::fromSymbol("Am");
auto notes = chord.getNotes(); // {Note::A, Note::C, Note::E}
```

### `getName() -> std::string`

Returns the full chord name by combining the root's display string with the quality suffix.

```cpp
auto chord = Chord::fromSymbol("F#m7");
chord.getName(); // "F#m7"
```

### `generateVoicings(const Fretboard& fretboard, const PlayabilityConfig& config = {}) -> std::vector<Voicing>`

Generates all voicings that pass the playability filter on the given fretboard.

**Algorithm:**
1. For each string, collect every fret where a chord note appears, plus `-1` (mute option)
2. Recursively enumerate all combinations (one fret choice per string)
3. Filter each complete combination through `isPlayable()`

Returns an empty vector if no voicings satisfy the constraints.

```cpp
auto fb = Fretboard::standard();
auto chord = Chord::fromSymbol("Am");

auto voicings = chord.generateVoicings(fb);
// 389 voicings with default config

PlayabilityConfig tight;
tight.maxFretSpan = 3;
auto fewer = chord.generateVoicings(fb, tight);
// Fewer voicings due to tighter span constraint
```

### `static printVoicing(const Voicing& voicing, std::ostream& os)`

Prints a voicing as a space-separated string of fret numbers, with `x` for muted strings, followed by a newline.

```cpp
Voicing v = {-1, 0, 2, 2, 1, 0};
Chord::printVoicing(v, std::cout); // "x 0 2 2 1 0\n"
```

---

## Supported Chord Qualities

These are the quality suffixes recognized by the formula registry:

| Quality suffix | Name | Intervals (semitones) | Example notes from C |
|---|---|---|---|
| `""` (empty) | Major | 0, 4, 7 | C, E, G |
| `"m"` | Minor | 0, 3, 7 | C, Eb, G |
| `"7"` | Dominant 7th | 0, 4, 7, 10 | C, E, G, Bb |
| `"maj7"` | Major 7th | 0, 4, 7, 11 | C, E, G, B |
| `"m7"` | Minor 7th | 0, 3, 7, 10 | C, Eb, G, Bb |
| `"dim"` | Diminished | 0, 3, 6 | C, Eb, Gb |
| `"aug"` | Augmented | 0, 4, 8 | C, E, G# |
| `"sus2"` | Suspended 2nd | 0, 2, 7 | C, D, G |
| `"sus4"` | Suspended 4th | 0, 5, 7 | C, F, G |
