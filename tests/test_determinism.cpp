// Determinism: the same seed and inputs give the same world, and the
// fingerprints of three scripted runs (an afternoon walk, dusk on the path,
// and the potter's wheel) are pinned so any change to the rules that
// alters the outcome shows up here (update the pin deliberately, in the same
// PR as the rule change, and say why).
#include <cinttypes>
#include <cmath>
#include <cstdio>

#include "check.h"
#include "sim/clay.h"
#include "sim/valley.h"
#include "sim/world.h"

using namespace atman;

namespace {

// Wander from your door toward the river, turning and hurrying now and then,
// bumping into houses on the way.
World scriptedWalk(uint64_t seed) {
    World w = newWorld(seed);
    for (int i = 0; i < 90 * kTickRate; ++i) {
        const float t = static_cast<float>(i) / kTickRate;
        Input in;
        in.forward = i % 300 < 250 ? 1.0f : -0.5f;
        in.strafe = i % 90 < 45 ? 0.35f : -0.35f;
        in.hurry = i % 200 < 40;
        in.yaw = 1.2f + 0.8f * detSin(t * 0.21f);
        in.pitch = 0.3f * detCos(t * 0.5f);
        tick(w, in);
    }
    return w;
}

// At dusk with the lamp lit, walk up the path from the village toward the
// ghat, turning toward the nearest snake with a wobble: fear, balking and
// lamplit looking all run.
World scriptedDusk(uint64_t seed) {
    World w = newWorld(seed);
    w.player.x = 7.0f;
    w.player.z = 6.6f;
    w.tick = static_cast<int64_t>((kDuskMinute + 5 - kStartMinute) / kGameMinutesPerSecond) * kTickRate;
    for (int i = 0; i < 60 * kTickRate; ++i) {
        const float t = static_cast<float>(i) / kTickRate;
        int nearest = -1;
        for (int c = 0; c < valley::kCoilCount && nearest < 0; ++c)
            if (snakeVisible(w, c)) nearest = c;
        Input in;
        in.forward = i % 120 < 70 ? 1.0f : 0.0f;
        in.strafe = 0.15f * detSin(t * 0.7f);
        in.yaw = 1.37f;
        in.pitch = -0.3f;
        if (nearest >= 0) {
            const Vec2 c = valley::kCoils[nearest];
            const float dx = c.x - w.player.x, dz = c.z - w.player.z;
            in.yaw = detAtan2(dx, -dz) + 0.15f * detSin(t * 1.3f);
            in.pitch = -detAtan2(1.5f, std::sqrt(dx * dx + dz * dz)) + 0.08f * detCos(t * 0.9f);
        }
        in.toggleLamp = i == 10;
        tick(w, in);
    }
    return w;
}

// At the wheel: throw and name three pots, look through the clay lens, then
// walk along the board and break the water jar.
World scriptedClay(uint64_t seed) {
    World w = newWorld(seed);
    w.player.x = 9.0f;
    w.player.z = -12.2f;
    const char* names[] = {"Ganga water", "Rice", "Evening lamp"};
    for (int i = 0; i < 40 * kTickRate; ++i) {
        const float t = static_cast<float>(i) / kTickRate;
        Input in;
        in.yaw = 0.3f + 0.2f * detSin(t * 0.8f);
        in.pitch = -0.6f;
        const int step = i / (5 * kTickRate), at = i % (5 * kTickRate);
        if (step < 3 && at == 0) in.throwForm = step == 0 ? 0 : step == 1 ? 1 : 3;
        if (step < 3 && at == 4 * kTickRate) in.name = names[step];
        in.toggleLens = i == 16 * kTickRate || i == 18 * kTickRate;
        if (i >= 20 * kTickRate) {
            // Step east toward the jar in slot 0, then look down at it.
            const Vec3 b = potBase(0);
            const float dx = b.x - w.player.x, dz = b.z - w.player.z;
            in.forward = w.player.x < 10.0f ? 0.6f : 0.0f;
            in.yaw = w.player.x < 10.0f ? kPi / 2 : detAtan2(dx, -dz);
            in.pitch = w.player.x < 10.0f ? -0.6f : -detAtan2(1.3f, std::sqrt(dx * dx + dz * dz));
            in.breakPot = i == 30 * kTickRate;
        }
        tick(w, in);
    }
    return w;
}

}  // namespace

int main() {
    const World a = scriptedWalk(1), b = scriptedWalk(1);
    CHECK(fingerprint(a) == fingerprint(b));
    CHECK(fingerprint(newWorld(1)) != fingerprint(newWorld(2)));

    // Pinned again in M2 and M3: the World now also holds the lamp, fear, the
    // coils and the pots, so the fingerprint covers them (the walk itself is
    // unchanged). Pinned again for chapter 3, which added the sleep's state
    // and two codex pages to the World; the three scripted runs end where
    // they did under 0.1.2 (the same positions, fear and pots).
    const uint64_t pinned = 0xfc7ee28e8f0d40f1ULL;
    std::printf("scripted walk fingerprint: %016" PRIx64 " (player at %.3f, %.3f)\n", fingerprint(a), a.player.x,
                a.player.z);
    CHECK(fingerprint(a) == pinned);

    const World d = scriptedDusk(1);
    CHECK(fingerprint(d) == fingerprint(scriptedDusk(1)));
    int seen = 0;
    for (const Coil& c : d.coils) seen += c.seenThrough ? 1 : 0;
    std::printf("scripted dusk fingerprint: %016" PRIx64 " (player at %.3f, %.3f, fear %.3f, %d of %d seen through)\n",
                fingerprint(d), d.player.x, d.player.z, d.fear, seen, valley::kCoilCount);
    CHECK(seen > 0);  // the walk does see through at least one snake
    CHECK(d.fearRetired);
    CHECK(fingerprint(d) == 0xb359090000a90761ULL);

    const World c = scriptedClay(1);
    CHECK(fingerprint(c) == fingerprint(scriptedClay(1)));
    std::printf("scripted clay fingerprint: %016" PRIx64 " (%d pots, lens %d, jar broken %d, player at %.3f, %.3f)\n",
                fingerprint(c), c.potsMade, c.clayLens ? 1 : 0, c.pots[0].broken ? 1 : 0, c.player.x, c.player.z);
    CHECK(c.potsMade == 3);
    CHECK(c.pots[0].broken && !c.pots[1].broken && !c.pots[2].broken);
    CHECK(fingerprint(c) == 0x688bd8f1422f2ac8ULL);

    TEST_MAIN_END();
}
