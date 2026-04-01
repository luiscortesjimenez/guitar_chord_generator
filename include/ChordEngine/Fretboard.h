#ifndef CHORDENGINE_FRETBOARD_H
#define CHORDENGINE_FRETBOARD_H

#include <string>
#include <vector>
#include <utility>
#include <ostream>

namespace ChordEngine {

// The 12 chromatic pitch classes (sharps only, no flats).
// Backed by int (0-11) for semitone math.
enum class Note : int {
    C = 0, Cs, D, Ds, E, F, Fs, G, Gs, A, As, B
};

// Transpose a Note by semitones, wrapping around at 12.
Note operator+(Note base, int semitones);

std::string noteToString(Note note);

// A Note + octave, so we can tell E2 apart from E4.
struct Pitch {
    Note note;
    int  octave;

    // MIDI note number, handy for numeric comparison.
    int toMidi() const;

    bool operator==(const Pitch& other) const;
    bool operator<(const Pitch& other) const;
};

// Transpose a Pitch by semitones, crossing octave boundaries.
Pitch operator+(Pitch pitch, int semitones);

// 2D grid of pitches: rows = strings, columns = frets.
// Pre computed at construction for O(1) lookups.
class Fretboard {
public:
    // Throws if tuning is empty or numFrets < 1.
    Fretboard(std::vector<Pitch> tuning, int numFrets);

    static Fretboard standard(int numFrets = 22);
    static Fretboard dropD(int numFrets = 22);

    Pitch getPitchAt(int string, int fret) const;
    Note  getNoteAt(int string, int fret) const;

    int   getNumStrings() const;
    int   getNumFrets() const;
    const std::vector<Pitch>& getTuning() const;

    // Returns every (string, fret) where the given note appears.
    std::vector<std::pair<int,int>> findNotePositions(Note target) const;

    void printGrid(std::ostream& os) const;

private:
    std::vector<Pitch> tuning_;
    int numFrets_;
    std::vector<std::vector<Pitch>> grid_;

    void buildGrid();
};

} // namespace ChordEngine

#endif // CHORDENGINE_FRETBOARD_H
