# API Reference: Fretboard

Header: `include/ChordEngine/Fretboard.h`
Implementation: `src/Fretboard.cpp`
Namespace: `ChordEngine`

---

## `enum class Note`

Represents one of the 12 chromatic pitch classes, using sharps only. Backed by `int` (0-11) so it can be used in semitone arithmetic.

| Value | Int | Value | Int |
|---|---|---|---|
| `Note::C` | 0 | `Note::Fs` | 6 |
| `Note::Cs` | 1 | `Note::G` | 7 |
| `Note::D` | 2 | `Note::Gs` | 8 |
| `Note::Ds` | 3 | `Note::A` | 9 |
| `Note::E` | 4 | `Note::As` | 10 |
| `Note::F` | 5 | `Note::B` | 11 |

### `operator+(Note base, int semitones) -> Note`

Transpose a note up or down by a number of semitones, wrapping around the 12-note cycle.

```cpp
Note a = Note::A;
Note b = a + 3;          // Note::C  (A + minor 3rd)
Note c = Note::C + (-1); // Note::B  (C down one semitone)
```

### `noteToString(Note note) -> std::string`

Returns the human-readable name of a note: `"C"`, `"C#"`, `"D"`, ..., `"B"`.

```cpp
std::string name = noteToString(Note::Fs); // "F#"
```

---

## `struct Pitch`

A `Note` paired with an octave number, representing an absolute pitch (e.g. E2 is the low E string, E4 is the high E string).

### Fields

| Field | Type | Description |
|---|---|---|
| `note` | `Note` | Pitch class |
| `octave` | `int` | Octave number (e.g. 2, 3, 4) |

### `toMidi() -> int`

Converts the pitch to a MIDI note number (0-127). Formula: `(octave + 1) * 12 + note`. Middle C (C4) is MIDI 60.

```cpp
Pitch p{Note::E, 2};
int midi = p.toMidi(); // 40
```

### `operator==(const Pitch& other) -> bool`

Two pitches are equal only if both the note and octave match. `E2 != E4`.

### `operator<(const Pitch& other) -> bool`

Compares by MIDI number. Lower-sounding pitches sort first.

### `operator+(Pitch pitch, int semitones) -> Pitch`

Transpose a pitch by semitones, crossing octave boundaries as needed. Unlike `Note + int` which wraps within a single octave, this advances into higher or lower octaves.

```cpp
Pitch b3{Note::B, 3};
Pitch c4 = b3 + 1;     // {Note::C, 4} — crossed the octave boundary
```

---

## `class Fretboard`

Models a guitar fretboard as a 2D grid where rows are strings and columns are frets (0 = open string). The grid is pre-computed at construction time for O(1) lookups.

### Constructor

```cpp
Fretboard(std::vector<Pitch> tuning, int numFrets);
```

Builds a fretboard from an arbitrary tuning (a vector of open-string pitches, low string first) and a fret count.

**Throws:** `std::invalid_argument` if `tuning` is empty or `numFrets < 1`.

```cpp
Fretboard custom(
    {{Note::D, 2}, {Note::A, 2}, {Note::D, 3},
     {Note::G, 3}, {Note::B, 3}, {Note::E, 4}},
    24
);
```

### `static standard(int numFrets = 22) -> Fretboard`

Creates a standard-tuning fretboard (E2 A2 D3 G3 B3 E4).

```cpp
auto fb = Fretboard::standard();    // 6 strings, 22 frets
auto fb = Fretboard::standard(24);  // 6 strings, 24 frets
```

### `static dropD(int numFrets = 22) -> Fretboard`

Creates a Drop D tuning fretboard (D2 A2 D3 G3 B3 E4).

### `getPitchAt(int string, int fret) -> Pitch`

Returns the full pitch (note + octave) at the given position.

**Parameters:**
- `string` — zero-based string index (0 = lowest-pitched string)
- `fret` — fret number (0 = open string)

**Throws:** `std::out_of_range` if either index is out of bounds.

```cpp
auto fb = Fretboard::standard();
Pitch p = fb.getPitchAt(0, 5); // {Note::A, 2} — fret 5 on low E
```

### `getNoteAt(int string, int fret) -> Note`

Returns just the note (no octave) at the given position. Delegates to `getPitchAt` internally.

```cpp
Note n = fb.getNoteAt(1, 3); // Note::C — fret 3 on the A string
```

### `getNumStrings() -> int`

Returns the number of strings (length of the tuning vector).

### `getNumFrets() -> int`

Returns the fret count passed to the constructor.

### `getTuning() -> const std::vector<Pitch>&`

Returns a reference to the open-string pitches.

### `findNotePositions(Note target) -> std::vector<std::pair<int,int>>`

Scans the entire grid and returns every `(string, fret)` pair where the given note appears.

```cpp
auto positions = fb.findNotePositions(Note::A);
// Includes (0, 5), (1, 0), (1, 12), (3, 2), (5, 5), ...
```

### `printGrid(std::ostream& os)`

Prints a formatted fret chart:

```
        0   1   2   3   4   5  ...
  E2    E   F  F#   G  G#   A  ...
  A2    A  A#   B   C  C#   D  ...
  ...
```
