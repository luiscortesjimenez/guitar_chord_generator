# Music Theory Primer

This document covers the music theory concepts behind ChordArchitect. It is aimed at programmers who may not have a music background.

## The Chromatic Scale

Western music divides the octave into 12 equally spaced notes called **semitones**. A semitone is the smallest interval on a standard guitar — the distance of one fret. The 12 notes, in order, are:

```
C  C#  D  D#  E  F  F#  G  G#  A  A#  B
0   1  2   3  4  5   6  7   8  9  10  11
```

After B the cycle repeats at the next octave: B -> C (one octave higher).

### Why sharps only?

Each "in-between" note has two names. For example, the note between C and D can be called C# (C sharp) or Db (D flat). ChordArchitect uses sharps exclusively to keep the mapping one-to-one: each `Note` enum value has exactly one string representation. This avoids the complexity of enharmonic spelling (choosing between C# and Db depending on context).

## Octaves and Pitch

A **note** like "A" refers to a pitch class — all the A's on a piano or guitar, regardless of how high or low they sound. To specify an exact pitch, we pair the note with an octave number:

- **A2** — the open A string on a guitar (low)
- **A4** — the A above middle C (the standard tuning reference at 440 Hz)

The numbers increase as pitch goes up. Each octave starts at C, so B3 is followed by C4.

ChordArchitect represents this with the `Pitch` struct: a `Note` plus an `octave` integer.

## Intervals

An **interval** is the distance between two notes, measured in semitones:

| Semitones | Interval name | Example from C |
|---|---|---|
| 0 | Unison (root) | C -> C |
| 1 | Minor 2nd | C -> C# |
| 2 | Major 2nd | C -> D |
| 3 | Minor 3rd | C -> D# |
| 4 | Major 3rd | C -> E |
| 5 | Perfect 4th | C -> F |
| 6 | Tritone | C -> F# |
| 7 | Perfect 5th | C -> G |
| 8 | Augmented 5th | C -> G# |
| 9 | Major 6th | C -> A |
| 10 | Minor 7th | C -> A# |
| 11 | Major 7th | C -> B |
| 12 | Octave | C -> C (next octave) |

Intervals are the building blocks of chords. A chord is defined by its **interval formula** — a set of semitone distances from the root note.

## Chord Construction

A **chord** is three or more notes sounded together. Chords are built by stacking intervals on top of a root note.

### Example: A minor (Am)

The minor chord formula is `{0, 3, 7}` — root, minor 3rd, perfect 5th:

1. Start at the root: **A** (semitone 9)
2. Add 3 semitones: **C** (semitone 0, wraps around)
3. Add 7 semitones: **E** (semitone 4)

So Am = {A, C, E}.

### Common chord types

| Chord type | Formula | Intervals | From C |
|---|---|---|---|
| Major | 0, 4, 7 | root, major 3rd, perfect 5th | C, E, G |
| Minor | 0, 3, 7 | root, minor 3rd, perfect 5th | C, Eb, G |
| Dominant 7th | 0, 4, 7, 10 | root, major 3rd, perfect 5th, minor 7th | C, E, G, Bb |
| Major 7th | 0, 4, 7, 11 | root, major 3rd, perfect 5th, major 7th | C, E, G, B |
| Minor 7th | 0, 3, 7, 10 | root, minor 3rd, perfect 5th, minor 7th | C, Eb, G, Bb |
| Diminished | 0, 3, 6 | root, minor 3rd, tritone | C, Eb, Gb |
| Augmented | 0, 4, 8 | root, major 3rd, augmented 5th | C, E, G# |
| Suspended 2nd | 0, 2, 7 | root, major 2nd, perfect 5th | C, D, G |
| Suspended 4th | 0, 5, 7 | root, perfect 4th, perfect 5th | C, F, G |

### Chord symbols

Chord symbols combine a root note with a quality suffix:

- `C` — C major (no suffix means major)
- `Am` — A minor
- `G7` — G dominant 7th
- `Fmaj7` — F major 7th
- `F#m7` — F-sharp minor 7th

## Voicings

A chord defines *which* notes to play, but not *where* to play them on the guitar. A **voicing** is a specific assignment of fingers to strings and frets that produces the chord's notes.

For example, Am has three notes (A, C, E), but on a 6-string guitar with 22 frets there are hundreds of positions where these notes appear. A voicing picks one fret per string (or mutes the string):

```
x 0 2 2 1 0     Classic open Am
x 0 7 5 5 5     Am barre at 5th fret
5 7 7 5 5 5     Full barre Am
```

Each of these sounds the notes A, C, and E, but at different octaves and with different tonal qualities.

### Why so many voicings?

- Each of the chord's 3-4 notes can appear at multiple positions on the neck
- Notes can be doubled (the same pitch class on two different strings)
- Some strings can be muted
- The same notes at different octaves produce different timbres

ChordArchitect generates all valid combinations and filters them by playability.

## Playability Constraints

Not every mathematically valid voicing is physically playable. ChordArchitect applies three filters:

### Fret span

The distance between the lowest and highest fretted notes (excluding open strings). A human hand can typically span 4 frets comfortably. A voicing like frets 1-3-8-2 (span of 7) is not playable for most people.

### Minimum strings sounded

A chord needs enough notes to sound full. The default minimum is 4 out of 6 strings. Voicings with only 2 strings sounded would sound thin and incomplete.

### Root in bass

The lowest sounding note in a voicing is the **bass note**. When the bass note is the chord's root (e.g. A is the bass note in an Am voicing), the chord sounds stable and grounded. This is called **root position**. When a different chord tone is in the bass, it is called an **inversion** — still valid musically, but produces a different effect.

By default ChordArchitect requires root in bass. This can be disabled with the `--no-root-bass` flag to include inversions.
