// The after-mode's days. Nothing is asked of the player any more, but the days
// are not all the same: some evenings the village sets clay lamps along the
// ghat steps and floats more on the river, and some mornings a flock of egrets
// crosses the valley. Which day is which follows from the seed and the day
// alone, so it needs no saving and is the same every time (the second day of
// the after-mode is always a lamp night, the third always has egrets).
#pragma once

#include <cstdint>

#include "sim/world.h"

namespace atman {

enum class DayKind : int32_t { Ordinary = 0, LampNight = 1, Egrets = 2 };

// What kind of day it is: always Ordinary before the after-mode and on its
// first day.
DayKind dayKind(const World& w);

// The lamp night's lamps are lit from this minute (a little after sunset),
// and the egrets cross between these minutes of the morning.
constexpr int kLampsLitMinute = 18 * 60 + 30;
constexpr int kEgretsFromMinute = 6 * 60 + 40, kEgretsToMinute = 7 * 60 + 10;

}  // namespace atman
