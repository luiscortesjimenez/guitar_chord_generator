# CLI Usage

ChordArchitect has two modes: pass chord names as arguments for batch output, or run with no arguments for an interactive prompt.

## Synopsis

```
guitar_chord_generator [options] [chord ...]
```

## Options

| Flag | Argument | Default | Description |
|---|---|---|---|
| `-h`, `--help` | — | — | Print usage information and exit |
| `--tuning` | `standard` or `dropd` | `standard` | Guitar tuning to use |
| `--max-voicings` | integer | `10` | Maximum number of voicings to display per chord |
| `--max-span` | integer | `4` | Maximum fret span (distance between lowest and highest fretted notes) |
| `--min-strings` | integer | `4` | Minimum number of strings that must be sounded |
| `--no-root-bass` | — | off (root required) | Don't require the root note in the bass position |

## Argument Mode

When one or more chord names are provided as positional arguments, ChordArchitect processes each one and exits.

### Single chord

```bash
./build/guitar_chord_generator Am
```

```
Am (389 voicings, showing first 10):
  1. x x 7 5 5 5
  2. x x 7 5 5 8
  3. x x 7 5 5 0
  ...
```

### Multiple chords

```bash
./build/guitar_chord_generator Am G7 Cmaj7
```

Each chord is printed with a header and its voicings, separated by a blank line.

### Custom options

```bash
./build/guitar_chord_generator --tuning dropd --max-voicings 5 --max-span 3 Em
```

This generates Em voicings on a Drop D tuned guitar, limited to a 3-fret span, showing at most 5 results.

### Relaxing constraints

```bash
./build/guitar_chord_generator --no-root-bass --min-strings 3 F#m7
```

This allows voicings where the bass note is not the root and only 3 strings need to be sounded, which opens up more partial voicing options.

## Interactive Mode

Running with no chord names starts the REPL:

```bash
./build/guitar_chord_generator
```

```
ChordArchitect — interactive mode (type a chord name, or "quit" to exit)

ChordArchitect> Am
Am (389 voicings, showing first 10):
  1. x x 7 5 5 5
  2. x x 7 5 5 8
  ...

ChordArchitect> G7
G7 (360 voicings, showing first 10):
  1. x x 5 4 3 1
  ...

ChordArchitect> quit
```

Type any chord symbol at the prompt. Type `quit`, `exit`, or press Enter on an empty line to leave.

Options like `--tuning` and `--max-voicings` are still applied in interactive mode — set them when launching the program:

```bash
./build/guitar_chord_generator --tuning dropd --max-voicings 3
```

## Error Handling

Invalid chord symbols produce an error message without crashing.

In **argument mode**, the error is printed and processing continues with the next chord:

```bash
./build/guitar_chord_generator Am Xblah G7
```

```
Am (389 voicings, showing first 10):
  ...

Error: Invalid root note: X ("Xblah")

G7 (360 voicings, showing first 10):
  ...
```

In **interactive mode**, the error is printed and the prompt returns:

```
ChordArchitect> Xblah
Error: Invalid root note: X ("Xblah")

ChordArchitect>
```

## Supported Chord Symbols

A chord symbol is a root note (A-G, optionally followed by #) plus a quality suffix:

| Symbol examples | Quality | Notes (from C) |
|---|---|---|
| `C`, `G`, `F#` | major | C E G |
| `Am`, `Em`, `F#m` | minor | C Eb G |
| `G7`, `C7` | dominant 7th | C E G Bb |
| `Cmaj7`, `Fmaj7` | major 7th | C E G B |
| `Am7`, `Em7` | minor 7th | C Eb G Bb |
| `Bdim`, `C#dim` | diminished | C Eb Gb |
| `Caug`, `Gaug` | augmented | C E G# |
| `Dsus2`, `Asus2` | suspended 2nd | C D G |
| `Dsus4`, `Asus4` | suspended 4th | C F G |
