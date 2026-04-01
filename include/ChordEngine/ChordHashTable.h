#ifndef CHORDENGINE_CHORDHASHTABLE_H
#define CHORDENGINE_CHORDHASHTABLE_H

#include "ChordEngine/Chord.h"
#include <string>
#include <vector>
#include <cstddef>

namespace ChordEngine {

// Tombstones keep linear probing chains intact after deletion.
enum class SlotState { Empty, Occupied, Tombstone };

struct ChordEntry {
    std::string          key;
    Chord                chord;
    std::vector<Voicing> voicings;

    // Dummy default so we can resize the slot array freely.
    ChordEntry() : chord(Note::C, "") {}
};

// Open addressing hash table: DJB2 hash, linear probing, 0.75 load cap.
class ChordHashTable {
public:
    explicit ChordHashTable(std::size_t initialCapacity = 64);

    void insert(const std::string& chordName, const Chord& chord,
                const std::vector<Voicing>& voicings);

    const ChordEntry* find(const std::string& chordName) const;

    // Marks slot as tombstone. Returns true if key existed.
    bool remove(const std::string& chordName);

    std::size_t size() const;
    std::size_t capacity() const;
    bool        empty() const;

    class Iterator;
    Iterator begin() const;
    Iterator end() const;

private:
    struct Slot {
        SlotState  state = SlotState::Empty;
        ChordEntry entry;
    };

    std::vector<Slot> slots_;
    std::size_t       entryCount_ = 0;   // live entries
    std::size_t       usedSlots_  = 0;   // occupied + tombstones
    static constexpr double maxLoadFactor_ = 0.75;

    std::size_t hashKey(const std::string& key) const;
    std::size_t probe(const std::string& key) const;
    void rehash(std::size_t newCapacity);
};

// Skips empty/tombstone slots, only yields occupied entries.
class ChordHashTable::Iterator {
public:
    Iterator(const std::vector<Slot>& slots, std::size_t index);

    const ChordEntry& operator*() const;
    const ChordEntry* operator->() const;
    Iterator& operator++();
    bool operator==(const Iterator& other) const;
    bool operator!=(const Iterator& other) const;

private:
    const std::vector<Slot>* slots_;
    std::size_t index_;

    void advanceToNextOccupied();
};

} // namespace ChordEngine

#endif // CHORDENGINE_CHORDHASHTABLE_H
