#include "sim/days.h"

namespace atman {

DayKind dayKind(const World& w) {
    if (!w.after || w.sleep != Sleep::Awake || w.day < 2) return DayKind::Ordinary;
    if (w.day == 2) return DayKind::LampNight;
    if (w.day == 3) return DayKind::Egrets;
    // After that, one day in four of each, mixed by the seed.
    uint64_t h = w.seed * 0x9E3779B97F4A7C15ULL + static_cast<uint64_t>(w.day) * 0xBF58476D1CE4E5B9ULL;
    h ^= h >> 31;
    switch (h % 4) {
        case 0: return DayKind::LampNight;
        case 1: return DayKind::Egrets;
        default: return DayKind::Ordinary;
    }
}

}  // namespace atman
