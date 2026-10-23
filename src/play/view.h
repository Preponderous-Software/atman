// What the player sees each frame: the sky at the game's hour, the camera at
// the player's eyes, and the HUD.
#pragma once

#include <string>
#include <vector>

#include "bridge/scene.h"
#include "sim/world.h"
#include "strata/strata.hpp"

namespace atman::play {

// The valley's place and date, for the sun. The game is set on the spring
// equinox at about 25.3 N, 83.0 E (the middle Ganges plain); the year only
// fixes the sky's geometry.
constexpr double kLatitude = 25.3;
constexpr double kLongitude = 83.0;
constexpr double kFirstMidnightUtc = 1774031400.0;  // 2026-03-21 00:00 at UTC+5:30

void setLighting(strata::Engine& engine);

// Places the sun for the game's time; returns its elevation in degrees.
float setSky(strata::Engine& engine, const World& w);

// The carried lamp, as a point light, while it is lit.
void setLamp(strata::Engine& engine, const World& w);

StrataCamera playerCamera(const World& w, float aspect);

// One line of the HUD's text, top to bottom.
struct HudLine {
    std::string text;
    bool heading = false;
};

// The HUD's left column: the time, the body's needs (and fear, while there is
// a snake), the lamp, standing, the quest log, and the names of the pots the
// player has made.
std::vector<HudLine> hudLines(const World& w);

// The line under the crosshair: what the player can do here (throw, name,
// break), or empty. `typing` is the name typed so far while naming a pot.
std::string prompt(const World& w, const std::string& typing);

// "[#####.....]" for a value in 0..1.
std::string bar(float value, int width);

// Draws one frame (not presented). `help` adds the controls line.
void render(strata::Engine& engine, const World& w, bool help, const std::string& typing = "");

}  // namespace atman::play
