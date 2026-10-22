// Determinism: the same seed and inputs give the same world, and the
// fingerprint of a scripted walk is pinned so any change to the rules that
// alters the outcome shows up here (update the pin deliberately, in the same
// PR as the rule change, and say why).
#include <cinttypes>
#include <cstdio>

#include "check.h"
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

}  // namespace

int main() {
    const World a = scriptedWalk(1), b = scriptedWalk(1);
    CHECK(fingerprint(a) == fingerprint(b));
    CHECK(fingerprint(newWorld(1)) != fingerprint(newWorld(2)));

    const uint64_t pinned = 0x7e805081d39d32dfULL;
    std::printf("scripted walk fingerprint: %016" PRIx64 " (player at %.3f, %.3f)\n", fingerprint(a), a.player.x,
                a.player.z);
    CHECK(fingerprint(a) == pinned);

    TEST_MAIN_END();
}
