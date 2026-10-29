// What the valley sounds like, worked out from the world: the steady sounds
// (the river, the wind, birds by day and crickets at night, the wheel while
// it turns, a heartbeat while there is fear, the drone of the after-mode) and
// the one-off sounds of things happening (a jar filled, a pot broken, a page
// added to the codex). No device and no engine: the synth (synth.h) turns
// these into samples, and the tests can check them without either.
#pragma once

#include <string>
#include <vector>

#include "sim/world.h"

namespace atman::audio {

// The steady sounds, each 0 (silent) to 1 (full).
struct Ambience {
    float river = 0;     // near the water
    float wind = 0;      // always a little, more on the hills
    float birds = 0;     // by day, most around dawn and dusk
    float crickets = 0;  // at night
    float wheel = 0;     // the potter's wheel turning
    float heart = 0;     // fear
    float drone = 0;     // the after-mode
    float steps = 0;     // footsteps per second
    bool stone = false;  // the steps fall on stone (the ghat)
};

// One-off sounds.
enum class Cue : int {
    FillJar,     // water drawn from the river
    DigClay,     // clay dug from the bank
    ChoreDone,   // water given, clay left at the wheel
    Drink,
    Eat,
    Wake,        // a new morning
    LampOn,
    LampOff,
    PotMade,
    PotBroken,
    Lens,        // the clay lens on or off
    Page,        // a new page in the codex
    Rope,        // a snake seen through
    Sunset,      // the shrine's evening bell, as the sun goes down (#40)
    Count
};

// `listener` is where the ears are (the eyes, or the free camera while
// watching); `stepsPerSecond` is how fast the body is walking, 0 standing.
Ambience ambience(const World& w, Vec3 listener, float stepsPerSecond);

// The sounds that tell the player something, in words, for captions: the
// heartbeat while afraid, the wheel while a pot is thrown, the river alone in
// deep sleep. (The after-mode's tanpura plays for good, so it is not
// captioned; its first morning says the teachings are done.) Empty when none of them plays.
std::string captions(const World& w, const Ambience& a);

// The one-off sounds for the change from `before` to `after` (one frame).
std::vector<Cue> cues(const World& before, const World& after);

}  // namespace atman::audio
