// The codex: every verse the game has shown, each as a plate (an image made
// offline by tools/plates/make_plates.py, since Strata's text is ASCII only),
// plus the other-schools page, which is open from the start. The entries and
// their plates are built into the game from content/codex and content/plates.
// No engine here: the play layer draws the plates.
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "sim/world.h"

namespace atman::codex {

struct Entry {
    const char* id;
    const char* menu;  // its line in the codex's menu (ASCII)
    bool always;       // open from the start
    CodexId unlock;    // otherwise, the record that opens it
    const unsigned char* plate;  // PNG
    size_t plateSize;
};

extern const Entry kEntries[];
extern const int kEntryCount;

// Every plate is this size, with the menu column at its left.
constexpr int kPlateWidth = 1600;
constexpr int kPlateHeight = 900;
constexpr int kMenuWidth = 460;

bool isOpen(const World& w, const Entry& e);

// The index in kEntries of the entry with this id, or -1.
int find(const char* id);

// The entries the player can read, as indexes into kEntries, in order.
std::vector<int> openEntries(const World& w);

struct Image {
    int width = 0, height = 0;
    std::vector<uint8_t> rgba;
};

// The entry's plate as RGBA8, top row first; empty if it does not decode.
Image decodePlate(const Entry& e);

}  // namespace atman::codex
