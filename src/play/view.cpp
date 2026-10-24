#include "play/view.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>

#include "sim/chores.h"
#include "sim/clay.h"
#include "sim/days.h"
#include "sim/valley.h"

namespace atman::play {

namespace V = atman::valley;

namespace {

constexpr float kFovY = 70.0f;
// The HUD's text size and line height, at the player's text size (F2; see
// setTextScale).
float gTextSize = 16.0f;
float gLine = 20.0f;
constexpr float kMargin = 16.0f;
constexpr float kLampRange = 16.0f;
constexpr float kLampIntensity = 3.0f;
constexpr bridge::Rgb kLampColor{1.0f, 0.62f, 0.26f};
constexpr float kDoorLampRange = 9.0f, kDoorLampIntensity = 1.6f;
constexpr float kNightFill = 0.9f;
constexpr float kFlySpeed = 4.0f, kFlyFast = 12.0f;  // metres per second
constexpr float kLabelRange = 16.0f;       // places are named within this many metres
constexpr float kHouseLabelRange = 30.0f;
constexpr float kLabelNear = 3.0f;  // houses, by their roofs, from further
constexpr float kDarkSeconds = 0.6f, kFadeSeconds = 2.4f;  // waking: dark, then the morning fades in
constexpr float kDreamFadeSeconds = 4.0f;  // the dream goes out over its last seconds

StrataCamera lookFrom(Vec3 eye, float yaw, float pitch, float aspect) {
    const Vec3 d = lookDirection(yaw, pitch);
    const float e[3] = {eye.x, eye.y, eye.z};
    const float target[3] = {eye.x + d.x, eye.y + d.y, eye.z + d.z};
    const float up[3] = {0, 1, 0};
    return strata::cameraLookAt(e, target, up, kFovY, aspect, 0.05f, 400.0f);
}

// How dark the screen still is after waking: 1 is black.
float wakeDarkness(const World& w) {
    if (w.wokeTick < 0) return 0;
    const float s = static_cast<float>(w.tick - w.wokeTick) / kTickRate;
    if (s < kDarkSeconds) return 1;
    return clampf(1.0f - (s - kDarkSeconds) / kFadeSeconds, 0.0f, 1.0f);
}

// Waking's dark, the dream going out over its last seconds, and deep sleep,
// which is all dark.
float darkness(const World& w) {
    float dark = wakeDarkness(w);
    if (w.sleep == Sleep::Deep) return 1;
    if (w.sleep == Sleep::Dream) {
        const float left = static_cast<float>(kDreamSeconds) - sleptSeconds(w);
        dark = std::max(dark, clampf(1.0f - left / kDreamFadeSeconds, 0.0f, 1.0f));
    }
    return dark;
}

// One black point big enough to cover the screen, just in front of the eye.
void drawDarkness(strata::Engine& engine, Vec3 eye, float yaw, float pitch, float alpha, int32_t width,
                  int32_t height) {
    const Vec3 d = lookDirection(yaw, pitch);
    StrataPoint p{};
    p.position[0] = eye.x + d.x * 0.2f;
    p.position[1] = eye.y + d.y * 0.2f;
    p.position[2] = eye.z + d.z * 0.2f;
    p.size_pixels = 3.0f * static_cast<float>(width > height ? width : height);
    p.color[3] = alpha;
    engine.submitPoints(&p, 1);
}

void text(std::vector<StrataText>& out, std::vector<std::string>& keep, float x, float y, const std::string& s,
          const float color[4]) {
    keep.push_back(s);
    StrataText t{};
    t.x = x;
    t.y = y;
    t.pixel_size = gTextSize;
    t.flags = STRATA_TEXT_SHADOW;
    for (int i = 0; i < 4; ++i) t.color[i] = color[i];
    out.push_back(t);
}

std::string lower(const char* s) {
    std::string out = s;
    for (char& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

// The motes over the drying board: soft, warm, twinkling in place. Only their
// brightness changes with time; where they are never does.
void drawMotes(strata::Engine& engine, const World& w) {
    const auto& m = motes();
    std::vector<StrataPoint> points(m.size());
    const float t = static_cast<float>(w.tick) / kTickRate;
    for (size_t i = 0; i < m.size(); ++i) {
        StrataPoint& p = points[i];
        p.position[0] = m[i].x;
        p.position[1] = m[i].y;
        p.position[2] = m[i].z;
        p.size_pixels = 2.5f;
        p.world_diameter = 0.035f;
        const float twinkle = 0.5f + 0.5f * std::sin(t * 1.7f + static_cast<float>(i) * 2.39f);
        p.color[0] = 1.0f;
        p.color[1] = 0.9f;
        p.color[2] = 0.7f;
        p.color[3] = 0.2f + 0.45f * twinkle;
        p.flags = STRATA_POINT_SOFT;
    }
    engine.submitPoints(points.data(), points.size());
}

// Where a house's doorway lamp hangs: beside the door, under the eave.
Vec3 doorLamp(const V::House& h) {
    static const float out[4][2] = {{0, -0.5f}, {0.5f, 0}, {0, 0.5f}, {-0.5f, 0}};  // off the wall, by face
    const float x = h.door.x + out[h.doorFace][0], z = h.door.z + out[h.doorFace][1];
    return {x, V::groundHeight(h.door.x, h.door.z) + 2.4f, z};
}

// 0 by day, rising to 1 as the sun goes 6 degrees below the horizon.
float nightness(float sunElevation) { return clampf(-sunElevation / 6.0f, 0.0f, 1.0f); }

// The doorway lamps' flames, seen from across the village.
void drawDoorLamps(strata::Engine& engine, float night) {
    if (night <= 0) return;
    std::vector<StrataPoint> flames;
    for (const V::House& h : V::kHouses) {
        const Vec3 at = doorLamp(h);
        StrataPoint p{};
        p.position[0] = at.x;
        p.position[1] = at.y;
        p.position[2] = at.z;
        p.size_pixels = 3.0f;
        p.world_diameter = 0.22f;
        p.color[0] = 1.0f;
        p.color[1] = 0.75f;
        p.color[2] = 0.35f;
        p.color[3] = 0.9f * night;
        p.flags = STRATA_POINT_SOFT;
        flames.push_back(p);
    }
    engine.submitPoints(flames.data(), flames.size());
}

// The lamp night: small clay lamps along four of the ghat's steps, and more
// floating down the river from its foot, from just after sunset until dawn.
bool lampNight(const World& w) {
    const int minute = static_cast<int>(skyMinutes(w)) % 1440;
    return dayKind(w) == DayKind::LampNight && (minute >= kLampsLitMinute || minute < kDawnMinute);
}

std::vector<Vec3> ghatLamps(const World& w) {
    std::vector<Vec3> out;
    const V::Ghat& g = V::kGhat;
    for (int i = 1; i < g.steps; i += 2) {
        const float x = g.x0 + g.stepDepth() * (static_cast<float>(i) + 0.8f);
        for (float z = -6.0f; z <= 6.01f; z += 1.5f) out.push_back({x, g.stepHeight(i) + 0.06f, z});
    }
    // Floating lamps: each is set on the water at the foot of the steps and
    // drifts downstream (south) and out, then the next is set in its place.
    const float minutes = static_cast<float>(skyMinutes(w) - std::floor(skyMinutes(w) / 1440) * 1440);
    const float since = minutes >= kLampsLitMinute ? minutes - kLampsLitMinute : minutes + 1440 - kLampsLitMinute;
    constexpr int kFloating = 14;
    constexpr float kCycle = 70;  // game minutes from the steps to out of sight
    for (int k = 0; k < kFloating; ++k) {
        const float t = since - static_cast<float>(k) * kCycle / kFloating;
        if (t < 0) continue;
        const float age = std::fmod(t, kCycle);
        const float x = g.x1 + 0.5f + 0.09f * age + 0.3f * std::sin(age * 0.21f + static_cast<float>(k));
        const float z = -3.0f + static_cast<float>(k % 5) * 1.5f + 0.9f * age;
        if (x < V::kRiverEast - 1.0f) out.push_back({x, V::kWaterY + 0.05f, z});
    }
    return out;
}

void drawGhatLamps(strata::Engine& engine, const std::vector<Vec3>& lamps, float flicker) {
    std::vector<StrataPoint> flames;
    for (size_t i = 0; i < lamps.size(); ++i) {
        StrataPoint p{};
        p.position[0] = lamps[i].x;
        p.position[1] = lamps[i].y;
        p.position[2] = lamps[i].z;
        p.size_pixels = 4.0f;
        p.world_diameter = 0.16f;
        p.color[0] = 1.0f;
        p.color[1] = 0.72f;
        p.color[2] = 0.3f;
        p.color[3] = 0.75f + 0.2f * std::sin(flicker * 7.3f + static_cast<float>(i) * 1.7f);
        p.flags = STRATA_POINT_SOFT;
        flames.push_back(p);
    }
    engine.submitPoints(flames.data(), flames.size());
}

// The egrets' morning: a flock of nine in a loose V crossing the valley from
// the south-east to the north-west, high over the river and the village.
void drawEgrets(strata::Engine& engine, const World& w) {
    if (dayKind(w) != DayKind::Egrets) return;
    const float minute = static_cast<float>(std::fmod(skyMinutes(w), 1440.0));
    if (minute < kEgretsFromMinute || minute > kEgretsToMinute) return;
    const float p = (minute - kEgretsFromMinute) / static_cast<float>(kEgretsToMinute - kEgretsFromMinute);
    const Vec3 from{60.0f, 9.0f, 70.0f}, to{-50.0f, 11.0f, -70.0f};
    const Vec3 lead{from.x + (to.x - from.x) * p, from.y + (to.y - from.y) * p + 1.5f * std::sin(p * kPi), from.z + (to.z - from.z) * p};
    // Back along the flight, and to either side.
    const float len = std::sqrt((to.x - from.x) * (to.x - from.x) + (to.z - from.z) * (to.z - from.z));
    const float bx = -(to.x - from.x) / len, bz = -(to.z - from.z) / len;
    std::vector<StrataPoint> birds;
    for (int i = 0; i < 9; ++i) {
        const int row = (i + 1) / 2;
        const float side = i == 0 ? 0.0f : (i % 2 ? 1.0f : -1.0f);
        StrataPoint b{};
        b.position[0] = lead.x + bx * 1.6f * static_cast<float>(row) - bz * side * 1.4f * static_cast<float>(row);
        b.position[1] = lead.y - 0.2f * static_cast<float>(row);
        b.position[2] = lead.z + bz * 1.6f * static_cast<float>(row) + bx * side * 1.4f * static_cast<float>(row);
        const float flap = 0.5f + 0.5f * std::sin(minute * 40.0f + static_cast<float>(i) * 0.9f);
        b.size_pixels = 3.0f;
        b.world_diameter = 0.6f + 0.35f * flap;  // an egret's wings span about a metre
        b.color[0] = b.color[1] = b.color[2] = 0.97f;
        b.color[3] = 1.0f;
        birds.push_back(b);
    }
    engine.submitPoints(birds.data(), birds.size());
}

// A place the HUD names when the player is near it and can see it.
struct Place {
    const char* name;
    Vec3 at;                    // where the name hangs
    float range;                // named within this many metres
    const Box2* own = nullptr;  // a house's own walls do not hide its name
};

std::vector<Place> places(const World& w) {
    std::vector<Place> out;
    for (const V::House& h : V::kHouses) {
        const Box2& f = h.footprint;
        const float x = (f.x0 + f.x1) / 2, z = (f.z0 + f.z1) / 2;
        out.push_back({h.name, {x, V::groundHeight(x, z) + h.wallHeight + h.roofRise + 1.0f, z}, kHouseLabelRange, &f});
    }
    out.push_back({"The well", {V::kWell.x, V::groundHeight(V::kWell.x, V::kWell.z) + 2.4f, V::kWell.z}, kLabelRange});
    out.push_back({"The potter's wheel", {V::kWheel.x, V::groundHeight(V::kWheel.x, V::kWheel.z) + 1.6f, V::kWheel.z}, kLabelRange});
    out.push_back({"Clay bank", {V::kClayBank.x, V::groundHeight(V::kClayBank.x, V::kClayBank.z) + 1.4f, V::kClayBank.z}, kLabelRange});
    out.push_back({"The ghat", {V::kGhat.x0 + 1.0f, V::kGhat.top + 1.8f, 0.0f}, kLabelRange});
    // In the dream each place has the next one's name, and nothing about that
    // seems strange (CONCEPT.md, chapter 4: the potter is also your mother).
    if (w.sleep == Sleep::Dream) {
        const char* first = out.front().name;
        for (size_t i = 0; i + 1 < out.size(); ++i) out[i].name = out[i + 1].name;
        out.back().name = first;
    }
    return out;
}

// True if nothing solid stands between the eye and a place (the place's own
// edge, a well's rim or a house's walls, does not count).
bool inSight(Vec3 eye, const Place& place) {
    const float dx = place.at.x - eye.x, dz = place.at.z - eye.z;
    const float d = std::sqrt(dx * dx + dz * dz);
    for (float s = 0.5f; s < d - 1.6f; s += 0.25f) {
        const float x = eye.x + dx * s / d, z = eye.z + dz * s / d;
        const Box2* f = place.own;
        if (f && x > f->x0 - 1 && x < f->x1 + 1 && z > f->z0 - 1 && z < f->z1 + 1) break;  // reached the house
        if (V::blocked(x, z)) return false;
    }
    return true;
}

// Where a world point lands on the screen, in pixels; false if behind the eye.
bool toScreen(const StrataCamera& cam, Vec3 p, int32_t width, int32_t height, float& x, float& y) {
    float v[4], c[4];
    const float in[4] = {p.x, p.y, p.z, 1};
    for (int r = 0; r < 4; ++r) v[r] = cam.view[r] * in[0] + cam.view[4 + r] * in[1] + cam.view[8 + r] * in[2] + cam.view[12 + r] * in[3];
    for (int r = 0; r < 4; ++r)
        c[r] = cam.projection[r] * v[0] + cam.projection[4 + r] * v[1] + cam.projection[8 + r] * v[2] + cam.projection[12 + r] * v[3];
    if (c[3] <= 0.1f) return false;
    x = (c[0] / c[3] * 0.5f + 0.5f) * static_cast<float>(width);
    y = (0.5f - c[1] / c[3] * 0.5f) * static_cast<float>(height);
    return x > 0 && x < static_cast<float>(width) && y > 0 && y < static_cast<float>(height);
}

}  // namespace

void setLighting(strata::Engine& engine, bool lightweight) {
    StrataLighting light = strata::lightingDefaults(STRATA_LIGHTING_SUN);
    light.cascade_count = lightweight ? 3 : 4;
    light.shadow_map_size = lightweight ? 1024 : 2048;
    light.shadow_distance = 80.0f;
    light.fog = 1;
    engine.setLighting(light);
}

float setSky(strata::Engine& engine, const World& w) {
    const StrataSolarPosition sun =
        strata::solarPosition(kFirstMidnightUtc + skyMinutes(w) * 60.0, kLatitude, kLongitude);
    engine.setSunDirection(sun.to_sun);
    return sun.elevation_degrees;
}

void setLights(strata::Engine& engine, const World& w, float sunElevation) {
    std::vector<StrataPointLight> lights;
    auto add = [&](Vec3 at, float range, bridge::Rgb color, float intensity) {
        StrataPointLight l{};
        l.position[0] = at.x;
        l.position[1] = at.y;
        l.position[2] = at.z;
        l.range = range;
        l.color[0] = color.r;
        l.color[1] = color.g;
        l.color[2] = color.b;
        l.intensity = intensity;
        lights.push_back(l);
    };
    if (w.lampLit) {
        // Held at the shoulder and a little ahead, on the right.
        const Player& p = w.player;
        const float sy = detSin(p.yaw), cy = detCos(p.yaw);
        add({p.x + 0.35f * sy + 0.25f * cy, V::walkHeight(p.x, p.z) + V::kEyeHeight - 0.3f, p.z - 0.35f * cy + 0.25f * sy},
            kLampRange, kLampColor, kLampIntensity);
    }
    // After sunset the village lights its doorways, and a faint fill from the
    // sky keeps the shapes of things (the engine's moon alone leaves them black).
    const float night = nightness(sunElevation);
    if (night > 0) {
        for (const V::House& h : V::kHouses) add(doorLamp(h), kDoorLampRange, kLampColor, kDoorLampIntensity * night);
        add({8.0f, 40.0f, 0.0f}, 120.0f, {0.55f, 0.65f, 1.0f}, kNightFill * night);
    }
    // The lamp night's steps glow.
    if (lampNight(w))
        for (float z : {-4.5f, 0.0f, 4.5f}) add({V::kGhat.x0 + 4.0f, 0.6f, z}, 7.0f, kLampColor, 1.2f);
    engine.setPointLights(lights.empty() ? nullptr : lights.data(), static_cast<uint32_t>(lights.size()));
}

StrataCamera playerCamera(const World& w, float aspect) {
    const Player& p = w.player;
    return lookFrom({p.x, V::walkHeight(p.x, p.z) + V::kEyeHeight, p.z}, p.yaw, p.pitch, aspect);
}

FreeCamera cameraAtEyes(const World& w) {
    const Player& p = w.player;
    return {{p.x, V::walkHeight(p.x, p.z) + V::kEyeHeight, p.z}, p.yaw, p.pitch};
}

FreeCamera cameraBehind(const World& w) {
    const Player& p = w.player;
    const float sy = detSin(p.yaw), cy = detCos(p.yaw);
    return {{p.x - 4.0f * sy, V::walkHeight(p.x, p.z) + 3.0f, p.z + 4.0f * cy}, p.yaw, -0.32f};
}

void fly(FreeCamera& cam, float forward, float strafe, bool fast, float seconds) {
    const Vec3 d = lookDirection(cam.yaw, cam.pitch);
    const Vec3 right{detCos(cam.yaw), 0, detSin(cam.yaw)};
    const float step = (fast ? kFlyFast : kFlySpeed) * seconds;
    cam.eye = cam.eye + d * (forward * step) + right * (strafe * step);
    // Stay over the valley, and above its ground.
    cam.eye.x = clampf(cam.eye.x, V::kMin, V::kMax);
    cam.eye.z = clampf(cam.eye.z, V::kMin, V::kMax);
    const float floor = V::walkHeight(cam.eye.x, cam.eye.z) + 0.3f;
    cam.eye.y = clampf(cam.eye.y, floor, 60.0f);
}

std::string bar(float value, int width) {
    const int filled = static_cast<int>(clampf(value, 0.0f, 1.0f) * static_cast<float>(width) + 0.5f);
    std::string s = "[";
    for (int i = 0; i < width; ++i) s += i < filled ? '#' : '.';
    return s + "]";
}

std::vector<HudLine> hudLines(const World& w, bool watching) {
    std::vector<HudLine> lines;
    if (w.sleep == Sleep::Deep) return lines;  // no day, no body, nothing to do
    lines.push_back({std::to_string(21 + w.day) + " March  " + clockText(w), true});
    lines.push_back({"Thirst    " + bar(w.needs.thirst, 10)});
    lines.push_back({"Hunger    " + bar(w.needs.hunger, 10)});
    lines.push_back({"Tiredness " + bar(w.needs.tiredness, 10)});
    if (fearShown(w)) lines.push_back({"Fear      " + bar(w.fear, 10)});
    lines.push_back({std::string("Lamp      ") + (w.lampLit ? "lit" : "out")});
    if (w.carry != Carry::Nothing) lines.push_back({std::string("Carrying  ") + (w.carry == Carry::Water ? "water" : "clay")});
    if (!w.after) lines.push_back({std::string("Standing  ") + standingText(w.standing)});
    lines.push_back({""});
    lines.push_back({"NEXT", true});
    lines.push_back({nextStep(w)});
    lines.push_back({""});
    // The day's chores. In the after-mode they earn nothing, so they are offered, not asked.
    lines.push_back({w.after ? "CHORES  (as you like)" : "CHORES", true});
    for (int i = 0; i < kQuestCount; ++i)
        lines.push_back({std::string(w.questDone[i] ? "[x] " : "[ ] ") + questText(static_cast<QuestId>(i))});
    const std::vector<std::string> made = inventory(w);
    if (!made.empty()) {
        lines.push_back({""});
        lines.push_back({w.clayLens ? "MADE  (through the clay lens)" : "MADE", true});
        for (const std::string& name : made) lines.push_back({w.clayLens ? name + ": clay" : name});
    }
    if (w.after) {
        lines.push_back({""});
        lines.push_back({watching ? "Watching   step back in (V)" : "Living   step back and watch (V)"});
    }
    return lines;
}

std::string prompt(const World& w, const std::string& typing) {
    if (w.sleep != Sleep::Awake) return "";  // in the dream nothing is there to be done
    if (w.naming >= 0)
        return "Name your " + lower(formShape(w.pots[w.naming].form).name) + ": " + typing + "_   (Enter)";
    if (w.throwing.active)
        return "Throwing a " + lower(formShape(w.throwing.form).name) + " " +
               bar(w.throwing.seconds / kThrowSeconds, 10) + "  stay at the wheel";
    const Use use = usable(w);
    if (use != Use::None) return std::string("E  ") + useText(use);
    const int pot = lookedAtPot(w);
    if (pot >= 0) return std::string("B  break the ") + w.pots[pot].name.data();
    if (atWheel(w)) {
        if (w.potsMade >= valley::kBoardSlots) return "The drying board is full";
        return "At the wheel:  1 water jar   2 bowl   3 cup   4 lamp";
    }
    if (isDusk(w) && !w.lampLit) return "F  light your lamp";
    return "";
}

void setTextScale(float scale) {
    gTextSize = std::round(16.0f * scale);
    gLine = std::round(20.0f * scale);
}

std::vector<std::string> wrapped(const std::string& text, size_t columns) {
    std::vector<std::string> out;
    for (size_t at = 0; at < text.size();) {
        size_t end = text.find('\n', at);
        if (end == std::string::npos) end = text.size();
        std::string line;
        for (size_t i = at; i < end;) {
            size_t space = text.find(' ', i);
            if (space == std::string::npos || space > end) space = end;
            const std::string word = text.substr(i, space - i);
            if (!line.empty() && line.size() + 1 + word.size() > columns) {
                out.push_back(line);
                line.clear();
            }
            line += (line.empty() ? "" : " ") + word;
            i = space + 1;
        }
        out.push_back(line);
        at = end + 1;
    }
    return out;
}

void render(strata::Engine& engine, const World& w, bool help, const std::string& typing,
            const FreeCamera* watching, bool noText, const std::string& message, Cover cover,
            const std::string& caption) {
    const float elevation = setSky(engine, w);
    setLights(engine, w, elevation);
    int32_t width = 0, height = 0;
    engine.drawableSize(width, height);
    const float aspect = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
    const FreeCamera view = watching ? *watching : cameraAtEyes(w);
    const StrataCamera camera = lookFrom(view.eye, view.yaw, view.pitch, aspect);
    engine.setCamera(camera);

    StrataFrameDesc fd{};
    fd.clear_flags = STRATA_CLEAR_COLOR | STRATA_CLEAR_DEPTH | STRATA_CLEAR_SKY;
    engine.beginFrame(fd);
    const bool deep = w.sleep == Sleep::Deep;  // no objects at all
    if (!deep) {
        drawMotes(engine, w);
        drawDoorLamps(engine, nightness(elevation));
        if (lampNight(w)) drawGhatLamps(engine, ghatLamps(w), static_cast<float>(w.tick) / kTickRate);
        drawEgrets(engine, w);
    }
    const float dark = darkness(w);
    if (dark > 0) drawDarkness(engine, view.eye, view.yaw, view.pitch, dark, width, height);

    std::vector<StrataText> texts;
    std::vector<std::string> keep;
    keep.reserve(32);
    const float ink[4] = {0.95f, 0.92f, 0.85f, 1.0f};
    const float lamp[4] = {1.0f, 0.62f, 0.22f, 1.0f};
    float y = kMargin;
    std::vector<float> hudRight;  // where each HUD line ends, top to bottom
    for (const HudLine& l : hudLines(w, watching != nullptr)) {
        hudRight.push_back(noText ? 0 : kMargin + gTextSize * static_cast<float>(l.text.size()));
        if (!l.text.empty()) text(texts, keep, kMargin, y, l.text, l.heading ? lamp : ink);
        y += gLine;
    }
    const size_t hudTexts = texts.size();
    // Deep sleep has nothing to show: only a dim point that slowly breathes,
    // so that the dark does not look like the game has stopped. (Drawn with
    // the names of places, so a narrow page shows it too.)
    if (w.sleep == Sleep::Deep) {
        const float breath = 0.5f - 0.5f * std::cos(sleptSeconds(w) * 2 * kPi / 5.0f);
        const float dim[4] = {ink[0], ink[1], ink[2], 0.08f + 0.22f * breath};
        text(texts, keep, static_cast<float>(width) / 2 - gTextSize / 2, static_cast<float>(height) / 2 - gTextSize / 2,
             ".", dim);
    }
    // The names of places nearby, fading out with distance.
    for (const Place& place : deep ? std::vector<Place>{} : places(w)) {
        const float dx = place.at.x - view.eye.x, dz = place.at.z - view.eye.z;
        const float d = std::sqrt(dx * dx + dz * dz);
        float sx = 0, sy = 0;
        // Not when standing at it (the prompt speaks for it there), nor out of range or sight.
        if (d < kLabelNear || d > place.range || !inSight(view.eye, place) || !toScreen(camera, place.at, width, height, sx, sy)) continue;
        const float fade[4] = {ink[0], ink[1], ink[2], clampf((place.range - d) / 4.0f, 0.0f, 0.85f)};
        const std::string name = place.name;
        const float wide = gTextSize * static_cast<float>(name.size());
        float left = clampf(sx - wide / 2, kMargin, static_cast<float>(width) - kMargin - wide);  // all on screen
        // Clear of the HUD's words on the lines the label's row would cross,
        // and far enough after them not to read as their next word: moved
        // right while it still starts left of its place, otherwise not shown.
        const float gap = 3 * gTextSize;  // three characters
        float clear = 0;
        for (size_t i = 0; i < hudRight.size(); ++i) {
            const float top = kMargin + gLine * static_cast<float>(i);
            if (sy + gTextSize > top && sy < top + gTextSize && hudRight[i] > 0) clear = std::max(clear, hudRight[i] + gap);
        }
        bool covered = clear > sx || clear + wide > static_cast<float>(width) - kMargin;
        left = std::max(left, clear);
        if (left < cover.right && sy < cover.bottom) covered = true;  // under the page's own HUD
        if (covered) continue;
        text(texts, keep, left, sy, name, fade);
    }
    const size_t labelsEnd = texts.size();
    // The crosshair, and what can be done here just below it (not while watching).
    if (!watching && w.sleep != Sleep::Deep)
        text(texts, keep, static_cast<float>(width) / 2 - gTextSize / 2, static_cast<float>(height) / 2 - gTextSize / 2,
             "+", ink);
    const std::string here = watching ? std::string() : prompt(w, typing);
    if (!here.empty())
        text(texts, keep, static_cast<float>(width) / 2 - gTextSize * 0.5f * static_cast<float>(here.size()),
             static_cast<float>(height) / 2 + 2 * gLine, here, ink);
    // As many characters as fit across the window.
    const auto columns = static_cast<size_t>(std::max(16.0f, (static_cast<float>(width) - 2 * kMargin) / gTextSize));
    // The message, centred, under the prompt: higher up it would cross the
    // HUD's lines in a window 720 pixels tall.
    float my = static_cast<float>(height) / 2 + 4 * gLine;
    for (const std::string& line : wrapped(message, columns)) {
        text(texts, keep, static_cast<float>(width) / 2 - gTextSize * 0.5f * static_cast<float>(line.size()), my, line, lamp);
        my += gLine;
    }
    // The controls, packed into as few lines as the window's width allows,
    // along the bottom; the captions just above them.
    float bottom = static_cast<float>(height) - kMargin - gTextSize;
    if (help && w.sleep != Sleep::Deep) {
        std::vector<std::string> items;
        if (watching) items = {"W A S D fly", "Shift faster", "Mouse look", "V step back in"};
        else {
            items = {"W A S D walk", "Shift hurry", "Mouse look", "E use", "F lamp"};
            if (lensAvailable(w)) items.push_back("C clay lens");
            if (w.after) items.push_back("V watch");
        }
        for (const char* item : {"Tab codex", "F5 save", "F9 load", "M sound", "F2 text size", "F3 captions"})
            items.push_back(item);
        if (!watching) items.push_back("Esc free the mouse");
        items.push_back("F1 hide help");
        std::vector<std::string> lines(1);
        for (const std::string& item : items) {
            if (!lines.back().empty() && lines.back().size() + 3 + item.size() > columns) lines.emplace_back();
            lines.back() += (lines.back().empty() ? "" : "   ") + item;
        }
        for (size_t i = lines.size(); i-- > 0;) {
            text(texts, keep, kMargin, bottom, lines[i], ink);
            bottom -= gLine;
        }
    }
    const float dim[4] = {ink[0], ink[1], ink[2], 0.8f};
    for (const std::string& line : wrapped(caption, columns)) {
        text(texts, keep, static_cast<float>(width) / 2 - gTextSize * 0.5f * static_cast<float>(line.size()), bottom - gLine,
             line, dim);
        bottom -= gLine;
    }
    for (size_t i = 0; i < texts.size(); ++i) texts[i].utf8 = keep[i].c_str();
    // A narrow page shows the HUD itself, but the names of places stay on the canvas.
    if (!noText) engine.submitText(texts.data(), texts.size());
    else if (labelsEnd > hudTexts) engine.submitText(texts.data() + hudTexts, labelsEnd - hudTexts);
    engine.endFrame();
}

}  // namespace atman::play
