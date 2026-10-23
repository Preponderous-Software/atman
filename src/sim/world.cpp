#include "sim/world.h"

#include <cstdio>
#include <cstring>

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

// Moves the player by (dx, dz), sliding along whatever blocks the way: each
// axis is tried on its own, so walking into a wall at an angle follows it.
void move(Player& p, float dx, float dz) {
    if (!valley::blocked(p.x + dx, p.z + dz)) {
        p.x += dx;
        p.z += dz;
        return;
    }
    if (!valley::blocked(p.x + dx, p.z)) p.x += dx;
    else if (!valley::blocked(p.x, p.z + dz)) p.z += dz;
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
    move(p, (sy * f + cy * s) * step, (-cy * f + sy * s) * step);

    const float minutes = static_cast<float>(kGameMinutesPerSecond / kTickRate);
    w.needs.thirst = clampf(w.needs.thirst + kThirstPerMinute * minutes, 0.0f, 1.0f);
    w.needs.hunger = clampf(w.needs.hunger + kHungerPerMinute * minutes, 0.0f, 1.0f);
    w.needs.tiredness = clampf(w.needs.tiredness + kTirednessPerMinute * minutes, 0.0f, 1.0f);
    ++w.tick;
}

double gameMinutes(const World& w) {
    return kStartMinute + static_cast<double>(w.tick) / kTickRate * kGameMinutesPerSecond;
}

std::string clockText(const World& w) {
    const int minute = static_cast<int>(gameMinutes(w)) % 1440;
    char buf[32];
    std::snprintf(buf, sizeof buf, "%d:%02d %s", (minute / 60 + 11) % 12 + 1, minute % 60, minute < 720 ? "am" : "pm");
    return buf;
}

const char* questText(QuestId q) {
    switch (q) {
        case QuestId::CarryWater: return "Carry water from the ghat to the potter";
        case QuestId::FetchClay: return "Fetch clay from the riverbank";
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
    return f.h;
}

}  // namespace atman
