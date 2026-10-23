// The Rope: at dusk the coils on the path are seen as snakes; fear rises near
// them and nobody walks up to one; looking closely by lamplight for about two
// seconds sees each one as the rope it always was. Nothing is rewarded, and
// after the last snake the fear meter is gone for good.
#include <cmath>

#include "check.h"
#include "sim/valley.h"
#include "sim/world.h"

using namespace atman;
namespace V = atman::valley;

namespace {

// A world at (x, z), with the clock set to `minute`.
World at(float x, float z, int minute) {
    World w = newWorld(1);
    w.player.x = x;
    w.player.z = z;
    w.tick = static_cast<int64_t>((minute - kStartMinute) / kGameMinutesPerSecond) * kTickRate;
    return w;
}

// The view angles from the player's eyes to coil i.
void aim(const World& w, int i, float& yaw, float& pitch) {
    const Vec2 c = V::kCoils[i];
    const float dx = c.x - w.player.x, dz = c.z - w.player.z;
    const float dy = V::groundHeight(c.x, c.z) + 0.1f - (V::walkHeight(w.player.x, w.player.z) + V::kEyeHeight);
    yaw = std::atan2(dx, -dz);
    pitch = std::atan2(dy, std::sqrt(dx * dx + dz * dz));
}

// Stand still and look at coil i for `ticks`.
void lookAt(World& w, int i, int ticks) {
    for (int t = 0; t < ticks; ++t) {
        Input in;
        aim(w, i, in.yaw, in.pitch);
        tick(w, in);
    }
}

void lightLamp(World& w) {
    Input in;
    in.yaw = w.player.yaw;
    in.pitch = w.player.pitch;
    in.toggleLamp = true;
    tick(w, in);
}

void walk(World& w, float yaw, int ticks) {
    for (int t = 0; t < ticks; ++t) {
        Input in;
        in.yaw = yaw;
        in.forward = 1;
        tick(w, in);
    }
}

float distanceTo(const World& w, int i) {
    const float dx = w.player.x - V::kCoils[i].x, dz = w.player.z - V::kCoils[i].z;
    return std::sqrt(dx * dx + dz * dz);
}

constexpr int kDusk = kDuskMinute;
constexpr int kAfternoon = 17 * 60;

}  // namespace

int main() {
    // The coils lie on open ground, on the path, apart from each other.
    for (int i = 0; i < V::kCoilCount; ++i) {
        const Vec2 c = V::kCoils[i];
        CHECK(!V::blocked(c.x, c.z));
        CHECK(std::fabs(c.z - (4.0f - 0.2f * (c.x - 20.0f))) < 1.0f);
        for (int j = 0; j < i; ++j) {
            const float dx = c.x - V::kCoils[j].x, dz = c.z - V::kCoils[j].z;
            CHECK(dx * dx + dz * dz > 16.0f);
        }
    }

    // Dusk lasts the night; the next morning is light again.
    CHECK(!isDusk(at(0, 0, 18 * 60)));
    CHECK(isDusk(at(0, 0, kDuskMinute)));
    CHECK(isDusk(at(0, 0, 23 * 60)));
    CHECK(isDusk(at(0, 0, 24 * 60 + 5 * 60)));
    CHECK(!isDusk(at(0, 0, 24 * 60 + 7 * 60)));
    CHECK(isDusk(at(0, 0, 24 * 60 + kDuskMinute)));

    // In the afternoon there is nothing on the path, and nothing to fear.
    {
        World w = at(V::kCoils[0].x - 4, V::kCoils[0].z, kAfternoon);
        CHECK(!isDusk(w));
        CHECK(!fearShown(w));
        for (int i = 0; i < V::kCoilCount; ++i) CHECK(!snakeVisible(w, i));
        walk(w, kPi / 2, 5 * kTickRate);  // east, straight over the coil
        CHECK(w.fear == 0);
        CHECK(w.player.x > V::kCoils[0].x);
    }

    // At dusk the coils are snakes, the fear meter is on the HUD, and fear
    // rises as the player comes closer.
    {
        World w = at(V::kCoils[0].x - 8, V::kCoils[0].z, kDusk);
        lookAt(w, 0, 1);
        CHECK(isDusk(w));
        CHECK(fearShown(w));
        for (int i = 0; i < V::kCoilCount; ++i) CHECK(snakeVisible(w, i));
        lookAt(w, 0, 3 * kTickRate);
        const float farFear = w.fear;
        CHECK(farFear > 0);
        World n = at(V::kCoils[0].x - 2.5f, V::kCoils[0].z, kDusk);
        lookAt(n, 0, 3 * kTickRate);
        CHECK(n.fear > farFear);
        CHECK(n.fear <= 1.0f);
    }

    // Nobody walks up to a snake: walking straight at one stops short of it,
    // and walking away is always allowed, even from too close.
    {
        World w = at(V::kCoils[0].x - 5, V::kCoils[0].z, kDusk);
        walk(w, kPi / 2, 10 * kTickRate);
        CHECK(distanceTo(w, 0) > 1.3f);
        CHECK(distanceTo(w, 0) < 1.5f);
        World close = at(V::kCoils[0].x - 0.6f, V::kCoils[0].z, kDusk);
        walk(close, -kPi / 2, kTickRate);
        CHECK(distanceTo(close, 0) > 1.5f);
    }

    // Looking without the lamp changes nothing, however long.
    {
        World w = at(V::kCoils[0].x - 2.5f, V::kCoils[0].z, kDusk);
        lookAt(w, 0, 10 * kTickRate);
        CHECK(lookedAtCoil(w) == 0);
        CHECK(snakeVisible(w, 0));
    }

    // With the lamp, from too far away, nothing changes either.
    {
        World w = at(V::kCoils[0].x - 5, V::kCoils[0].z, kDusk);
        lightLamp(w);
        lookAt(w, 0, 10 * kTickRate);
        CHECK(lookedAtCoil(w) == -1);
        CHECK(snakeVisible(w, 0));
    }

    // Looking away from it, the lamp's light finds nothing.
    {
        World w = at(V::kCoils[0].x - 2.5f, V::kCoils[0].z, kDusk);
        lightLamp(w);
        for (int t = 0; t < 10 * kTickRate; ++t) {
            Input in;
            in.yaw = -kPi / 2;  // west, away from the coil
            tick(w, in);
        }
        CHECK(lookedAtCoil(w) == -1);
        CHECK(snakeVisible(w, 0));
    }

    // Close, by lamplight, for about two seconds: the snake is a rope. Just
    // short of that, it is still a snake.
    {
        World w = at(V::kCoils[0].x - 2.5f, V::kCoils[0].z, kDusk);
        lightLamp(w);
        lookAt(w, 0, 2 * kTickRate - 5);
        CHECK(snakeVisible(w, 0));
        lookAt(w, 0, 6);
        CHECK(!snakeVisible(w, 0));
        CHECK(w.coils[0].seenThrough);
        // The others are still snakes, so fear is still on the HUD.
        CHECK(snakeVisible(w, 1) && snakeVisible(w, 2));
        CHECK(fearShown(w));
        // The rope does not block the path: the player can walk over it.
        walk(w, kPi / 2, 5 * kTickRate);
        CHECK(w.player.x > V::kCoils[0].x + 1);
    }

    // Glancing away loses the look; it has to be held.
    {
        World w = at(V::kCoils[0].x - 2.5f, V::kCoils[0].z, kDusk);
        lightLamp(w);
        for (int k = 0; k < 6; ++k) {
            lookAt(w, 0, kTickRate);
            for (int t = 0; t < kTickRate; ++t) {
                Input in;
                in.yaw = -kPi / 2;
                tick(w, in);
            }
        }
        CHECK(snakeVisible(w, 0));
    }

    // No reward: seeing through a snake changes the coil, and records the
    // codex entry, and nothing else the player could count.
    {
        World seen = at(V::kCoils[0].x - 2.5f, V::kCoils[0].z, kDusk);
        lightLamp(seen);
        CHECK(!seen.codex[static_cast<int>(CodexId::RopeAndSnake)]);
        const int32_t standing = seen.standing;
        const auto quests = seen.questDone;
        const Needs needs = seen.needs;
        lookAt(seen, 0, 3 * kTickRate);
        CHECK(seen.coils[0].seenThrough);
        CHECK(seen.codex[static_cast<int>(CodexId::RopeAndSnake)]);
        CHECK(seen.standing == standing);
        CHECK(seen.questDone == quests);
        // The body's needs went on exactly as they would have anyway.
        World unseen = at(V::kCoils[0].x - 2.5f, V::kCoils[0].z, kDusk);
        lightLamp(unseen);
        for (int t = 0; t < 3 * kTickRate; ++t) {
            Input in;
            in.yaw = -kPi / 2;
            tick(unseen, in);
        }
        CHECK(seen.needs.thirst == unseen.needs.thirst && seen.needs.hunger == unseen.needs.hunger &&
              seen.needs.tiredness == unseen.needs.tiredness);
        CHECK(seen.needs.thirst > needs.thirst);
    }

    // After the last snake, the fear meter leaves the HUD and does not come
    // back: not later that night, and not the next evening.
    {
        World w = at(V::kCoils[0].x, V::kCoils[0].z + 2.5f, kDusk);
        lightLamp(w);
        for (int i = 0; i < V::kCoilCount; ++i) {
            w.player.x = V::kCoils[i].x + 2.5f;
            w.player.z = V::kCoils[i].z;
            CHECK(fearShown(w));
            lookAt(w, i, 3 * kTickRate);
            CHECK(w.coils[i].seenThrough);
        }
        CHECK(w.fearRetired);
        CHECK(!fearShown(w));
        CHECK(w.fear == 0);
        for (int i = 0; i < V::kCoilCount; ++i) CHECK(!snakeVisible(w, i));
        w.tick += static_cast<int64_t>(24 * 60 / kGameMinutesPerSecond) * kTickRate;
        lookAt(w, 0, kTickRate);
        CHECK(!fearShown(w));
        CHECK(w.fear == 0);
    }

    // The lamp key lights the lamp and puts it out again.
    {
        World w = newWorld(1);
        CHECK(!w.lampLit);
        lightLamp(w);
        CHECK(w.lampLit);
        lightLamp(w);
        CHECK(!w.lampLit);
    }

    TEST_MAIN_END();
}
