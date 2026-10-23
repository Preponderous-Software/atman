#include "sim/chores.h"

#include "sim/clay.h"
#include "sim/valley.h"

namespace atman {

namespace V = atman::valley;

namespace {

float distance(const Player& p, Vec2 c) {
    const float dx = p.x - c.x, dz = p.z - c.z;
    return std::sqrt(dx * dx + dz * dz);
}

bool atDoor(const World& w, int house) { return distance(w.player, V::kHouses[house].door) <= V::kDoorReach; }

bool atRiver(const World& w) {
    const Player& p = w.player;
    return p.x >= V::kDrawWaterX && p.x <= V::kGhat.x1 && p.z >= V::kGhat.z0 && p.z <= V::kGhat.z1;
}

// The next morning at kWakeMinute, as a tick.
int64_t nextMorning(const World& w) {
    const double minutes = gameMinutes(w);
    const auto day = static_cast<int64_t>((minutes - kWakeMinute) / 1440.0) + 1;
    const int64_t wake = day * 1440 + kWakeMinute;
    return static_cast<int64_t>((wake - kStartMinute) / kGameMinutesPerSecond) * kTickRate;
}

void sleep(World& w) {
    w.tick = nextMorning(w);
    ++w.day;
    w.wokeTick = w.tick;
    w.needs.tiredness = 0;
    w.needs.thirst = clampf(w.needs.thirst + 0.1f, 0.0f, 1.0f);
    w.needs.hunger = clampf(w.needs.hunger + 0.15f, 0.0f, 1.0f);
    w.lampLit = false;
    w.questDone = {};  // a new day's chores
    const Vec2 step = yourDoorstep();
    w.player.x = step.x;
    w.player.z = step.z;
    w.player.yaw = 0;  // out of the door, north toward the well
    w.player.pitch = 0;
    if (teachingsDone(w)) w.after = true;
}

void done(World& w, QuestId q) {
    w.questDone[static_cast<int>(q)] = true;
    w.carry = Carry::Nothing;
    // In the after-mode the chores are done for nothing (BG 2.47 [P9]).
    if (!w.after) ++w.standing;
}

}  // namespace

Vec2 yourDoorstep() {
    const V::House& h = V::kHouses[V::YourHouse];
    return {h.door.x, h.door.z - 1.0f};  // the door is on the north wall
}

bool teachingsDone(const World& w) {
    for (bool c : w.codex)
        if (!c) return false;
    return w.fearRetired;
}

Use usable(const World& w) {
    if (w.naming >= 0 || w.throwing.active) return Use::None;
    const bool carryingNothing = w.carry == Carry::Nothing;
    if (carryingNothing && atRiver(w) && !w.questDone[static_cast<int>(QuestId::CarryWater)]) return Use::FillJar;
    if (w.carry == Carry::Water && atDoor(w, V::PotterHouse)) return Use::GiveWater;
    if (carryingNothing && distance(w.player, V::kClayBank) <= V::kClayReach &&
        !w.questDone[static_cast<int>(QuestId::FetchClay)])
        return Use::DigClay;
    if (w.carry == Carry::Clay && atWheel(w)) return Use::LeaveClay;
    if (distance(w.player, V::kWell) <= V::kWellReach) return Use::Drink;
    if (atDoor(w, V::YourHouse)) return isDusk(w) ? Use::Sleep : Use::Eat;
    return Use::None;
}

const char* useText(Use u) {
    switch (u) {
        case Use::FillJar: return "fill the water jar";
        case Use::GiveWater: return "give the potter the water";
        case Use::DigClay: return "dig clay";
        case Use::LeaveClay: return "leave the clay by the wheel";
        case Use::Drink: return "drink from the well";
        case Use::Eat: return "eat";
        case Use::Sleep: return "sleep until morning";
        case Use::None: break;
    }
    return "";
}

void choresTick(World& w, const Input& in) {
    if (!in.use) return;
    switch (usable(w)) {
        case Use::FillJar: w.carry = Carry::Water; break;
        case Use::GiveWater: done(w, QuestId::CarryWater); break;
        case Use::DigClay: w.carry = Carry::Clay; break;
        case Use::LeaveClay: done(w, QuestId::FetchClay); break;
        case Use::Drink: w.needs.thirst = 0; break;
        case Use::Eat: w.needs.hunger = 0; break;
        case Use::Sleep: sleep(w); break;
        case Use::None: break;
    }
}

}  // namespace atman
