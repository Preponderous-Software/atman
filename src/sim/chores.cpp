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

// Out of the door at kWakeMinute the next morning.
void wake(World& w) {
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
    w.sleep = Sleep::Awake;
    w.sleptTick = -1;
    if (teachingsDone(w)) w.after = true;
}

void sleep(World& w) {
    if (!firstTeachingsDone(w) || w.codex[static_cast<int>(CodexId::Fourth)]) {
        wake(w);
        return;
    }
    // The first night after the rope and the clay: the dream, at the same
    // doorstep, as if it were the same evening.
    w.sleep = Sleep::Dream;
    w.sleptTick = w.tick;
    w.wokeTick = w.tick;  // it opens out of the dark, as a morning does
    w.lampLit = false;
    w.carry = Carry::Nothing;
    const Vec2 step = yourDoorstep();
    w.player.x = step.x;
    w.player.z = step.z;
    w.player.yaw = 0;
    w.player.pitch = 0;
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

bool firstTeachingsDone(const World& w) {
    for (CodexId c : {CodexId::RopeAndSnake, CodexId::ClayAndPots, CodexId::PotSpace})
        if (!w.codex[static_cast<int>(c)]) return false;
    return w.fearRetired;
}

bool teachingsDone(const World& w) {
    for (bool c : w.codex)
        if (!c) return false;
    return w.fearRetired;
}

void sleepTick(World& w) {
    const float s = sleptSeconds(w);
    if (w.sleep == Sleep::Dream && s >= static_cast<float>(kDreamSeconds)) {
        w.sleep = Sleep::Deep;  // the dream goes, and nothing comes in its place
        w.sleptTick = w.tick;
    } else if (w.sleep == Sleep::Deep && s >= static_cast<float>(kDeepSleepSeconds)) {
        w.codex[static_cast<int>(CodexId::Dream)] = true;
        w.codex[static_cast<int>(CodexId::Fourth)] = true;
        wake(w);
    }
}

Use usable(const World& w) {
    if (w.sleep != Sleep::Awake || w.naming >= 0 || w.throwing.active) return Use::None;
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

std::string nextStep(const World& w) {
    if (w.sleep == Sleep::Dream) return "Nothing here seems strange";
    if (w.sleep == Sleep::Deep) return "";
    if (w.after && teachingsDone(w)) return "Nothing more is asked of you";
    if (w.potsMade < kPotsForLens)
        return "Throw pots at the potter's wheel (" + std::to_string(w.potsMade) + " of " + std::to_string(kPotsForLens) + ")";
    if (!w.codex[static_cast<int>(CodexId::ClayAndPots)]) return "Look at your pots through the clay lens (C)";
    if (!w.codex[static_cast<int>(CodexId::PotSpace)]) return "Break one of your pots (B)";
    if (!w.fearRetired) {
        int seen = 0;
        for (const Coil& c : w.coils) seen += c.seenThrough ? 1 : 0;
        if (!isDusk(w)) return seen ? "A snake is still on the path; it comes back at dusk" : "Wait on the ghat steps for the sunset";
        // Near a snake (fear rising), say what to do about it.
        if (w.fear > 0.2f) return w.lampLit ? "Hold the lamp to it and look at it steadily" : "Light your lamp (F) and look at it";
        if (seen)
            return "Find the other snakes on the path (" + std::to_string(seen) + " of " + std::to_string(V::kCoilCount) +
                   " seen)";
        return "Walk back to the village along the path";
    }
    return isDusk(w) ? "Go home and sleep (E at your door)" : "Live the day; sleep at home after dark";
}

bool nextPlace(const World& w, Vec2& at) {
    if (w.sleep != Sleep::Awake || (w.after && teachingsDone(w))) return false;
    if (w.potsMade < kPotsForLens) {
        at = V::kWheel;
        return true;
    }
    if (!w.codex[static_cast<int>(CodexId::ClayAndPots)]) return false;  // the lens works anywhere
    if (!w.codex[static_cast<int>(CodexId::PotSpace)]) {
        at = {(V::kBoard.x0 + V::kBoard.x1) / 2, (V::kBoard.z0 + V::kBoard.z1) / 2};
        return true;
    }
    if (!w.fearRetired && !isDusk(w)) {
        at = {(V::kGhat.x0 + V::kGhat.x1) / 2, 0.0f};
        return true;
    }
    if (!w.fearRetired) {  // at dusk, the nearest snake still on the path
        int nearest = -1;
        for (int i = 0; i < V::kCoilCount; ++i)
            if (!w.coils[i].seenThrough &&
                (nearest < 0 || distance(w.player, V::kCoils[i]) < distance(w.player, V::kCoils[nearest])))
                nearest = i;
        if (nearest >= 0) {
            at = V::kCoils[nearest];
            return true;
        }
    }
    if (!isDusk(w)) return false;
    at = yourDoorstep();
    return true;
}

std::string bearingText(const World& w, Vec2 at) {
    const float d = distance(w.player, at);
    if (d < 3.0f) return "";
    // The angle from where the player looks to the place, + to the right.
    float a = detAtan2(at.x - w.player.x, -(at.z - w.player.z)) - w.player.yaw;
    while (a > kPi) a -= 2 * kPi;
    while (a < -kPi) a += 2 * kPi;
    const float deg = std::fabs(a) * 180.0f / kPi;
    const char* side = a > 0 ? "right" : "left";
    std::string where;
    if (deg < 20) where = "ahead";
    else if (deg < 70) where = std::string("ahead, to the ") + side;
    else if (deg < 110) where = std::string("to your ") + side;
    else if (deg < 160) where = std::string("behind you, to the ") + side;
    else where = "behind you";
    const int metres = d < 10 ? static_cast<int>(d + 0.5f) : static_cast<int>(d / 5 + 0.5f) * 5;
    return std::to_string(metres) + " m " + where;
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
