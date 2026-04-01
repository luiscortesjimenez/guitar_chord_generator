#include "ChordEngine/Fretboard.h"
#include <stdexcept>
#include <iomanip>

namespace ChordEngine {

// --- Note helpers ---

Note operator+(Note base, int semitones) {
    int wrappedNote = (static_cast<int>(base) + semitones) % 12;
    if (wrappedNote < 0) wrappedNote += 12; // C++ modulo can go negative
    return static_cast<Note>(wrappedNote);
}

std::string noteToString(Note note) {
    static const char* noteNames[] = {
        "C", "C#", "D", "D#", "E", "F",
        "F#", "G", "G#", "A", "A#", "B"
    };
    return noteNames[static_cast<int>(note)];
}

// --- Pitch ---

int Pitch::toMidi() const {
    return (octave + 1) * 12 + static_cast<int>(note);
}

bool Pitch::operator==(const Pitch& other) const {
    return note == other.note && octave == other.octave;
}

bool Pitch::operator<(const Pitch& other) const {
    return toMidi() < other.toMidi();
}

// Convert to MIDI, add semitones, convert back. Handles octave crossings.
Pitch operator+(Pitch pitch, int semitones) {
    int transposedMidi = pitch.toMidi() + semitones;
    int octave = (transposedMidi / 12) - 1;
    auto note = static_cast<Note>(transposedMidi % 12);
    return {note, octave};
}

// --- Fretboard construction ---

Fretboard::Fretboard(std::vector<Pitch> tuning, int numFrets)
    : tuning_(std::move(tuning)), numFrets_(numFrets)
{
    if (tuning_.empty())
        throw std::invalid_argument("Tuning must have at least one string");
    if (numFrets_ < 1)
        throw std::invalid_argument("Fret count must be at least 1");
    buildGrid();
}

// Fill grid_[string][fret] with the pitch at each position.
void Fretboard::buildGrid() {
    grid_.resize(tuning_.size());
    for (std::size_t stringIdx = 0; stringIdx < tuning_.size(); ++stringIdx) {
        grid_[stringIdx].resize(numFrets_ + 1);
        for (int fretIdx = 0; fretIdx <= numFrets_; ++fretIdx) {
            grid_[stringIdx][fretIdx] = tuning_[stringIdx] + fretIdx;
        }
    }
}

// --- Static factories ---

Fretboard Fretboard::standard(int numFrets) {
    return Fretboard(
        {{Note::E, 2}, {Note::A, 2}, {Note::D, 3},
         {Note::G, 3}, {Note::B, 3}, {Note::E, 4}},
        numFrets
    );
}

Fretboard Fretboard::dropD(int numFrets) {
    return Fretboard(
        {{Note::D, 2}, {Note::A, 2}, {Note::D, 3},
         {Note::G, 3}, {Note::B, 3}, {Note::E, 4}},
        numFrets
    );
}

// --- Queries ---

Pitch Fretboard::getPitchAt(int string, int fret) const {
    if (string < 0 || string >= static_cast<int>(grid_.size()))
        throw std::out_of_range("String index out of range");
    if (fret < 0 || fret > numFrets_)
        throw std::out_of_range("Fret index out of range");
    return grid_[string][fret];
}

Note Fretboard::getNoteAt(int string, int fret) const {
    return getPitchAt(string, fret).note;
}

int Fretboard::getNumStrings() const { return static_cast<int>(tuning_.size()); }
int Fretboard::getNumFrets()   const { return numFrets_; }

const std::vector<Pitch>& Fretboard::getTuning() const { return tuning_; }

std::vector<std::pair<int,int>> Fretboard::findNotePositions(Note target) const {
    std::vector<std::pair<int,int>> positions;
    for (int stringIdx = 0; stringIdx < static_cast<int>(grid_.size()); ++stringIdx) {
        for (int fretIdx = 0; fretIdx <= numFrets_; ++fretIdx) {
            if (grid_[stringIdx][fretIdx].note == target)
                positions.emplace_back(stringIdx, fretIdx);
        }
    }
    return positions;
}

// --- Display ---

void Fretboard::printGrid(std::ostream& os) const {
    const int colWidth = 4;

    os << std::setw(colWidth) << " ";
    for (int fretIdx = 0; fretIdx <= numFrets_; ++fretIdx)
        os << std::setw(colWidth) << fretIdx;
    os << "\n";

    for (int stringIdx = 0; stringIdx < static_cast<int>(grid_.size()); ++stringIdx) {
        os << std::setw(colWidth - 1) << noteToString(tuning_[stringIdx].note)
           << tuning_[stringIdx].octave;
        for (int fretIdx = 0; fretIdx <= numFrets_; ++fretIdx)
            os << std::setw(colWidth) << noteToString(grid_[stringIdx][fretIdx].note);
        os << "\n";
    }
}

} // namespace ChordEngine
