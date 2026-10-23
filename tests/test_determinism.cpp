// Determinism: the same seed and inputs give the same world, and the
// fingerprints of two scripted walks (afternoon, and dusk on the path) are pinned so any change to the rules that
// alters the outcome shows up here (update the pin deliberately, in the same
// PR as the rule change, and say why).
#include <cinttypes>
#include <cmath>
#include <cstdio>

#include "check.h"
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

}  // namespace

int main() {
    const World a = scriptedWalk(1), b = scriptedWalk(1);
    CHECK(fingerprint(a) == fingerprint(b));
    CHECK(fingerprint(newWorld(1)) != fingerprint(newWorld(2)));

    // Pinned again in M2: the World now also holds the lamp, fear and the
    // coils, so the fingerprint covers them (the walk itself is unchanged).
    const uint64_t pinned = 0xec4a218f9b35df67ULL;
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
    CHECK(fingerprint(d) == 0xb578373a910f2437ULL);

    TEST_MAIN_END();
}
