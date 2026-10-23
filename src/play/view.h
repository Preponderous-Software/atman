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

// `lightweight`: fewer, smaller shadow cascades, for phones and the browser.
void setLighting(strata::Engine& engine, bool lightweight = false);

// Places the sun for the game's time; returns its elevation in degrees.
float setSky(strata::Engine& engine, const World& w);

// The point lights: the carried lamp while it is lit, and after sunset the
// lamps in the village's doorways and a faint fill from the night sky.
void setLights(strata::Engine& engine, const World& w, float sunElevation);

StrataCamera playerCamera(const World& w, float aspect);

// A camera that flies free of the body, for watching the avatar in the
// after-mode. Not part of the World: watching changes nothing in it.
struct FreeCamera {
    Vec3 eye;
    float yaw = 0, pitch = 0;
};

// Starts at the player's eyes, looking the same way.
FreeCamera cameraAtEyes(const World& w);
// Behind and above the avatar, looking the way it faces.
FreeCamera cameraBehind(const World& w);
// Flies by `forward` and `strafe` (-1..1) for `seconds`, along the look direction.
void fly(FreeCamera& cam, float forward, float strafe, bool fast, float seconds);

// One line of the HUD's text, top to bottom.
struct HudLine {
    std::string text;
    bool heading = false;
};

// The HUD's left column: the date and time, the body's needs (and fear, while
// there is a snake), the lamp, what the player carries, standing, what to do
// next, the quest log, and the names of the pots the player has made. In the after-mode it is
// quieter: no standing, and a line saying whether the player is living as
// the avatar or watching it.
std::vector<HudLine> hudLines(const World& w, bool watching = false);

// The line under the crosshair: what the player can do here (throw, name,
// break, a chore, eat, drink, sleep, light the lamp after dark), or empty. `typing` is the name typed so
// far while naming a pot.
std::string prompt(const World& w, const std::string& typing);

// "[#####.....]" for a value in 0..1.
std::string bar(float value, int width);

// Draws one frame (not presented). `help` adds the controls line. With
// `watching`, the view is from that camera and the HUD says so. `noText`
// leaves the words off the canvas (a narrow browser page shows them itself).
// `message` (saved, loaded, refused) shows above the middle of the screen.
// `cover` is the part of the canvas, from its top-left corner, that the page
// covers with its own HUD; the names of places keep out of it.
struct Cover {
    float right = 0, bottom = 0;  // in canvas pixels
};
void render(strata::Engine& engine, const World& w, bool help, const std::string& typing = "",
            const FreeCamera* watching = nullptr, bool noText = false, const std::string& message = "",
            Cover cover = {});

}  // namespace atman::play
