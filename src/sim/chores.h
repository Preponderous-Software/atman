// The prologue's ordinary life: the day's chores, water from the well, a meal
// at home, and sleep. Chores earn standing in the village, as an ordinary game
// would, until the after-mode, when they are done for nothing. Sleeping at
// home at night brings the next morning; the first morning after both
// teachings is the after-mode, which never ends.
#pragma once

#include <string>

#include "sim/world.h"

namespace atman {

// What E does where the player stands, in the order the rules check.
enum class Use : int32_t {
    None = 0,
    FillJar,    // at the ghat's lowest steps, for the potter
    GiveWater,  // at the potter's door, carrying water
    DigClay,    // at the clay bank
    LeaveClay,  // at the wheel, carrying clay
    Drink,      // at the well
    Eat,        // at your door, by day
    Sleep,      // at your door, at night
};

constexpr int kWakeMinute = 6 * 60 + 30;  // sleeping brings 6:30 the next morning

Use usable(const World& w);
// "fill the water jar" and so on, for the prompt; empty for Use::None.
const char* useText(Use u);

// Whether both teachings are done: every snake seen through, a pot broken,
// and the clay lens used.
bool teachingsDone(const World& w);

// The HUD's one-line answer to "what now?": the next teaching in the order
// an afternoon allows, then sleep, then, in the after-mode, that nothing more
// is asked. Keys are named in brackets, (C), for the page to rename on a
// touch screen.
std::string nextStep(const World& w);

// Where to stand to sleep: just outside your door.
Vec2 yourDoorstep();

// One tick of chores; tick() calls it.
void choresTick(World& w, const Input& in);

}  // namespace atman
