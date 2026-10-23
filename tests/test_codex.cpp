// The codex: one entry for each record the rules keep, the other-schools page
// open from the start, the rest opening only with their record, and every
// plate built in and readable at the size the screen expects.
#include <cstring>

#include "check.h"
#include "codex/codex.h"
#include "sim/world.h"

using namespace atman;
using namespace atman::codex;

int main() {
    CHECK(kEntryCount == kCodexCount + 1);

    // Each record opens exactly one entry; one entry is always open.
    int always = 0;
    for (int c = 0; c < kCodexCount; ++c) {
        int opens = 0;
        for (int i = 0; i < kEntryCount; ++i)
            if (!kEntries[i].always && static_cast<int>(kEntries[i].unlock) == c) ++opens;
        CHECK(opens == 1);
    }
    for (int i = 0; i < kEntryCount; ++i)
        if (kEntries[i].always) ++always;
    CHECK(always == 1);

    // A new game opens only the other-schools page; each record adds its own.
    {
        World w = newWorld(1);
        const std::vector<int> open = openEntries(w);
        CHECK(open.size() == 1);
        CHECK(std::strcmp(kEntries[open[0]].id, "other-schools") == 0);
        w.codex[static_cast<int>(CodexId::PotSpace)] = true;
        const std::vector<int> more = openEntries(w);
        CHECK(more.size() == 2);
        CHECK(!kEntries[more[0]].always || !kEntries[more[1]].always);
        for (int c = 0; c < kCodexCount; ++c) w.codex[c] = true;
        CHECK(openEntries(w).size() == static_cast<size_t>(kEntryCount));
    }

    // Menu lines are short ASCII; plates decode at the plate size, with the
    // menu column dark and the page light.
    for (int i = 0; i < kEntryCount; ++i) {
        const Entry& e = kEntries[i];
        CHECK(std::strlen(e.menu) > 0 && std::strlen(e.menu) <= 22);
        for (const char* c = e.menu; *c; ++c) CHECK(*c >= 32 && *c <= 126);
        const Image img = decodePlate(e);
        CHECK(img.width == kPlateWidth && img.height == kPlateHeight);
        CHECK(img.rgba.size() == static_cast<size_t>(kPlateWidth) * kPlateHeight * 4);
        if (img.rgba.empty()) continue;
        auto at = [&](int x, int y) { return img.rgba[(static_cast<size_t>(y) * kPlateWidth + x) * 4]; };
        CHECK(at(kMenuWidth / 2, kPlateHeight / 2) < 64);
        CHECK(at(kPlateWidth - 8, kPlateHeight - 8) > 192);
    }

    // Bytes that are not a PNG are refused.
    {
        const unsigned char junk[] = {1, 2, 3, 4};
        const Entry bad{"bad", "bad", true, CodexId::Count, junk, sizeof junk};
        CHECK(decodePlate(bad).rgba.empty());
    }

    TEST_MAIN_END();
}
