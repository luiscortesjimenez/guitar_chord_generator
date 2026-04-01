#ifndef CHORDENGINE_CHORD_H
#define CHORDENGINE_CHORD_H

#include "ChordEngine/Fretboard.h"
#include <string>
#include <vector>
#include <unordered_map>
#include <ostream>

namespace ChordEngine {

// One way to play a chord: fret number per string, -1 = muted.
using Voicing = std::vector<int>;

// Filters for physically playable voicings.
struct PlayabilityConfig {
    int  maxFretSpan       = 4;
    int  minStringsSounded = 4;
    bool requireRootInBass = true;
};

// A chord defined by root note + quality ("m7", "maj7", "").
// Can parse symbols, compute notes, and generate voicings.
class Chord {
public:
    // Quality must be in the formula registry ("", "m", "7", etc.).
    Chord(Note root, const std::string& quality);

    // Parse something like "Cmaj7" or "F#m7" into root + quality.
    static Chord fromSymbol(const std::string& symbol);

    Note                    getRoot() const;
    const std::string&      getQuality() const;
    const std::vector<int>& getIntervals() const;

    // Ex: Am -> {A, C, E}
    std::vector<Note> getNotes() const;

    std::string getName() const;

    std::vector<Voicing> generateVoicings(
        const Fretboard& fretboard,
        const PlayabilityConfig& config = {}
    ) const;

    // Prints as "x 0 2 2 1 0\n".
    static void printVoicing(const Voicing& voicing, std::ostream& os);

private:
    Note             root_;
    std::string      quality_;
    std::vector<int> intervals_;

    // Quality string -> semitone intervals. Built once, shared.
    static const std::unordered_map<std::string, std::vector<int>>& getChordFormulas();

    bool isPlayable(
        const Voicing& voicing,
        const Fretboard& fretboard,
        const PlayabilityConfig& config
    ) const;
};

} // namespace ChordEngine

#endif // CHORDENGINE_CHORD_H
