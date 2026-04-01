#include <catch2/catch_test_macros.hpp>
#include "ChordEngine/Fretboard.h"
#include "ChordEngine/Chord.h"
#include "ChordEngine/ChordHashTable.h"
#include <algorithm>
#include <unordered_set>

using namespace ChordEngine;

// --- Fretboard Tests ---

TEST_CASE("Standard tuning open strings are correct", "[fretboard]") {
    auto fretboard = Fretboard::standard();

    REQUIRE(fretboard.getNumStrings() == 6);
    REQUIRE(fretboard.getNumFrets() == 22);

    auto tuning = fretboard.getTuning();
    REQUIRE(tuning[0] == Pitch{Note::E, 2});
    REQUIRE(tuning[1] == Pitch{Note::A, 2});
    REQUIRE(tuning[2] == Pitch{Note::D, 3});
    REQUIRE(tuning[3] == Pitch{Note::G, 3});
    REQUIRE(tuning[4] == Pitch{Note::B, 3});
    REQUIRE(tuning[5] == Pitch{Note::E, 4});
}

TEST_CASE("getNoteAt returns correct notes at known positions", "[fretboard]") {
    auto fretboard = Fretboard::standard();

    REQUIRE(fretboard.getNoteAt(0, 5) == Note::A);  // fret 5 low E
    REQUIRE(fretboard.getNoteAt(1, 0) == Note::A);  // open A
    REQUIRE(fretboard.getNoteAt(1, 3) == Note::C);  // fret 3 A string
    REQUIRE(fretboard.getNoteAt(2, 2) == Note::E);  // fret 2 D string
}

TEST_CASE("Fret 12 is one octave above the open string", "[fretboard]") {
    auto fretboard = Fretboard::standard();

    for (int stringIdx = 0; stringIdx < fretboard.getNumStrings(); ++stringIdx) {
        auto openPitch   = fretboard.getPitchAt(stringIdx, 0);
        auto fret12Pitch = fretboard.getPitchAt(stringIdx, 12);

        REQUIRE(openPitch.note == fret12Pitch.note);
        REQUIRE(fret12Pitch.octave == openPitch.octave + 1);
    }
}

TEST_CASE("findNotePositions includes known A positions", "[fretboard]") {
    auto fretboard = Fretboard::standard();
    auto positions = fretboard.findNotePositions(Note::A);

    auto hasPosition = [&](int stringIdx, int fretIdx) {
        return std::find(positions.begin(), positions.end(),
                         std::make_pair(stringIdx, fretIdx)) != positions.end();
    };

    REQUIRE(hasPosition(1, 0));   // open A string
    REQUIRE(hasPosition(0, 5));   // fret 5 low E
    REQUIRE(hasPosition(1, 12));
    REQUIRE(hasPosition(5, 5));
}

TEST_CASE("Out-of-range access throws", "[fretboard]") {
    auto fretboard = Fretboard::standard();

    REQUIRE_THROWS_AS(fretboard.getPitchAt(-1, 0), std::out_of_range);
    REQUIRE_THROWS_AS(fretboard.getPitchAt(6, 0), std::out_of_range);
    REQUIRE_THROWS_AS(fretboard.getPitchAt(0, -1), std::out_of_range);
    REQUIRE_THROWS_AS(fretboard.getPitchAt(0, 23), std::out_of_range);
}

TEST_CASE("Constructor rejects invalid arguments", "[fretboard]") {
    REQUIRE_THROWS_AS(Fretboard({}, 22), std::invalid_argument);
    REQUIRE_THROWS_AS(Fretboard({{Note::E, 2}}, 0), std::invalid_argument);
}

TEST_CASE("Drop D tuning lowers the low string", "[fretboard]") {
    auto fretboard = Fretboard::dropD();
    REQUIRE(fretboard.getPitchAt(0, 0) == Pitch{Note::D, 2});
    REQUIRE(fretboard.getPitchAt(1, 0) == Pitch{Note::A, 2});
}

// --- Chord Tests ---

TEST_CASE("fromSymbol parses Am correctly", "[chord]") {
    auto chord = Chord::fromSymbol("Am");
    REQUIRE(chord.getRoot() == Note::A);
    REQUIRE(chord.getQuality() == "m");
    REQUIRE(chord.getName() == "Am");
}

TEST_CASE("fromSymbol parses F#m7 correctly", "[chord]") {
    auto chord = Chord::fromSymbol("F#m7");
    REQUIRE(chord.getRoot() == Note::Fs);
    REQUIRE(chord.getQuality() == "m7");
    REQUIRE(chord.getName() == "F#m7");
}

TEST_CASE("fromSymbol parses plain major (no suffix)", "[chord]") {
    auto chord = Chord::fromSymbol("C");
    REQUIRE(chord.getRoot() == Note::C);
    REQUIRE(chord.getQuality() == "");
    REQUIRE(chord.getName() == "C");
}

TEST_CASE("getNotes returns correct pitch classes", "[chord]") {
    SECTION("Am = A, C, E") {
        auto chord = Chord::fromSymbol("Am");
        auto notes = chord.getNotes();
        REQUIRE(notes.size() == 3);
        REQUIRE(notes[0] == Note::A);
        REQUIRE(notes[1] == Note::C);
        REQUIRE(notes[2] == Note::E);
    }

    SECTION("Cmaj7 = C, E, G, B") {
        auto chord = Chord::fromSymbol("Cmaj7");
        auto notes = chord.getNotes();
        REQUIRE(notes.size() == 4);
        REQUIRE(notes[0] == Note::C);
        REQUIRE(notes[1] == Note::E);
        REQUIRE(notes[2] == Note::G);
        REQUIRE(notes[3] == Note::B);
    }
}

TEST_CASE("getName round-trips for various chords", "[chord]") {
    for (const char* symbol : {"C", "Am", "G7", "Dmaj7", "Em7", "F#m7", "Bdim", "Gaug"}) {
        auto chord = Chord::fromSymbol(symbol);
        REQUIRE(chord.getName() == symbol);
    }
}

TEST_CASE("Invalid chord symbols throw", "[chord]") {
    REQUIRE_THROWS_AS(Chord::fromSymbol(""), std::invalid_argument);
    REQUIRE_THROWS_AS(Chord::fromSymbol("Xm"), std::invalid_argument);
    REQUIRE_THROWS_AS(Chord::fromSymbol("Cblah"), std::invalid_argument);
}

TEST_CASE("generateVoicings includes classic Am voicing", "[chord]") {
    auto fretboard = Fretboard::standard();
    auto chord = Chord::fromSymbol("Am");
    auto voicings = chord.generateVoicings(fretboard);

    Voicing classicAm = {-1, 0, 2, 2, 1, 0};
    bool found = std::find(voicings.begin(), voicings.end(), classicAm)
                 != voicings.end();
    REQUIRE(found);
}

TEST_CASE("Generated voicings respect playability constraints", "[chord]") {
    auto fretboard = Fretboard::standard();
    auto chord = Chord::fromSymbol("Am");

    PlayabilityConfig config;
    config.maxFretSpan = 4;
    config.minStringsSounded = 4;
    config.requireRootInBass = true;

    auto voicings = chord.generateVoicings(fretboard, config);
    auto chordNotes = chord.getNotes();

    for (const auto& voicing : voicings) {
        int soundedCount = 0;
        int minFret = 999;
        int maxFret = 0;
        std::unordered_set<int> notesPresent;

        for (int stringIdx = 0; stringIdx < static_cast<int>(voicing.size()); ++stringIdx) {
            if (voicing[stringIdx] == -1) continue;
            ++soundedCount;
            Note soundedNote = fretboard.getNoteAt(stringIdx, voicing[stringIdx]);
            notesPresent.insert(static_cast<int>(soundedNote));
            if (voicing[stringIdx] > 0) {
                minFret = std::min(minFret, voicing[stringIdx]);
                maxFret = std::max(maxFret, voicing[stringIdx]);
            }
        }

        REQUIRE(soundedCount >= config.minStringsSounded);

        if (minFret <= maxFret)
            REQUIRE((maxFret - minFret) <= config.maxFretSpan);

        for (Note chordNote : chordNotes)
            REQUIRE(notesPresent.count(static_cast<int>(chordNote)) > 0);
    }
}

// --- ChordHashTable Tests ---

static void insertTestChord(ChordHashTable& table, const std::string& symbol) {
    auto chord = Chord::fromSymbol(symbol);
    auto fretboard = Fretboard::standard();
    auto voicings = chord.generateVoicings(fretboard);
    table.insert(symbol, chord, voicings);
}

TEST_CASE("Insert and find retrieves correct entry", "[hashtable]") {
    ChordHashTable table;
    insertTestChord(table, "Am");

    const ChordEntry* entry = table.find("Am");
    REQUIRE(entry != nullptr);
    REQUIRE(entry->key == "Am");
    REQUIRE(entry->chord.getRoot() == Note::A);
    REQUIRE(!entry->voicings.empty());
}

TEST_CASE("Find on empty table returns nullptr", "[hashtable]") {
    ChordHashTable table;
    REQUIRE(table.find("Am") == nullptr);
}

TEST_CASE("Find on missing key returns nullptr", "[hashtable]") {
    ChordHashTable table;
    insertTestChord(table, "Am");
    REQUIRE(table.find("G7") == nullptr);
}

TEST_CASE("Insert overwrites existing key", "[hashtable]") {
    ChordHashTable table;
    insertTestChord(table, "Am");
    REQUIRE(table.size() == 1);

    insertTestChord(table, "Am");
    REQUIRE(table.size() == 1);

    const ChordEntry* entry = table.find("Am");
    REQUIRE(entry != nullptr);
}

TEST_CASE("Remove returns correct bool and clears entry", "[hashtable]") {
    ChordHashTable table;
    insertTestChord(table, "Am");
    insertTestChord(table, "G");

    REQUIRE(table.remove("Am") == true);
    REQUIRE(table.find("Am") == nullptr);
    REQUIRE(table.remove("Am") == false);

    REQUIRE(table.find("G") != nullptr);
}

TEST_CASE("Remove on missing key returns false", "[hashtable]") {
    ChordHashTable table;
    REQUIRE(table.remove("Am") == false);
}

TEST_CASE("Size and empty track correctly", "[hashtable]") {
    ChordHashTable table;
    REQUIRE(table.empty());
    REQUIRE(table.size() == 0);

    insertTestChord(table, "Am");
    REQUIRE(!table.empty());
    REQUIRE(table.size() == 1);

    insertTestChord(table, "G");
    REQUIRE(table.size() == 2);

    table.remove("Am");
    REQUIRE(table.size() == 1);

    table.remove("G");
    REQUIRE(table.empty());
}

TEST_CASE("Iterator visits exactly the live entries", "[hashtable]") {
    ChordHashTable table;
    insertTestChord(table, "Am");
    insertTestChord(table, "C");
    insertTestChord(table, "G");

    int iterationCount = 0;
    for (const auto& entry : table) {
        (void)entry;
        ++iterationCount;
    }
    REQUIRE(iterationCount == 3);

    table.remove("C");
    iterationCount = 0;
    for (const auto& entry : table) {
        (void)entry;
        ++iterationCount;
    }
    REQUIRE(iterationCount == 2);
}

TEST_CASE("Rehash preserves all entries", "[hashtable]") {
    ChordHashTable table(4); // small capacity to force rehash

    for (const char* symbol : {"Am", "C", "G", "Em", "D", "Dm"}) {
        insertTestChord(table, symbol);
    }
    REQUIRE(table.size() == 6);

    for (const char* symbol : {"Am", "C", "G", "Em", "D", "Dm"}) {
        REQUIRE(table.find(symbol) != nullptr);
    }
}

TEST_CASE("Tombstone does not break probe chain", "[hashtable]") {
    ChordHashTable table(4);

    insertTestChord(table, "Am");
    insertTestChord(table, "C");

    table.remove("Am");
    REQUIRE(table.find("Am") == nullptr);
    REQUIRE(table.find("C") != nullptr);
}
