// The game clock and the body's needs: time runs at one game minute per
// second of play from 4:30 pm, and needs rise with it and stay within 0..1.
#include <cmath>
#include <string>

#include "check.h"
#include "sim/world.h"

using namespace atman;

namespace {
void wait(World& w, int seconds) {
    for (int i = 0; i < seconds * kTickRate; ++i) tick(w, Input{});
}
}  // namespace

int main() {
    World w = newWorld(1);
    CHECK(gameMinutes(w) == kStartMinute);
    CHECK(clockText(w) == "4:30 pm");

    wait(w, 90);
    CHECK(std::fabs(gameMinutes(w) - (kStartMinute + 90)) < 1e-6);
    CHECK(clockText(w) == "6:00 pm");

    // Midnight, then morning.
    wait(w, 6 * 60);
    CHECK(clockText(w) == "12:00 am");
    wait(w, 6 * 60 + 15);
    CHECK(clockText(w) == "6:15 am");

    // Noon reads as pm, the minute before as am.
    World n = newWorld(1);
    n.tick = static_cast<int64_t>((12 * 60 - kStartMinute + 1440) * kTickRate);
    CHECK(clockText(n) == "12:00 pm");
    n.tick -= kTickRate;
    CHECK(clockText(n) == "11:59 am");

    // Needs rise over time and stop at 1.
    World a = newWorld(1);
    const Needs before = a.needs;
    wait(a, 60);
    CHECK(a.needs.thirst > before.thirst);
    CHECK(a.needs.hunger > before.hunger);
    CHECK(a.needs.tiredness > before.tiredness);
    CHECK(a.needs.thirst - before.thirst > a.needs.hunger - before.hunger);  // thirst fills fastest
    wait(a, 24 * 60);
    CHECK(a.needs.thirst == 1.0f && a.needs.hunger == 1.0f && a.needs.tiredness == 1.0f);

    // The HUD's words exist for every quest and standing.
    for (int i = 0; i < kQuestCount; ++i) CHECK(std::string(questText(static_cast<QuestId>(i))).size() > 10);
    CHECK(std::string(standingText(0)) == "Newcomer");

    TEST_MAIN_END();
}
