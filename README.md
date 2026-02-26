## Guitar Chord Generator ##

## Project Structure ##

ChordArchitect/
├── .gitignore          # Tells Git what to ignore (build/, .DS_Store)
├── CMakeLists.txt      # The "Master" build instructions
├── README.md           # Your project's front door
├── docs/               # Technical notes, math logic, and design sketches
├── include/            # All your .h (header) files
│   └── ChordEngine/
│       ├── Fretboard.h
│       ├── Chord.h
│       └── ChordHashTable.h
├── src/                # All your .cpp (source) files
│   ├── main.cpp
│   ├── Fretboard.cpp
│   ├── Chord.cpp
│   └── ChordHashTable.cpp
└── tests/              # Unit tests to prove your code works
    └── test_main.cpp