# API Reference: ChordHashTable

Header: `include/ChordEngine/ChordHashTable.h`
Implementation: `src/ChordHashTable.cpp`
Namespace: `ChordEngine`

---

## `enum class SlotState`

Tracks the state of each slot in the hash table's backing array.

| Value | Meaning |
|---|---|
| `SlotState::Empty` | Never used. Terminates probe chains. |
| `SlotState::Occupied` | Holds a live entry. |
| `SlotState::Tombstone` | Previously occupied, now deleted. Skipped during search but reusable during insert. |

Tombstones are necessary for linear probing. Without them, deleting an entry could break the probe chain for keys that were inserted after it and hashed to an earlier index.

---

## `struct ChordEntry`

The data stored in each occupied slot.

| Field | Type | Description |
|---|---|---|
| `key` | `std::string` | The chord name used as the lookup key (e.g. `"Am"`, `"G7"`) |
| `chord` | `Chord` | The parsed `Chord` object |
| `voicings` | `std::vector<Voicing>` | Precomputed voicings for this chord |

The default constructor initializes `chord` with a dummy C major chord so that slot arrays can be resized without requiring every entry to be explicitly initialized.

---

## `class ChordHashTable`

A custom open-addressing hash table mapping chord name strings to `ChordEntry` values. Uses linear probing for collision resolution and DJB2 for hashing. Automatically doubles capacity when the load factor exceeds 0.75.

### Complexity

| Operation | Average | Worst case |
|---|---|---|
| `insert` | O(1) amortized | O(n) during rehash |
| `find` | O(1) | O(n) with pathological hashing |
| `remove` | O(1) | O(n) with pathological hashing |
| Iteration | O(capacity) | O(capacity) |

In practice, the 0.75 load factor cap guarantees at least 25% of slots are empty, keeping probe chains short.

### Constructor

```cpp
explicit ChordHashTable(std::size_t initialCapacity = 64);
```

Creates an empty table with the given initial slot count. All slots start as `Empty`.

```cpp
ChordHashTable table;       // 64 slots
ChordHashTable small(8);    // 8 slots (useful for testing rehash)
```

### `insert(const std::string& chordName, const Chord& chord, const std::vector<Voicing>& voicings)`

Inserts a new entry or overwrites an existing one with the same key. If the key already exists, its chord and voicings are replaced and the size does not change.

After a new insertion, checks the load factor (`usedSlots / capacity`). If it exceeds 0.75, the table is rehashed to double its capacity.

```cpp
auto fb = Fretboard::standard();
auto chord = Chord::fromSymbol("Am");
auto voicings = chord.generateVoicings(fb);

ChordHashTable table;
table.insert("Am", chord, voicings);
```

### `find(const std::string& chordName) -> const ChordEntry*`

Looks up an entry by chord name. Returns a `const` pointer to the entry if found, or `nullptr` if the key is not in the table.

```cpp
const ChordEntry* entry = table.find("Am");
if (entry) {
    std::cout << entry->key << ": "
              << entry->voicings.size() << " voicings\n";
}
```

### `remove(const std::string& chordName) -> bool`

Marks the entry's slot as a tombstone. Returns `true` if the key was found and removed, `false` if it was not present.

The entry count decreases but the used-slot count stays the same (tombstones still occupy space for probing). Accumulated tombstones are cleaned up when the next rehash occurs.

```cpp
bool removed = table.remove("Am"); // true
table.find("Am");                  // nullptr
```

### `size() -> std::size_t`

Returns the number of live (occupied) entries. Does not count tombstones.

### `capacity() -> std::size_t`

Returns the total number of slots in the backing array.

### `empty() -> bool`

Returns `true` if the table has no live entries.

---

## `class ChordHashTable::Iterator`

A const forward iterator that skips empty and tombstone slots, yielding only live `ChordEntry` references. Enables range-based for loops over the table.

### Usage

```cpp
ChordHashTable table;
// ... insert entries ...

for (const auto& entry : table) {
    std::cout << entry.key << ": "
              << entry.voicings.size() << " voicings\n";
}
```

### `begin() -> Iterator`

Returns an iterator pointing to the first occupied slot, or `end()` if the table is empty.

### `end() -> Iterator`

Returns a past-the-end iterator.

### Iterator operations

| Operation | Description |
|---|---|
| `*it` | Returns a `const ChordEntry&` for the current entry |
| `it->key` | Access entry members via arrow operator |
| `++it` | Advance to the next occupied slot |
| `it == other` | Equality comparison (compares slot index) |
| `it != other` | Inequality comparison |

---

## Internals

These are private but documented here for understanding:

**`hashKey(key)`** — DJB2 hash: starts with seed 5381, for each character multiplies the running hash by 33 and adds the character value, then mods by slot count.

**`probe(key)`** — Starting from `hashKey(key)`, walks forward one slot at a time (wrapping at capacity) until it finds an `Empty` slot (key not present) or an `Occupied` slot with a matching key (key found). Tombstones are skipped.

**`rehash(newCapacity)`** — Allocates a new slot array of the given size, re-inserts all live entries, and discards tombstones. After rehash, `usedSlots == entryCount`.
