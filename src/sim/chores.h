// The prologue's ordinary life: the day's chores, water from the well, a meal
// at home, and sleep. Chores earn standing in the village, as an ordinary game
// would, until the after-mode, when they are done for nothing. Sleeping at
// home at night brings the next morning; the first night after the rope and
// the clay brings the dream and the deep sleep first (chapter 3), and the
// morning after it is the after-mode, which never ends.
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

// Whether the first two teachings are done: every snake seen through, a pot
// broken, and the clay lens used. The next sleep is the dream.
bool firstTeachingsDone(const World& w);
// Whether every teaching is done: the first two, and the three states.
bool teachingsDone(const World& w);

// The HUD's one-line answer to "what now?": the next teaching in the order
// an afternoon allows, then sleep, then, in the after-mode, that nothing more
// is asked. Keys are named in brackets, (C), for the page to rename on a
// touch screen.
std::string nextStep(const World& w);

// Where the next step is done, when it is done in one place: the wheel, the
// drying board, the ghat, or your door (at dusk, whether the path's snakes
// are met yet or not; they are not given away). False when there is
// no one place (the clay lens, a dream, the after-mode).
bool nextPlace(const World& w, Vec2& at);
// Which way that place is, in words, from where the player stands and looks:
// "30 m ahead, to the right". Empty when it is within a few metres.
std::string bearingText(const World& w, Vec2 at);

// Where to stand to sleep: just outside your door.
Vec2 yourDoorstep();

// One tick of chores; tick() calls it.
void choresTick(World& w, const Input& in);
// One tick of the dream or the deep sleep: each ends after its time, and
// waking from the deep sleep opens the dream's and the fourth's pages.
void sleepTick(World& w);

}  // namespace atman
