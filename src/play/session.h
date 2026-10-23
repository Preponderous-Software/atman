// A game being played: the world, the scene, the codex screen, the input, and
// watching the avatar in the after-mode, one frame at a time. The desktop game
// (app/main.cpp) drives it from its own loop; the browser game
// (app/web_main.cpp) from the browser's.
#pragma once

#include <string>

#include "bridge/scene.h"
#include "play/codex_screen.h"
#include "play/view.h"
#include "sim/autopilot.h"
#include "sim/world.h"
#include "strata/strata.hpp"

namespace atman::play {

struct SessionOptions {
    uint64_t seed = 1;
    bool autopilot = false;  // the autopilot plays from the start, watched from behind
    int speed = 1;           // game seconds per second of real time
    // In a browser the page owns the pointer lock and the canvas size; Esc
    // never quits.
    bool browser = false;
};

class Session {
public:
    Session(strata::Engine& engine, const SessionOptions& options);

    // An event from the engine.
    void handle(const StrataEvent& e);
    // Advances by `dt` seconds of real time and draws a frame (not presented).
    void frame(double dt);
    bool running() const { return running_; }

    // The page's input (the browser): look by a pixel delta; walk (or fly,
    // while watching) with a stick, -1..1 each, beyond 0.95 hurries; press a
    // key by its SDL scancode; name the pot waiting for one.
    void look(float dx, float dy);
    void setStick(float forward, float strafe);
    void press(int scancode);
    void name(const std::string& text);
    // The page took or released the pointer lock.
    void setCaptured(bool on);
    void setHelp(bool on) { help_ = on; }
    // No words on the canvas (a narrow page shows them itself).
    void setCompactHud(bool on) { compact_ = on; }

    const World& world() const { return w_; }
    bool captured() const { return captured_; }
    bool help() const { return help_; }
    bool watching() const { return watching_; }
    bool codexOpen() const { return pages_.isOpen(); }
    const std::string& typing() const { return typing_; }

private:
    void capture(bool on);
    void onKeyDown(int scancode, bool repeat);
    void step();

    strata::Engine& engine_;
    SessionOptions options_;
    World w_;
    bridge::Scene scene_;
    CodexScreen pages_;
    Autopilot pilot_;
    bool watching_ = false;
    bool following_ = false;  // the camera follows the avatar until the player flies it
    FreeCamera cam_;
    static constexpr int kMaxScancode = 512;
    bool keys_[kMaxScancode] = {};
    bool running_ = true, captured_ = false, help_ = true, compact_ = false;
    float yaw_ = 0, pitch_ = 0;
    float stickForward_ = 0, stickStrafe_ = 0;
    Input once_;                // one-shot actions from key presses, for the next tick
    std::string typing_, named_;  // the name being typed, and the one just entered
    double pending_ = 0;
};

}  // namespace atman::play
