#include "audio/cues.h"

#include "sim/valley.h"

namespace atman::audio {

namespace V = atman::valley;

namespace {

int pages(const World& w) {
    int n = 0;
    for (bool c : w.codex) n += c ? 1 : 0;
    return n;
}

int broken(const World& w) {
    int n = 0;
    for (const Pot& p : w.pots) n += p.made && p.broken ? 1 : 0;
    return n;
}

int seenThrough(const World& w) {
    int n = 0;
    for (const Coil& c : w.coils) n += c.seenThrough ? 1 : 0;
    return n;
}

int done(const World& w) {
    int n = 0;
    for (bool q : w.questDone) n += q ? 1 : 0;
    return n;
}

// A bump of height 1 at `peak` minutes, `width` minutes wide on each side.
float around(float minute, float peak, float width) {
    const float d = (minute - peak) / width;
    return d > -1 && d < 1 ? 1 - d * d : 0;
}

}  // namespace

Ambience ambience(const World& w, Vec3 listener, float stepsPerSecond) {
    Ambience a;
    if (w.sleep == Sleep::Deep) {
        a.river = 0.6f;  // no valley and no body: only the river, from nowhere in particular
        return a;
    }
    // The river: full at the water's edge, half at 8 m, faint across the village.
    const float toWater = listener.x < V::kRiverWest ? V::kRiverWest - listener.x : 0.0f;
    a.river = 1.0f / (1.0f + (toWater / 8.0f) * (toWater / 8.0f));
    a.wind = clampf(0.25f + V::groundHeight(listener.x, listener.z) / 14.0f + (listener.y - 2.0f) / 40.0f, 0.0f, 1.0f);
    const float minute = static_cast<float>(static_cast<int>(skyMinutes(w)) % 1440);  // the dream's own evening
    if (isDusk(w)) {
        a.crickets = 1;
    } else {
        // Birds all day, busiest after dawn and before dusk.
        a.birds = clampf(0.35f + around(minute, 7 * 60, 90) + around(minute, 17 * 60 + 30, 60), 0.0f, 1.0f);
    }
    a.wheel = w.throwing.active ? 1.0f : 0.0f;
    a.heart = fearShown(w) ? clampf((w.fear - 0.15f) / 0.85f, 0.0f, 1.0f) : 0.0f;
    a.drone = w.after ? 1.0f : 0.0f;
    a.steps = stepsPerSecond;
    const V::Ghat& g = V::kGhat;
    a.stone = w.player.x >= g.x0 && w.player.x <= g.x1 && w.player.z >= g.z0 && w.player.z <= g.z1;
    return a;
}

std::string captions(const World& w, const Ambience& a) {
    std::vector<const char*> heard;
    if (w.sleep == Sleep::Deep) heard.push_back("the river");
    if (a.heart > 0.1f) heard.push_back("your heart pounding");
    if (a.wheel > 0) heard.push_back("the wheel turning");
    std::string out;
    for (const char* h : heard) out += (out.empty() ? "(" : ", ") + std::string(h);
    return out.empty() ? out : out + ")";
}

std::vector<Cue> cues(const World& before, const World& after) {
    std::vector<Cue> out;
    if (after.day > before.day) out.push_back(Cue::Wake);
    if (before.carry == Carry::Nothing && after.carry == Carry::Water) out.push_back(Cue::FillJar);
    if (before.carry == Carry::Nothing && after.carry == Carry::Clay) out.push_back(Cue::DigClay);
    if (done(after) > done(before)) out.push_back(Cue::ChoreDone);
    if (after.needs.thirst < 0.01f && before.needs.thirst >= 0.01f) out.push_back(Cue::Drink);
    if (after.needs.hunger < 0.01f && before.needs.hunger >= 0.01f && after.day == before.day) out.push_back(Cue::Eat);
    if (after.lampLit != before.lampLit && after.day == before.day && after.sleep == before.sleep)
        out.push_back(after.lampLit ? Cue::LampOn : Cue::LampOff);
    if (after.potsMade > before.potsMade) out.push_back(Cue::PotMade);
    if (broken(after) > broken(before)) out.push_back(Cue::PotBroken);
    if (after.clayLens != before.clayLens) out.push_back(Cue::Lens);
    if (seenThrough(after) > seenThrough(before)) out.push_back(Cue::Rope);
    if (pages(after) > pages(before)) out.push_back(Cue::Page);
    return out;
}

}  // namespace atman::audio
