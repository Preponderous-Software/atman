// Walking: the player moves the way they face, at walking or hurrying pace,
// follows the ground, and is stopped by houses, the well and the river.
#include <cmath>

#include "check.h"
#include "sim/valley.h"
#include "sim/world.h"

using namespace atman;
namespace V = atman::valley;

namespace {

void walk(World& w, float yaw, float forward, int ticks, bool hurry = false) {
    for (int i = 0; i < ticks; ++i) {
        Input in;
        in.yaw = yaw;
        in.forward = forward;
        in.hurry = hurry;
        tick(w, in);
    }
}

World at(float x, float z) {
    World w = newWorld(1);
    w.player.x = x;
    w.player.z = z;
    return w;
}

}  // namespace

int main() {
    // A new game starts outside your house, facing east, on open ground.
    const World start = newWorld(1);
    CHECK(start.player.x == V::kStart.x && start.player.z == V::kStart.z);
    CHECK(!V::blocked(start.player.x, start.player.z));

    // Walking east for one second covers the walking pace; hurrying covers more.
    {
        World w = newWorld(1);
        walk(w, kPi / 2, 1, kTickRate);
        CHECK(std::fabs(w.player.x - (V::kStart.x + 1.4f)) < 0.01f);
        CHECK(std::fabs(w.player.z - V::kStart.z) < 0.01f);
        World h = newWorld(1);
        walk(h, kPi / 2, 1, kTickRate, true);
        CHECK(h.player.x - V::kStart.x > 3.0f);
    }

    // Yaw 0 faces north (-z); strafing right while facing north goes east.
    {
        World w = at(0, 20);
        walk(w, 0, 1, kTickRate);
        CHECK(w.player.z < 20 - 1.3f);
        World s = at(0, 20);
        for (int i = 0; i < kTickRate; ++i) {
            Input in;
            in.strafe = 1;
            tick(s, in);
        }
        CHECK(s.player.x > 1.3f && std::fabs(s.player.z - 20) < 0.01f);
    }

    // Diagonal input is no faster than straight ahead.
    {
        World w = at(0, 20);
        for (int i = 0; i < kTickRate; ++i) {
            Input in;
            in.forward = 1;
            in.strafe = 1;
            tick(w, in);
        }
        const float d = std::sqrt(w.player.x * w.player.x + (w.player.z - 20) * (w.player.z - 20));
        CHECK(d < 1.41f);
    }

    // Walking north into your house stops at its wall.
    {
        World w = at(-8, 14);
        walk(w, 0, 1, 10 * kTickRate);
        CHECK(w.player.z >= V::kHouses[V::YourHouse].footprint.z1 + V::kPlayerRadius - 0.05f);
        CHECK(w.player.z < 13);
    }

    // Walking into a wall at an angle slides along it.
    {
        World w = at(-8, 13);
        walk(w, 0.6f, 1, 3 * kTickRate);
        CHECK(w.player.x > -6.5f);
        CHECK(w.player.z > V::kHouses[V::YourHouse].footprint.z1);
    }

    // The well is solid.
    {
        World w = at(V::kWell.x, V::kWell.z + 4);
        walk(w, 0, 1, 10 * kTickRate);
        CHECK(w.player.z > V::kWell.z + V::kWellRadius);
    }

    // The river stops you at the water's edge, and the ghat's lowest step is
    // still dry enough to stand on.
    {
        World w = at(20, 20);
        walk(w, kPi / 2, 1, 30 * kTickRate);
        CHECK(w.player.x < V::kRiverWest + 1.0f);
        CHECK(V::walkHeight(w.player.x, w.player.z) >= V::kWaterY);
        World g = at(22, 0);
        walk(g, kPi / 2, 1, 15 * kTickRate);
        CHECK(g.player.x > V::kGhat.x1 - V::kGhat.stepDepth());
        CHECK(V::walkHeight(g.player.x, g.player.z) > V::kWaterY);
    }

    // The ghat steps go down toward the water.
    {
        const V::Ghat& gh = V::kGhat;
        float last = 1e9f;
        for (int i = 0; i < gh.steps; ++i) {
            const float x = gh.x0 + gh.stepDepth() * (static_cast<float>(i) + 0.5f);
            const float h = V::walkHeight(x, 0);
            CHECK(h < last);
            last = h;
        }
        CHECK(last > V::kWaterY);
    }

    // The edge of the world holds.
    {
        World w = at(60, 60);
        walk(w, kPi * 0.75f, 1, 60 * kTickRate, true);
        CHECK(w.player.x <= V::kWalkLimit && w.player.z <= V::kWalkLimit);
    }

    // Pitch is limited to just short of straight up and down.
    {
        World w = newWorld(1);
        Input in;
        in.pitch = 3;
        tick(w, in);
        CHECK(w.player.pitch < 1.5f);
    }

    TEST_MAIN_END();
}
