#include "ChordEngine/Chord.h"
#include <stdexcept>
#include <algorithm>
#include <unordered_set>

namespace ChordEngine {

// --- Chord formula registry ---

// Maps quality suffix -> semitone intervals from root.
// Empty string "" = plain major.
const std::unordered_map<std::string, std::vector<int>>& Chord::getChordFormulas() {
    static const std::unordered_map<std::string, std::vector<int>> formulas = {
        {"",     {0, 4, 7}},        // major
        {"m",    {0, 3, 7}},        // minor
        {"7",    {0, 4, 7, 10}},    // dominant 7th
        {"maj7", {0, 4, 7, 11}},    // major 7th
        {"m7",   {0, 3, 7, 10}},    // minor 7th
        {"dim",  {0, 3, 6}},        // diminished
        {"aug",  {0, 4, 8}},        // augmented
        {"sus2", {0, 2, 7}},        // suspended 2nd
        {"sus4", {0, 5, 7}},        // suspended 4th
    };
    return formulas;
}

// --- Construction and parsing ---

Chord::Chord(Note root, const std::string& quality)
    : root_(root), quality_(quality)
{
    const auto& formulas = getChordFormulas();
    auto formulaIt = formulas.find(quality);
    if (formulaIt == formulas.end())
        throw std::invalid_argument("Unknown chord quality: \"" + quality + "\"");
    intervals_ = formulaIt->second;
}

// Extract root note letter (+ optional #) from the beginning of a symbol.
static Note parseRootNote(const std::string& symbol, std::size_t& parsePosition) {
    if (parsePosition >= symbol.size())
        throw std::invalid_argument("Empty chord symbol");

    char letter = symbol[parsePosition];
    if (letter < 'A' || letter > 'G')
        throw std::invalid_argument(
            std::string("Invalid root note: ") + letter);
    ++parsePosition;

    bool isSharp = (parsePosition < symbol.size() && symbol[parsePosition] == '#');
    if (isSharp) ++parsePosition;

    //  A   B   C   D   E   F   G
    static const int letterToSemitone[] = {9, 11, 0, 2, 4, 5, 7};
    int semitone = letterToSemitone[letter - 'A'];
    if (isSharp) semitone = (semitone + 1) % 12;

    return static_cast<Note>(semitone);
}

// Split "F#m7" into root=F#, quality="m7".
Chord Chord::fromSymbol(const std::string& symbol) {
    std::size_t parsePosition = 0;
    Note root = parseRootNote(symbol, parsePosition);
    std::string quality = symbol.substr(parsePosition);
    return Chord(root, quality);
}

// --- Getters ---

Note                    Chord::getRoot()      const { return root_; }
const std::string&      Chord::getQuality()   const { return quality_; }
const std::vector<int>& Chord::getIntervals() const { return intervals_; }

std::vector<Note> Chord::getNotes() const {
    std::vector<Note> notes;
    notes.reserve(intervals_.size());
    for (int interval : intervals_)
        notes.push_back(root_ + interval);
    return notes;
}

std::string Chord::getName() const {
    return noteToString(root_) + quality_;
}

// --- Voicing generation ---

// For each string, collect valid frets (+ mute option), then enumerate
// every combination and keep the ones that pass playability checks.
std::vector<Voicing> Chord::generateVoicings(
    const Fretboard& fretboard,
    const PlayabilityConfig& config) const
{
    int numStrings = fretboard.getNumStrings();
    std::vector<Note> chordNotes = getNotes();

    // Build per string candidate frets.
    std::vector<std::vector<int>> candidatesPerString(numStrings);
    for (int stringIdx = 0; stringIdx < numStrings; ++stringIdx) {
        candidatesPerString[stringIdx].push_back(-1);
    }

    for (Note chordNote : chordNotes) {
        for (auto [stringIdx, fretIdx] : fretboard.findNotePositions(chordNote)) {
            candidatesPerString[stringIdx].push_back(fretIdx);
        }
    }

    // Recursively try every combination (one choice per string).
    std::vector<Voicing> results;
    Voicing currentVoicing(numStrings);

    std::function<void(int)> enumerate = [&](int stringIdx) {
        if (stringIdx == numStrings) {
            if (isPlayable(currentVoicing, fretboard, config))
                results.push_back(currentVoicing);
            return;
        }
        for (int fretChoice : candidatesPerString[stringIdx]) {
            currentVoicing[stringIdx] = fretChoice;
            enumerate(stringIdx + 1);
        }
    };

    enumerate(0);
    return results;
}

// --- Playability filter ---

// Checks constraints in order of cheapness, bails early on failure.
bool Chord::isPlayable(
    const Voicing& voicing,
    const Fretboard& fretboard,
    const PlayabilityConfig& config) const
{
    int soundedCount = 0;
    int minFret = 999;
    int maxFret = 0;
    for (int fretValue : voicing) {
        if (fretValue == -1) continue;
        ++soundedCount;
        if (fretValue > 0) { // open strings don't count toward span
            minFret = std::min(minFret, fretValue);
            maxFret = std::max(maxFret, fretValue);
        }
    }

    if (soundedCount < config.minStringsSounded)
        return false;

    if (minFret <= maxFret && (maxFret - minFret) > config.maxFretSpan)
        return false;

    // All chord notes must appear somewhere in the voicing.
    std::vector<Note> chordNotes = getNotes();
    std::unordered_set<int> remainingNotes;
    for (Note note : chordNotes)
        remainingNotes.insert(static_cast<int>(note));

    for (int stringIdx = 0; stringIdx < static_cast<int>(voicing.size()); ++stringIdx) {
        if (voicing[stringIdx] == -1) continue;
        Note soundedNote = fretboard.getNoteAt(stringIdx, voicing[stringIdx]);
        remainingNotes.erase(static_cast<int>(soundedNote));
    }
    if (!remainingNotes.empty())
        return false;

    // Lowest sounded string must play the root.
    if (config.requireRootInBass) {
        for (int stringIdx = 0; stringIdx < static_cast<int>(voicing.size()); ++stringIdx) {
            if (voicing[stringIdx] == -1) continue;
            Note bassNote = fretboard.getNoteAt(stringIdx, voicing[stringIdx]);
            return bassNote == root_;
        }
    }

    return true;
}

// --- Display ---

void Chord::printVoicing(const Voicing& voicing, std::ostream& os) {
    for (std::size_t stringIdx = 0; stringIdx < voicing.size(); ++stringIdx) {
        if (stringIdx > 0) os << " ";
        if (voicing[stringIdx] == -1)
            os << "x";
        else
            os << voicing[stringIdx];
    }
    os << "\n";
}

} // namespace ChordEngine
