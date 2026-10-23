#include "sim/clay.h"

#include "sim/valley.h"

namespace atman {

namespace {

constexpr FormShape kForms[kFormCount] = {
    {"Water jar", 0.46f, 0.26f},
    {"Bowl", 0.14f, 0.24f},
    {"Cup", 0.16f, 0.09f},
    {"Lamp", 0.06f, 0.11f},
};

float horizontalDistance(const Player& p, Vec2 c) {
    const float dx = p.x - c.x, dz = p.z - c.z;
    return std::sqrt(dx * dx + dz * dz);
}

// Keeps printable ASCII, trims spaces at either end, and caps the length. An
// empty name falls back to the form's.
void setName(Pot& pot, const char* typed) {
    std::string s;
    for (const char* c = typed; *c && s.size() < static_cast<size_t>(kNameLength); ++c)
        if (*c >= 32 && *c <= 126 && !(s.empty() && *c == ' ')) s += *c;
    while (!s.empty() && s.back() == ' ') s.pop_back();
    if (s.empty()) s = formShape(pot.form).name;
    pot.name.fill('\0');
    for (size_t i = 0; i < s.size(); ++i) pot.name[i] = s[i];
}

// A small integer hash, so the motes are the same on every compiler.
uint32_t mix(uint32_t x) {
    x ^= x >> 16;
    x *= 0x7feb352dU;
    x ^= x >> 15;
    x *= 0x846ca68bU;
    x ^= x >> 16;
    return x;
}

float unit(uint32_t h) { return static_cast<float>(h >> 8) / 16777216.0f; }

std::array<Vec3, valley::kMoteCount> makeMotes() {
    std::array<Vec3, valley::kMoteCount> m{};
    const Box3& b = valley::kMoteSpace;
    for (int i = 0; i < valley::kMoteCount; ++i) {
        const auto k = static_cast<uint32_t>(i) * 3U;
        const float x = b.lo.x + (b.hi.x - b.lo.x) * unit(mix(k + 1));
        const float z = b.lo.z + (b.hi.z - b.lo.z) * unit(mix(k + 2));
        const float y = valley::groundHeight(x, z) + b.lo.y + (b.hi.y - b.lo.y) * unit(mix(k + 3));
        m[i] = {x, y, z};
    }
    return m;
}

}  // namespace

const FormShape& formShape(Form f) { return kForms[static_cast<int>(f)]; }

Vec3 potBase(int slot) {
    const Vec2 s = valley::boardSlot(slot);
    // The board is level, set on the highest ground under it.
    return {s.x, valley::boardTop(), s.z};
}

bool atWheel(const World& w) { return horizontalDistance(w.player, valley::kWheel) <= kWheelReach; }

bool lensAvailable(const World& w) { return w.potsMade >= kPotsForLens; }

int lookedAtPot(const World& w) {
    const Player& p = w.player;
    const Vec3 eye{p.x, valley::walkHeight(p.x, p.z) + valley::kEyeHeight, p.z};
    const Vec3 d = lookDirection(p.yaw, p.pitch);
    int best = -1;
    float bestT = 0;
    for (int i = 0; i < valley::kBoardSlots; ++i) {
        const Pot& pot = w.pots[i];
        if (!pot.made || pot.broken || i == w.naming) continue;
        if (horizontalDistance(p, valley::boardSlot(i)) > kPotReach) continue;
        const FormShape& f = formShape(pot.form);
        const Vec3 base = potBase(i);
        float t = 0;
        const float r = (f.height > f.radius ? f.height : f.radius) * 0.5f + 0.1f;
        if (!sightPasses(eye, d, {base.x, base.y + f.height / 2, base.z}, r, t)) continue;
        if (best < 0 || t < bestT) {
            best = i;
            bestT = t;
        }
    }
    return best;
}

std::vector<std::string> inventory(const World& w) {
    std::vector<std::string> names;
    for (const Pot& p : w.pots)
        if (p.made && !p.broken) names.emplace_back(p.name.data());
    return names;
}

const std::array<Vec3, valley::kMoteCount>& motes() {
    static const std::array<Vec3, valley::kMoteCount> m = makeMotes();
    return m;
}

void clayTick(World& w, const Input& in) {
    const float dt = 1.0f / kTickRate;
    if (w.naming >= 0) {
        if (in.name) {
            setName(w.pots[w.naming], in.name);
            w.naming = -1;
        }
    } else if (w.throwing.active) {
        // Walking away from the wheel lets the clay slump: start again.
        if (!atWheel(w)) {
            w.throwing = Throwing{};
        } else if ((w.throwing.seconds += dt) >= kThrowSeconds) {
            Pot& pot = w.pots[w.potsMade];
            pot.made = true;
            pot.form = w.throwing.form;
            w.naming = w.potsMade++;
            w.throwing = Throwing{};
        }
    } else if (in.throwForm >= 0 && in.throwForm < kFormCount && atWheel(w) && w.potsMade < valley::kBoardSlots) {
        w.throwing.active = true;
        w.throwing.form = static_cast<Form>(in.throwForm);
        w.throwing.seconds = 0;
    }

    if (in.breakPot) {
        const int slot = lookedAtPot(w);
        if (slot >= 0) {
            w.pots[slot].broken = true;
            w.codex[static_cast<int>(CodexId::PotSpace)] = true;
        }
    }
    if (in.toggleLens && lensAvailable(w)) {
        w.clayLens = !w.clayLens;
        w.codex[static_cast<int>(CodexId::ClayAndPots)] = true;
    }
}

}  // namespace atman
