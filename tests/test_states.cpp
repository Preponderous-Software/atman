// Chapter 3, Three States: the first sleep after the rope and the clay is the
// dream, the same valley with the sun running backwards and nothing to do but
// walk; then deep sleep, where no input reaches anything; then the morning,
// with the dream's and the fourth's codex pages open and the after-mode
// begun. Every other night is the cut to morning.
#include <cmath>

#include "check.h"
#include "sim/autopilot.h"
#include "sim/chores.h"
#include "sim/days.h"
#include "sim/world.h"

using namespace atman;

namespace {

Input still(const World& w) {
    Input in;
    in.yaw = w.player.yaw;
    in.pitch = w.player.pitch;
    return in;
}

// The first two teachings done, at night, at your door.
World readyToSleep() {
    World w = newWorld(1);
    for (CodexId c : {CodexId::RopeAndSnake, CodexId::ClayAndPots, CodexId::PotSpace}) w.codex[static_cast<int>(c)] = true;
    for (Coil& c : w.coils) c.seenThrough = true;
    w.fearRetired = true;
    w.potsMade = 0;
    w.tick = static_cast<int64_t>((20 * 60 - kStartMinute) / kGameMinutesPerSecond) * kTickRate;  // 8 pm
    const Vec2 step = yourDoorstep();
    w.player.x = step.x;
    w.player.z = step.z;
    return w;
}

void run(World& w, int seconds, Input in) {
    for (int i = 0; i < seconds * kTickRate; ++i) tick(w, in);
}

}  // namespace

int main() {
    // Before the first two teachings, sleep is the cut to morning, as ever.
    {
        World w = readyToSleep();
        w.codex[static_cast<int>(CodexId::PotSpace)] = false;
        CHECK(usable(w) == Use::Sleep);
        Input in = still(w);
        in.use = true;
        tick(w, in);
        CHECK(w.sleep == Sleep::Awake && w.day == 1 && !w.after);
    }

    // After them, the dream.
    World w = readyToSleep();
    CHECK(firstTeachingsDone(w) && !teachingsDone(w));
    CHECK(usable(w) == Use::Sleep);
    {
        Input in = still(w);
        in.use = true;
        tick(w, in);
    }
    CHECK(w.sleep == Sleep::Dream);
    CHECK(w.day == 0);  // still the same night
    CHECK(!isDusk(w));  // the dream's light is the evening's, before the dark
    CHECK(clockText(w) == "6:10 pm");
    CHECK(nextStep(w) == "Nothing here seems strange");
    // ...and under it, that the dream passes by itself: without that it read
    // as a hang to three of four AI players of 0.3.0 (#37).
    CHECK(sleepNote(w).find("passes by itself") != std::string::npos);
    CHECK(usable(w) == Use::None);

    // The dream's clock runs backwards, and the body walks, and that is all:
    // no needs, no lamp, no chores.
    const Needs needs = w.needs;
    const double sky = skyMinutes(w);
    const float x0 = w.player.x, z0 = w.player.z;
    Input walk = still(w);
    walk.forward = 1;
    walk.toggleLamp = true;
    walk.use = true;
    tick(w, walk);
    CHECK(skyMinutes(w) < sky);
    CHECK(!w.lampLit);
    CHECK(w.needs.thirst == needs.thirst && w.needs.hunger == needs.hunger);
    walk.toggleLamp = false;
    run(w, 10, walk);
    CHECK(std::fabs(w.player.x - x0) + std::fabs(w.player.z - z0) > 1.0f);
    CHECK(w.sleep == Sleep::Dream);

    // At the end of the dream, deep sleep: nothing the player does moves anything.
    run(w, kDreamSeconds - 10, still(w));
    CHECK(w.sleep == Sleep::Deep);
    CHECK(nextStep(w).empty());
    // The deep sleep's one line says it ends by itself (#37).
    CHECK(sleepNote(w).find("Deep sleep") == 0 && sleepNote(w).find("by itself") != std::string::npos);
    const float x1 = w.player.x, z1 = w.player.z;
    run(w, kDeepSleepSeconds - 1, walk);
    CHECK(w.player.x == x1 && w.player.z == z1);
    CHECK(w.sleep == Sleep::Deep);
    CHECK(!w.codex[static_cast<int>(CodexId::Dream)]);

    // Then the morning: both pages open, and the after-mode.
    run(w, 1, still(w));
    CHECK(w.sleep == Sleep::Awake && w.sleptTick == -1);
    CHECK(w.day == 1);
    CHECK(clockText(w) == "6:30 am");
    CHECK(w.codex[static_cast<int>(CodexId::Dream)] && w.codex[static_cast<int>(CodexId::Fourth)]);
    CHECK(teachingsDone(w) && w.after);
    CHECK(nextStep(w) == "Nothing more is asked of you");
    CHECK(sleepNote(w).empty());  // awake again, nothing to say

    // The next night is the cut to morning again.
    w.tick += static_cast<int64_t>(14 * 60 * kTickRate);  // 8:30 pm
    const Vec2 step = yourDoorstep();
    w.player.x = step.x;
    w.player.z = step.z;
    {
        Input in = still(w);
        in.use = true;
        tick(w, in);
    }
    CHECK(w.sleep == Sleep::Awake && w.day == 2);

    // The autopilot dreams and sleeps through it too, the same way twice.
    {
        World a = readyToSleep(), b = readyToSleep();
        Autopilot pa, pb;
        for (int i = 0; i < (kDreamSeconds + kDeepSleepSeconds + 120) * kTickRate; ++i) {
            tick(a, pa.next(a));
            tick(b, pb.next(b));
        }
        CHECK(a.codex[static_cast<int>(CodexId::Fourth)] && a.after);
        CHECK(fingerprint(a) == fingerprint(b));
    }

    // The after-mode's days: ordinary on its first, a lamp night on its
    // second, egrets on its third; after that a mix that follows from the
    // seed and the day, and nothing before the after-mode.
    {
        World d = newWorld(1);
        d.day = 2;
        CHECK(dayKind(d) == DayKind::Ordinary);  // not in the after-mode
        d.after = true;
        d.day = 1;
        CHECK(dayKind(d) == DayKind::Ordinary);
        d.day = 2;
        CHECK(dayKind(d) == DayKind::LampNight);
        d.day = 3;
        CHECK(dayKind(d) == DayKind::Egrets);
        int kinds[3] = {};
        for (d.day = 4; d.day < 404; ++d.day) ++kinds[static_cast<int>(dayKind(d))];
        CHECK(kinds[0] > kinds[1] && kinds[0] > kinds[2]);  // most days are ordinary
        CHECK(kinds[1] > 50 && kinds[2] > 50);
        World e = d;
        CHECK(dayKind(e) == dayKind(d));
    }

    TEST_MAIN_END();
}
