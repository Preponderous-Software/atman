// The autopilot plays the whole MVP from a new game: the chores, three named
// pots, the clay lens, a broken pot, the snakes at dusk, sleep, and the
// after-mode's first morning with its chores done again by noon. It ends on
// the pinned fingerprint on every compiler. Routes go around what is solid.
#include <cinttypes>
#include <cstdio>

#include "check.h"
#include "sim/autopilot.h"
#include "sim/chores.h"
#include "sim/valley.h"
#include "sim/world.h"

using namespace atman;
namespace V = atman::valley;

int main() {
    // A route around the potter's house, from its west side to its east side.
    {
        const std::vector<Vec2> r = planRoute({-3.0f, -13.0f}, {7.0f, -13.0f});
        CHECK(r.size() >= 2);
        CHECK(r.back().x == 7.0f && r.back().z == -13.0f);
        Vec2 a{-3.0f, -13.0f};
        for (Vec2 b : r) {
            for (int k = 1; k <= 50; ++k) {
                const float t = static_cast<float>(k) / 50;
                CHECK(!V::blocked(a.x + (b.x - a.x) * t, a.z + (b.z - a.z) * t));
            }
            a = b;
        }
        // In the open it is a straight line.
        CHECK(planRoute({0.0f, 20.0f}, {10.0f, 20.0f}).size() == 1);
    }

    // The playthrough.
    World w = newWorld(1);
    Autopilot pilot;
    bool sawDusk = false, slept = false;
    int64_t limit = 3LL * 24 * 60 * 60 * kTickRate;  // three game days at most
    for (int64_t t = 0; t < limit && !playthroughDone(w); ++t) {
        const int dayBefore = w.day;
        tick(w, pilot.next(w));
        if (isDusk(w) && w.day == 0) sawDusk = true;
        if (w.day != dayBefore) {
            slept = true;
            // By the first night every teaching was done, and nothing else was rewarded.
            CHECK(w.after);
            CHECK(w.potsMade == 3);
            CHECK(w.pots[0].broken && !w.pots[1].broken && !w.pots[2].broken);
            CHECK(w.standing == 2);  // the two chores, and nothing for the teachings
        }
    }
    CHECK(playthroughDone(w));
    CHECK(sawDusk && slept);
    CHECK(w.day == 1 && w.after);
    CHECK(w.standing == 2);  // the after-mode's chores earned nothing
    CHECK(!w.clayLens && w.fearRetired);
    for (const Coil& c : w.coils) CHECK(c.seenThrough);
    for (bool c : w.codex) CHECK(c);
    const uint64_t fp = fingerprint(w);
    std::printf("playthrough ends at %s on day %d, fingerprint %016" PRIx64 "\n", clockText(w).c_str(), w.day, fp);
    CHECK(fp == kPlaythroughFingerprint);

    TEST_MAIN_END();
}
