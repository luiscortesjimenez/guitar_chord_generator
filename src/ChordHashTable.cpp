#include "ChordEngine/ChordHashTable.h"
#include <utility>

namespace ChordEngine {

// --- Construction ---

ChordHashTable::ChordHashTable(std::size_t initialCapacity)
    : slots_(initialCapacity)
{
}

// --- Hashing and probing ---

// DJB2 hash, simple and good enough for short chord name strings.
std::size_t ChordHashTable::hashKey(const std::string& key) const {
    std::size_t hashValue = 5381;
    for (char ch : key)
        hashValue = hashValue * 33 + static_cast<unsigned char>(ch);
    return hashValue % slots_.size();
}

// Linear probe, walk forward until we hit Empty or a matching key.
// Tombstones are skipped (the real key might be further along).
std::size_t ChordHashTable::probe(const std::string& key) const {
    std::size_t slotIndex = hashKey(key);
    while (true) {
        const Slot& slot = slots_[slotIndex];
        if (slot.state == SlotState::Empty)
            return slotIndex;
        if (slot.state == SlotState::Occupied && slot.entry.key == key)
            return slotIndex;
        slotIndex = (slotIndex + 1) % slots_.size();
    }
}

// --- Insert ---

// Custom probe here so we can reuse tombstone slots on insert.
void ChordHashTable::insert(
    const std::string& chordName,
    const Chord& chord,
    const std::vector<Voicing>& voicings)
{
    std::size_t slotIndex = hashKey(chordName);
    std::size_t firstTombstone = slots_.size(); // sentinel

    while (true) {
        Slot& slot = slots_[slotIndex];

        if (slot.state == SlotState::Empty) {
            std::size_t targetIndex =
                (firstTombstone < slots_.size()) ? firstTombstone : slotIndex;
            Slot& targetSlot = slots_[targetIndex];

            targetSlot.state          = SlotState::Occupied;
            targetSlot.entry.key      = chordName;
            targetSlot.entry.chord    = chord;
            targetSlot.entry.voicings = voicings;

            ++entryCount_;
            if (targetIndex == slotIndex)
                ++usedSlots_;

            break;
        }

        if (slot.state == SlotState::Tombstone && firstTombstone == slots_.size()) {
            firstTombstone = slotIndex;
        }

        if (slot.state == SlotState::Occupied && slot.entry.key == chordName) {
            slot.entry.chord    = chord;
            slot.entry.voicings = voicings;
            return;
        }

        slotIndex = (slotIndex + 1) % slots_.size();
    }

    double loadFactor = static_cast<double>(usedSlots_) /
                        static_cast<double>(slots_.size());
    if (loadFactor > maxLoadFactor_)
        rehash(slots_.size() * 2);
}

// --- Find and remove ---

const ChordEntry* ChordHashTable::find(const std::string& chordName) const {
    std::size_t slotIndex = probe(chordName);
    if (slots_[slotIndex].state == SlotState::Occupied)
        return &slots_[slotIndex].entry;
    return nullptr;
}

// Tombstone the slot instead of shifting entries.
// entryCount_ drops but usedSlots_ doesn't — tombstones still
// count toward load factor until the next rehash cleans them up.
bool ChordHashTable::remove(const std::string& chordName) {
    std::size_t slotIndex = probe(chordName);
    if (slots_[slotIndex].state != SlotState::Occupied)
        return false;

    slots_[slotIndex].state = SlotState::Tombstone;
    --entryCount_;
    return true;
}

// --- Size queries ---

std::size_t ChordHashTable::size()     const { return entryCount_; }
std::size_t ChordHashTable::capacity() const { return slots_.size(); }
bool        ChordHashTable::empty()    const { return entryCount_ == 0; }

// --- Rehash ---

// Grow and re-insert all live entries. Tombstones get discarded.
void ChordHashTable::rehash(std::size_t newCapacity) {
    std::vector<Slot> oldSlots = std::move(slots_);

    slots_.assign(newCapacity, Slot{});
    entryCount_ = 0;
    usedSlots_  = 0;

    for (Slot& oldSlot : oldSlots) {
        if (oldSlot.state == SlotState::Occupied) {
            insert(oldSlot.entry.key,
                   oldSlot.entry.chord,
                   oldSlot.entry.voicings);
        }
    }
}

// --- Iterator ---

ChordHashTable::Iterator::Iterator(
    const std::vector<Slot>& slots, std::size_t index)
    : slots_(&slots), index_(index)
{
    advanceToNextOccupied();
}

void ChordHashTable::Iterator::advanceToNextOccupied() {
    while (index_ < slots_->size() &&
           (*slots_)[index_].state != SlotState::Occupied) {
        ++index_;
    }
}

const ChordEntry& ChordHashTable::Iterator::operator*() const {
    return (*slots_)[index_].entry;
}

const ChordEntry* ChordHashTable::Iterator::operator->() const {
    return &(*slots_)[index_].entry;
}

ChordHashTable::Iterator& ChordHashTable::Iterator::operator++() {
    ++index_;
    advanceToNextOccupied();
    return *this;
}

bool ChordHashTable::Iterator::operator==(const Iterator& other) const {
    return index_ == other.index_;
}

bool ChordHashTable::Iterator::operator!=(const Iterator& other) const {
    return index_ != other.index_;
}

ChordHashTable::Iterator ChordHashTable::begin() const {
    return Iterator(slots_, 0);
}

ChordHashTable::Iterator ChordHashTable::end() const {
    return Iterator(slots_, slots_.size());
}

} // namespace ChordEngine
