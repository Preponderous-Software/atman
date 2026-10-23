#include "play/view.h"

#include <cctype>
#include <cmath>
#include <cstdio>

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
    const Vec3 d = lookDirection(p.yaw, p.pitch);
    const float y = V::walkHeight(p.x, p.z) + V::kEyeHeight;
    const float eye[3] = {p.x, y, p.z};
    const float target[3] = {p.x + d.x, y + d.y, p.z + d.z};
    const float up[3] = {0, 1, 0};
    return strata::cameraLookAt(eye, target, up, kFovY, aspect, 0.05f, 400.0f);
}

std::string bar(float value, int width) {
    const int filled = static_cast<int>(clampf(value, 0.0f, 1.0f) * static_cast<float>(width) + 0.5f);
    std::string s = "[";
    for (int i = 0; i < width; ++i) s += i < filled ? '#' : '.';
    return s + "]";
}

std::vector<HudLine> hudLines(const World& w) {
    std::vector<HudLine> lines;
    lines.push_back({"21 March  " + clockText(w), true});
    lines.push_back({"Thirst    " + bar(w.needs.thirst, 10)});
    lines.push_back({"Hunger    " + bar(w.needs.hunger, 10)});
    lines.push_back({"Tiredness " + bar(w.needs.tiredness, 10)});
    if (fearShown(w)) lines.push_back({"Fear      " + bar(w.fear, 10)});
    lines.push_back({std::string("Lamp      ") + (w.lampLit ? "lit" : "out")});
    lines.push_back({std::string("Standing  ") + standingText(w.standing)});
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
    return lines;
}

std::string prompt(const World& w, const std::string& typing) {
    if (w.naming >= 0)
        return "Name your " + lower(formShape(w.pots[w.naming].form).name) + ": " + typing + "_   (Enter)";
    if (w.throwing.active)
        return "Throwing a " + lower(formShape(w.throwing.form).name) + " " +
               bar(w.throwing.seconds / kThrowSeconds, 10) + "  stay at the wheel";
    const int pot = lookedAtPot(w);
    if (pot >= 0) return std::string("B  break the ") + w.pots[pot].name.data();
    if (atWheel(w)) {
        if (w.potsMade >= valley::kBoardSlots) return "The drying board is full";
        return "At the wheel:  1 water jar   2 bowl   3 cup   4 lamp";
    }
    return "";
}

void render(strata::Engine& engine, const World& w, bool help, const std::string& typing) {
    setSky(engine, w);
    setLamp(engine, w);
    int32_t width = 0, height = 0;
    engine.drawableSize(width, height);
    const float aspect = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
    engine.setCamera(playerCamera(w, aspect));

    StrataFrameDesc fd{};
    fd.clear_flags = STRATA_CLEAR_COLOR | STRATA_CLEAR_DEPTH | STRATA_CLEAR_SKY;
    engine.beginFrame(fd);
    drawMotes(engine, w);

    std::vector<StrataText> texts;
    std::vector<std::string> keep;
    keep.reserve(32);
    const float ink[4] = {0.95f, 0.92f, 0.85f, 1.0f};
    const float lamp[4] = {1.0f, 0.62f, 0.22f, 1.0f};
    float y = kMargin;
    for (const HudLine& l : hudLines(w)) {
        if (!l.text.empty()) text(texts, keep, kMargin, y, l.text, l.heading ? lamp : ink);
        y += kLine;
    }
    // The crosshair, and what can be done here just below it.
    text(texts, keep, static_cast<float>(width) / 2 - kTextSize / 2, static_cast<float>(height) / 2 - kTextSize / 2, "+", ink);
    const std::string here = prompt(w, typing);
    if (!here.empty())
        text(texts, keep, static_cast<float>(width) / 2 - kTextSize * 0.5f * static_cast<float>(here.size()),
             static_cast<float>(height) / 2 + 2 * kLine, here, ink);
    if (help)
        text(texts, keep, kMargin, static_cast<float>(height) - kMargin - kTextSize,
             std::string("W A S D walk   Shift hurry   Mouse look   F lamp   ") +
                 (lensAvailable(w) ? "C clay lens   " : "") + "Esc free the mouse   F1 hide help",
             ink);
    for (size_t i = 0; i < texts.size(); ++i) texts[i].utf8 = keep[i].c_str();
    engine.submitText(texts.data(), texts.size());
    engine.endFrame();
}

}  // namespace atman::play
