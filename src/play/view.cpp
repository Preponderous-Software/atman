#include "play/view.h"

#include <cstdio>

#include "sim/valley.h"

namespace atman::play {

namespace V = atman::valley;

namespace {

constexpr float kFovY = 70.0f;
constexpr float kTextSize = 16.0f;
constexpr float kLine = 20.0f;
constexpr float kMargin = 16.0f;

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
    lines.push_back({std::string("Standing  ") + standingText(w.standing)});
    lines.push_back({""});
    lines.push_back({"QUESTS", true});
    for (int i = 0; i < kQuestCount; ++i)
        lines.push_back({std::string(w.questDone[i] ? "[x] " : "[ ] ") + questText(static_cast<QuestId>(i))});
    return lines;
}

void render(strata::Engine& engine, const World& w, bool help) {
    setSky(engine, w);
    int32_t width = 0, height = 0;
    engine.drawableSize(width, height);
    const float aspect = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
    engine.setCamera(playerCamera(w, aspect));

    StrataFrameDesc fd{};
    fd.clear_flags = STRATA_CLEAR_COLOR | STRATA_CLEAR_DEPTH | STRATA_CLEAR_SKY;
    engine.beginFrame(fd);

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
    // The crosshair.
    text(texts, keep, static_cast<float>(width) / 2 - kTextSize / 2, static_cast<float>(height) / 2 - kTextSize / 2, "+", ink);
    if (help)
        text(texts, keep, kMargin, static_cast<float>(height) - kMargin - kTextSize,
             "W A S D walk   Shift hurry   Mouse look   Esc free the mouse   F1 hide help", ink);
    for (size_t i = 0; i < texts.size(); ++i) texts[i].utf8 = keep[i].c_str();
    engine.submitText(texts.data(), texts.size());
    engine.endFrame();
}

}  // namespace atman::play
