// The autopilot: a stand-in player that lives the day from the world alone.
// Before the after-mode it plays the whole game (the chores, three pots, the
// clay lens, a broken pot, the snakes at dusk, sleep, and the dream and the
// deep sleep, walking to the ghat in the dream), which is the pinned
// playthrough; in the after-mode it is the avatar walking its chores while
// the player watches (prarabdha: the body goes on with what is already in
// motion [C5]). Deterministic: the same world always gets the same input.
#pragma once

#include <cstdint>
#include <vector>

#include "sim/world.h"

namespace atman {

class Autopilot {
public:
    // This tick's input for the world as it is.
    Input next(const World& w);
    // What it is about, for the HUD while the player watches.
    const char* doing() const { return doing_; }

private:
    void walkTo(const World& w, Vec2 to, bool hurry, Input& in);

    Vec2 goal_{};
    bool hasGoal_ = false;
    std::vector<Vec2> route_;
    float best_ = 0;      // closest the player has come to the next point
    int stalled_ = 0;     // ticks without getting closer
    const char* doing_ = "";
};

// A route from `from` to `to` around everything solid: straight if the way is
// clear, otherwise through points around the obstacles. Ends at `to`.
std::vector<Vec2> planRoute(Vec2 from, Vec2 to);

// The pinned playthrough ends here: the after-mode's first morning, both
// chores done again, at noon.
bool playthroughDone(const World& w);

// The fingerprint the playthrough ends on with seed 1 (see
// tests/test_autopilot.cpp and atman --headless --autopilot). Pinned again
// for chapter 3: the playthrough now dreams and sleeps deeply before the
// after-mode's first morning.
constexpr uint64_t kPlaythroughFingerprint = 0xb0790a95717290beULL;

}  // namespace atman
