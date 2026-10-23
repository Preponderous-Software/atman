#include "play/view.h"

#include <cctype>
#include <cmath>
#include <cstdio>

#include "sim/chores.h"
#include "sim/clay.h"
#include "sim/valley.h"

namespace atman::play {

namespace V = atman::valley;

namespace {

constexpr float kFovY = 70.0f;
constexpr float kTextSize = 16.0f;
constexpr float kLine = 20.0f;
constexpr float kMargin = 16.0f;
constexpr float kLampRange = 10.0f;
constexpr float kLampIntensity = 1.4f;
constexpr float kFlySpeed = 4.0f, kFlyFast = 12.0f;  // metres per second
constexpr float kDarkSeconds = 0.6f, kFadeSeconds = 2.4f;  // waking: dark, then the morning fades in

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
    t.pixel_size = kTextSize;
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

}  // namespace

void setLighting(strata::Engine& engine) {
    StrataLighting light = strata::lightingDefaults(STRATA_LIGHTING_SUN);
    light.cascade_count = 4;
    light.shadow_map_size = 2048;
    light.shadow_distance = 80.0f;
    light.fog = 1;
    engine.setLighting(light);
}

float setSky(strata::Engine& engine, const World& w) {
    const StrataSolarPosition sun =
        strata::solarPosition(kFirstMidnightUtc + gameMinutes(w) * 60.0, kLatitude, kLongitude);
    engine.setSunDirection(sun.to_sun);
    return sun.elevation_degrees;
}

void setLamp(strata::Engine& engine, const World& w) {
    if (!w.lampLit) {
        engine.setPointLights(nullptr, 0);
        return;
    }
    // Held low and a little ahead, on the right.
    const Player& p = w.player;
    const float sy = detSin(p.yaw), cy = detCos(p.yaw);
    const float y = V::walkHeight(p.x, p.z) + V::kEyeHeight - 0.55f;
    StrataPointLight lamp{};
    lamp.position[0] = p.x + 0.35f * sy + 0.25f * cy;
    lamp.position[1] = y;
    lamp.position[2] = p.z - 0.35f * cy + 0.25f * sy;
    lamp.range = kLampRange;
    lamp.color[0] = 1.0f;
    lamp.color[1] = 0.62f;
    lamp.color[2] = 0.26f;
    lamp.intensity = kLampIntensity;
    engine.setPointLights(&lamp, 1);
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
    lines.push_back({std::to_string(21 + w.day) + " March  " + clockText(w), true});
    lines.push_back({"Thirst    " + bar(w.needs.thirst, 10)});
    lines.push_back({"Hunger    " + bar(w.needs.hunger, 10)});
    lines.push_back({"Tiredness " + bar(w.needs.tiredness, 10)});
    if (fearShown(w)) lines.push_back({"Fear      " + bar(w.fear, 10)});
    lines.push_back({std::string("Lamp      ") + (w.lampLit ? "lit" : "out")});
    if (w.carry != Carry::Nothing) lines.push_back({std::string("Carrying  ") + (w.carry == Carry::Water ? "water" : "clay")});
    if (!w.after) lines.push_back({std::string("Standing  ") + standingText(w.standing)});
    lines.push_back({""});
    lines.push_back({"QUESTS", true});
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
        lines.push_back({watching ? "Watching   V steps back in" : "Living   V steps back to watch"});
    }
    return lines;
}

std::string prompt(const World& w, const std::string& typing) {
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
    return "";
}

void render(strata::Engine& engine, const World& w, bool help, const std::string& typing,
            const FreeCamera* watching) {
    setSky(engine, w);
    setLamp(engine, w);
    int32_t width = 0, height = 0;
    engine.drawableSize(width, height);
    const float aspect = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
    const FreeCamera view = watching ? *watching : cameraAtEyes(w);
    engine.setCamera(lookFrom(view.eye, view.yaw, view.pitch, aspect));

    StrataFrameDesc fd{};
    fd.clear_flags = STRATA_CLEAR_COLOR | STRATA_CLEAR_DEPTH | STRATA_CLEAR_SKY;
    engine.beginFrame(fd);
    drawMotes(engine, w);
    const float dark = wakeDarkness(w);
    if (dark > 0) drawDarkness(engine, view.eye, view.yaw, view.pitch, dark, width, height);

    std::vector<StrataText> texts;
    std::vector<std::string> keep;
    keep.reserve(32);
    const float ink[4] = {0.95f, 0.92f, 0.85f, 1.0f};
    const float lamp[4] = {1.0f, 0.62f, 0.22f, 1.0f};
    float y = kMargin;
    for (const HudLine& l : hudLines(w, watching != nullptr)) {
        if (!l.text.empty()) text(texts, keep, kMargin, y, l.text, l.heading ? lamp : ink);
        y += kLine;
    }
    // The crosshair, and what can be done here just below it (not while watching).
    if (!watching)
        text(texts, keep, static_cast<float>(width) / 2 - kTextSize / 2, static_cast<float>(height) / 2 - kTextSize / 2,
             "+", ink);
    const std::string here = watching ? std::string() : prompt(w, typing);
    if (!here.empty())
        text(texts, keep, static_cast<float>(width) / 2 - kTextSize * 0.5f * static_cast<float>(here.size()),
             static_cast<float>(height) / 2 + 2 * kLine, here, ink);
    if (help)
        text(texts, keep, kMargin, static_cast<float>(height) - kMargin - kTextSize,
             watching ? std::string("W A S D fly   Shift faster   Mouse look   V step back in   Tab codex   F1 hide help")
                      : std::string("W A S D walk   Shift hurry   Mouse look   E use   F lamp   ") +
                            (lensAvailable(w) ? "C clay lens   " : "") + (w.after ? "V watch   " : "") +
                            "Tab codex   Esc free the mouse   F1 hide help",
             ink);
    for (size_t i = 0; i < texts.size(); ++i) texts[i].utf8 = keep[i].c_str();
    engine.submitText(texts.data(), texts.size());
    engine.endFrame();
}

}  // namespace atman::play
