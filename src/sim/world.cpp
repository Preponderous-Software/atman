#include "sim/world.h"

#include <cmath>
#include <cstdio>
#include <cstring>

#include "sim/chores.h"
#include "sim/clay.h"
#include "sim/valley.h"

namespace atman {

namespace {

constexpr float kWalkSpeed = 1.4f;   // metres per second
constexpr float kHurrySpeed = 3.2f;
constexpr float kMaxPitch = 1.45f;   // just short of straight up or down

// Per game minute: thirst fills in about six hours, hunger in ten, tiredness in fourteen.
constexpr float kThirstPerMinute = 1.0f / (6 * 60);
constexpr float kHungerPerMinute = 1.0f / (10 * 60);
constexpr float kTirednessPerMinute = 1.0f / (14 * 60);

// The Rope. Fear rises within kFearRadius of a snake, and nobody walks closer
// than kBalkRadius to one. The lamp reaches kLampReach; looking at a snake by
// its light for kGazeSeconds is enough to see what it is.
constexpr float kFearRadius = 9.0f;
constexpr float kBalkRadius = 1.4f;
constexpr float kLampReach = 3.5f;
constexpr float kGazeSeconds = 2.0f;
constexpr float kFearRisePerSecond = 0.6f;
constexpr float kFearFallPerSecond = 0.25f;
constexpr float kCoilTop = 0.1f;  // the height a look aims at, above the ground
constexpr float kCoilRadiusLook = valley::kCoilRadius + 0.2f;  // a little slack around the coil

float distanceTo(const Player& p, Vec2 c) {
    const float dx = p.x - c.x, dz = p.z - c.z;
    return std::sqrt(dx * dx + dz * dz);
}

// A step from (x0, z0) to (x1, z1) is refused if it takes the player closer to
// a snake that is already too close. Steps away are always allowed, so dusk
// falling with the player beside a coil never traps them.
bool balks(const World& w, float x0, float z0, float x1, float z1) {
    for (int i = 0; i < valley::kCoilCount; ++i) {
        if (!snakeVisible(w, i)) continue;
        const Vec2 c = valley::kCoils[i];
        const float d0 = (x0 - c.x) * (x0 - c.x) + (z0 - c.z) * (z0 - c.z);
        const float d1 = (x1 - c.x) * (x1 - c.x) + (z1 - c.z) * (z1 - c.z);
        if (d1 < kBalkRadius * kBalkRadius && d1 < d0) return true;
    }
    return false;
}

bool stopped(const World& w, float x, float z) {
    return valley::blocked(x, z) || balks(w, w.player.x, w.player.z, x, z);
}

// Moves the player by (dx, dz), sliding along whatever blocks the way: each
// axis is tried on its own, so walking into a wall at an angle follows it.
void move(World& w, float dx, float dz) {
    Player& p = w.player;
    if (!stopped(w, p.x + dx, p.z + dz)) {
        p.x += dx;
        p.z += dz;
        return;
    }
    if (!stopped(w, p.x + dx, p.z)) p.x += dx;
    else if (!stopped(w, p.x, p.z + dz)) p.z += dz;
}

// One tick of the chapter: fear follows the nearest snake, and lamplit looking
// sees through it. Nothing else changes when a snake becomes a rope: no score,
// no standing, no quest. That is the teaching (see MVP.md, "No reward").
void rope(World& w) {
    const float dt = 1.0f / kTickRate;
    if (w.fearRetired || !isDusk(w)) return;

    const int looked = w.lampLit ? lookedAtCoil(w) : -1;
    for (int i = 0; i < valley::kCoilCount; ++i) {
        Coil& c = w.coils[i];
        if (c.seenThrough) continue;
        c.gaze = i == looked ? c.gaze + dt : clampf(c.gaze - dt, 0.0f, kGazeSeconds);
        if (c.gaze >= kGazeSeconds) {
            c.seenThrough = true;
            c.gaze = 0;
            w.codex[static_cast<int>(CodexId::RopeAndSnake)] = true;
        }
    }

    float target = 0;
    bool anySnake = false;
    for (int i = 0; i < valley::kCoilCount; ++i) {
        if (!snakeVisible(w, i)) continue;
        anySnake = true;
        const float d = distanceTo(w.player, valley::kCoils[i]);
        const float closeness = clampf(1.0f - (d - kBalkRadius) / (kFearRadius - kBalkRadius), 0.0f, 1.0f);
        if (closeness > target) target = closeness;
    }
    if (!anySnake) {
        w.fear = 0;
        w.fearRetired = true;
        return;
    }
    if (w.fear < target) w.fear = clampf(w.fear + kFearRisePerSecond * dt, 0.0f, target);
    else w.fear = clampf(w.fear - kFearFallPerSecond * dt, target, 1.0f);
}

struct Fnv {
    uint64_t h = 1469598103934665603ULL;
    void bytes(const void* data, size_t n) {
        const auto* b = static_cast<const unsigned char*>(data);
        for (size_t i = 0; i < n; ++i) {
            h ^= b[i];
            h *= 1099511628211ULL;
        }
    }
    template <typename T>
    void add(T v) {
        bytes(&v, sizeof v);
    }
};

}  // namespace

World newWorld(uint64_t seed) {
    World w;
    w.seed = seed;
    w.player.x = valley::kStart.x;
    w.player.z = valley::kStart.z;
    w.player.yaw = valley::kStartYaw;
    return w;
}

void tick(World& w, const Input& in) {
    if (w.sleep == Sleep::Deep) {  // no body and no valley: nothing the player does reaches anything
        ++w.tick;
        sleepTick(w);
        return;
    }
    const bool awake = w.sleep == Sleep::Awake;
    Player& p = w.player;
    p.yaw = in.yaw;
    p.pitch = clampf(in.pitch, -kMaxPitch, kMaxPitch);

    float f = clampf(in.forward, -1.0f, 1.0f), s = clampf(in.strafe, -1.0f, 1.0f);
    const float len2 = f * f + s * s;
    if (len2 > 1.0f) {
        const float inv = 1.0f / std::sqrt(len2);
        f *= inv;
        s *= inv;
    }
    const float step = (in.hurry ? kHurrySpeed : kWalkSpeed) / kTickRate;
    const float sy = detSin(p.yaw), cy = detCos(p.yaw);
    // Forward is (sin yaw, -cos yaw) on the ground; right is (cos yaw, sin yaw).
    move(w, (sy * f + cy * s) * step, (-cy * f + sy * s) * step);
    ++w.tick;
    if (!awake) {  // in the dream the body walks and looks, and that is all
        sleepTick(w);
        return;
    }
    if (in.toggleLamp) w.lampLit = !w.lampLit;

    const float minutes = static_cast<float>(kGameMinutesPerSecond / kTickRate);
    w.needs.thirst = clampf(w.needs.thirst + kThirstPerMinute * minutes, 0.0f, 1.0f);
    w.needs.hunger = clampf(w.needs.hunger + kHungerPerMinute * minutes, 0.0f, 1.0f);
    w.needs.tiredness = clampf(w.needs.tiredness + kTirednessPerMinute * minutes, 0.0f, 1.0f);
    rope(w);
    clayTick(w, in);
    choresTick(w, in);
}

bool isDusk(const World& w) {
    const int minute = static_cast<int>(skyMinutes(w)) % 1440;
    return minute >= kDuskMinute || minute < kDawnMinute;
}

bool snakeVisible(const World& w, int i) { return isDusk(w) && !w.coils[i].seenThrough; }

bool fearShown(const World& w) { return isDusk(w) && !w.fearRetired; }

int lookedAtCoil(const World& w) {
    const Player& p = w.player;
    const Vec3 eye{p.x, valley::walkHeight(p.x, p.z) + valley::kEyeHeight, p.z};
    const Vec3 d = lookDirection(p.yaw, p.pitch);
    int best = -1;
    float bestT = 0;
    for (int i = 0; i < valley::kCoilCount; ++i) {
        const Vec2 c = valley::kCoils[i];
        if (distanceTo(p, c) > kLampReach) continue;
        float t = 0;
        if (!sightPasses(eye, d, {c.x, valley::groundHeight(c.x, c.z) + kCoilTop, c.z}, kCoilRadiusLook, t)) continue;
        if (best < 0 || t < bestT) {
            best = i;
            bestT = t;
        }
    }
    return best;
}

double gameMinutes(const World& w) {
    return kStartMinute + static_cast<double>(w.tick) / kTickRate * kGameMinutesPerSecond;
}

float sleptSeconds(const World& w) {
    return w.sleep == Sleep::Awake ? 0.0f : static_cast<float>(w.tick - w.sleptTick) / kTickRate;
}

double skyMinutes(const World& w) {
    if (w.sleep != Sleep::Dream) return gameMinutes(w);
    const double day = std::floor((gameMinutes(w) - kDawnMinute) / 1440.0) * 1440.0;  // the evening it began
    return day + kDreamStartMinute - sleptSeconds(w) * kDreamMinutesPerSecond;
}

std::string clockText(const World& w) {
    const int minute = static_cast<int>(skyMinutes(w)) % 1440;
    char buf[32];
    std::snprintf(buf, sizeof buf, "%d:%02d %s", (minute / 60 + 11) % 12 + 1, minute % 60, minute < 720 ? "am" : "pm");
    return buf;
}

const char* questText(QuestId q) {
    switch (q) {
        case QuestId::CarryWater: return "Water from the ghat to the potter";
        case QuestId::FetchClay: return "Clay from the bank to the wheel";
        case QuestId::Count: break;
    }
    return "";
}

const char* standingText(int32_t standing) {
    if (standing <= 0) return "Newcomer";
    if (standing < 5) return "Known by sight";
    return "One of the village";
}

uint64_t fingerprint(const World& w) {
    Fnv f;
    f.add(w.seed);
    f.add(w.tick);
    f.add(w.player.x);
    f.add(w.player.z);
    f.add(w.player.yaw);
    f.add(w.player.pitch);
    f.add(w.needs.thirst);
    f.add(w.needs.hunger);
    f.add(w.needs.tiredness);
    f.add(w.standing);
    for (bool d : w.questDone) f.add(d);
    f.add(w.lampLit);
    f.add(w.fear);
    f.add(w.fearRetired);
    for (const Coil& c : w.coils) {
        f.add(c.seenThrough);
        f.add(c.gaze);
    }
    for (bool c : w.codex) f.add(c);
    for (const Pot& p : w.pots) {
        f.add(p.made);
        f.add(p.broken);
        f.add(p.form);
        f.bytes(p.name.data(), p.name.size());
    }
    f.add(w.potsMade);
    f.add(w.throwing.active);
    f.add(w.throwing.form);
    f.add(w.throwing.seconds);
    f.add(w.naming);
    f.add(w.clayLens);
    f.add(w.carry);
    f.add(w.day);
    f.add(w.wokeTick);
    f.add(w.after);
    f.add(w.sleep);
    f.add(w.sleptTick);
    return f.h;
}

}  // namespace atman
