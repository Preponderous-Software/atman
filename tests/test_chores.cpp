// The prologue's chores and the night: water from the ghat to the potter's
// door, clay from the bank to the wheel, each earning standing (until the
// after-mode, when they earn nothing); the well and a meal at home; and sleep
// at your door, which brings 6:30 the next morning and a new day's chores,
// or, the first night after both teachings, the dream (test_states.cpp).
#include <cmath>

#include "check.h"
#include "sim/chores.h"
#include "sim/clay.h"
#include "sim/valley.h"
#include "sim/world.h"

using namespace atman;
namespace V = atman::valley;

namespace {

World at(float x, float z) {
    World w = newWorld(1);
    w.player.x = x;
    w.player.z = z;
    return w;
}

void use(World& w) {
    Input in;
    in.yaw = w.player.yaw;
    in.pitch = w.player.pitch;
    in.use = true;
    tick(w, in);
}

void moveTo(World& w, Vec2 p) {
    w.player.x = p.x;
    w.player.z = p.z;
}

int minuteOfDay(const World& w) { return static_cast<int>(gameMinutes(w)) % 1440; }

}  // namespace

int main() {
    // Each place is where a player can stand.
    CHECK(!V::blocked(V::kClayBank.x, V::kClayBank.z));
    CHECK(!V::blocked(V::kDrawWaterX + 0.6f, 0.0f));
    CHECK(!V::blocked(yourDoorstep().x, yourDoorstep().z));

    // Water: fill at the ghat, give at the potter's door. Standing rises once.
    {
        World w = at(V::kDrawWaterX + 0.6f, 0.0f);
        CHECK(usable(w) == Use::FillJar);
        use(w);
        CHECK(w.carry == Carry::Water);
        CHECK(usable(w) == Use::None);  // one jar at a time
        moveTo(w, {2.0f, -9.5f});
        CHECK(usable(w) == Use::GiveWater);
        use(w);
        CHECK(w.carry == Carry::Nothing);
        CHECK(w.questDone[static_cast<int>(QuestId::CarryWater)]);
        CHECK(w.standing == 1);
        // Done for today: the river offers nothing more.
        moveTo(w, {V::kDrawWaterX + 0.6f, 0.0f});
        CHECK(usable(w) == Use::None);
    }

    // Clay: dig at the bank, leave it at the wheel.
    {
        World w = at(V::kClayBank.x, V::kClayBank.z);
        CHECK(usable(w) == Use::DigClay);
        use(w);
        CHECK(w.carry == Carry::Clay);
        moveTo(w, {9.0f, -12.2f});
        CHECK(usable(w) == Use::LeaveClay);
        use(w);
        CHECK(w.questDone[static_cast<int>(QuestId::FetchClay)]);
        CHECK(w.standing == 1);
        // Walking the clay elsewhere does nothing.
        World m = at(V::kClayBank.x, V::kClayBank.z);
        use(m);
        moveTo(m, {2.0f, -9.5f});
        CHECK(usable(m) == Use::None);
    }

    // The well answers thirst; home answers hunger by day; away from both, E does nothing.
    {
        World w = at(V::kWell.x, V::kWell.z + 1.8f);
        w.needs.thirst = 0.8f;
        CHECK(usable(w) == Use::Drink);
        use(w);
        CHECK(w.needs.thirst < 0.01f);
        moveTo(w, yourDoorstep());
        w.needs.hunger = 0.9f;
        CHECK(usable(w) == Use::Eat);
        use(w);
        CHECK(w.needs.hunger < 0.01f);
        moveTo(w, {0.0f, 20.0f});
        CHECK(usable(w) == Use::None);
    }

    // Sleep: only at night at your door. It brings 6:30 the next morning,
    // rested, the lamp out, standing kept, the day's chores on the list again.
    {
        World w = at(yourDoorstep().x, yourDoorstep().z);
        CHECK(usable(w) == Use::Eat);  // 4:30 pm: too early to sleep
        w.tick = static_cast<int64_t>((21 * 60 - kStartMinute) / kGameMinutesPerSecond) * kTickRate;
        w.lampLit = true;
        w.questDone = {true, true};
        w.standing = 2;
        CHECK(usable(w) == Use::Sleep);
        use(w);
        CHECK(w.day == 1);
        CHECK(minuteOfDay(w) == kWakeMinute);
        CHECK(gameMinutes(w) > 1440);
        CHECK(w.wokeTick == w.tick);
        CHECK(w.needs.tiredness == 0);
        CHECK(!w.lampLit);
        CHECK(!w.questDone[0] && !w.questDone[1]);
        CHECK(w.standing == 2);
        CHECK(!w.after);  // the teachings were not done
        CHECK(w.player.x == yourDoorstep().x && w.player.z == yourDoorstep().z);
        // Sleeping after midnight still wakes the same morning.
        World late = at(yourDoorstep().x, yourDoorstep().z);
        late.tick = static_cast<int64_t>((24 * 60 + 2 * 60 - kStartMinute) / kGameMinutesPerSecond) * kTickRate;
        use(late);
        CHECK(late.day == 1);
        CHECK(static_cast<int>(gameMinutes(late)) == 1440 + kWakeMinute);
    }

    // After both teachings, the next morning is the after-mode: the chores
    // come round again and earn nothing.
    {
        World w = at(yourDoorstep().x, yourDoorstep().z);
        w.tick = static_cast<int64_t>((21 * 60 - kStartMinute) / kGameMinutesPerSecond) * kTickRate;
        for (bool& c : w.codex) c = true;
        CHECK(!teachingsDone(w));  // the fear meter is still there
        w.fearRetired = true;
        CHECK(teachingsDone(w));
        w.standing = 2;
        use(w);
        CHECK(w.after);
        moveTo(w, {V::kDrawWaterX + 0.6f, 0.0f});
        use(w);
        moveTo(w, {2.0f, -9.5f});
        use(w);
        CHECK(w.questDone[static_cast<int>(QuestId::CarryWater)]);
        CHECK(w.standing == 2);
    }

    // Nothing is usable while a pot waits for its name.
    {
        World w = at(V::kWell.x, V::kWell.z + 1.8f);
        w.naming = 0;
        CHECK(usable(w) == Use::None);
    }

    // What the HUD says to do next, from the first afternoon to the after-mode.
    {
        World w = newWorld(1);
        auto atMinute = [&](int minute) {
            w.tick = static_cast<int64_t>((minute - kStartMinute) / kGameMinutesPerSecond) * kTickRate;
        };
        CHECK(nextStep(w) == "Throw pots at the potter's wheel (0 of 3)");
        w.potsMade = 2;
        CHECK(nextStep(w) == "Throw pots at the potter's wheel (2 of 3)");
        w.potsMade = kPotsForLens;
        CHECK(nextStep(w).find("clay lens") != std::string::npos);
        w.codex[static_cast<int>(CodexId::ClayAndPots)] = true;
        CHECK(nextStep(w).find("Break") == 0);
        w.codex[static_cast<int>(CodexId::PotSpace)] = true;
        CHECK(nextStep(w) == "Wait on the ghat steps for the sunset");
        atMinute(19 * 60);
        CHECK(isDusk(w));
        CHECK(nextStep(w) == "Walk back to the village along the path");
        // The bearing points along the path at the nearest snake, and NEXT
        // says what to do once one is near and how many are left.
        Vec2 at;
        CHECK(nextPlace(w, at));
        bool aCoil = false;
        for (const Vec2& c : V::kCoils) aCoil = aCoil || (c.x == at.x && c.z == at.z);
        CHECK(aCoil);
        w.fear = 0.5f;
        CHECK(nextStep(w) == "Light your lamp (F) and look at it");
        w.lampLit = true;
        CHECK(nextStep(w) == "Hold the lamp to it and look at it steadily");
        w.fear = 0;
        w.coils[0].seenThrough = true;
        CHECK(nextStep(w) == "Find the other snakes on the path (1 of 3 seen)");
        atMinute(24 * 60 + 9 * 60);  // the next morning
        CHECK(nextStep(w) == "A snake is still on the path; it comes back at dusk");
        atMinute(19 * 60);
        w.coils[0].seenThrough = false;
        w.lampLit = false;
        w.codex[static_cast<int>(CodexId::RopeAndSnake)] = true;
        w.fearRetired = true;
        CHECK(firstTeachingsDone(w) && !teachingsDone(w));
        CHECK(nextStep(w).find("Go home and sleep") == 0);
        moveTo(w, yourDoorstep());
        use(w);
        CHECK(w.sleep == Sleep::Dream && !w.after);  // the rest is in test_states.cpp
        CHECK(nextStep(w) == "Nothing here seems strange");
    }

    // Which way the next step's place is, in words.
    {
        World w = newWorld(1);
        Vec2 at;
        CHECK(nextPlace(w, at));
        CHECK(at.x == V::kWheel.x && at.z == V::kWheel.z);
        w.player.x = V::kWheel.x;
        w.player.z = V::kWheel.z + 20;  // 20 m south of the wheel
        w.player.yaw = 0;                // facing north, toward it
        CHECK(bearingText(w, at) == "20 m ahead");
        w.player.yaw = kPi / 2;  // facing east: the wheel is to the left
        CHECK(bearingText(w, at) == "20 m to your left");
        w.player.yaw = kPi;
        CHECK(bearingText(w, at) == "20 m behind you");
        w.player.z = V::kWheel.z + 2;
        CHECK(bearingText(w, at).empty());  // there already
        w.potsMade = kPotsForLens;
        CHECK(!nextPlace(w, at));  // the lens works anywhere
    }

    TEST_MAIN_END();
}
