// A game being played: the world and its save slot, the scene, the codex
// screen, the input, watching the avatar in the after-mode, and the autosave,
// one frame at a time. The desktop game
// (app/main.cpp) drives it from its own loop; the browser game
// (app/web_main.cpp) from the browser's.
#pragma once

#include <array>
#include <functional>
#include <string>

#include "audio/device.h"
#include "bridge/scene.h"
#include "play/codex_screen.h"
#include "play/view.h"
#include "sim/autopilot.h"
#include "sim/world.h"
#include "strata/strata.hpp"

namespace atman::play {

struct SessionOptions {
    uint64_t seed = 1;
    std::string slot;        // the save slot directory; empty = no saves
    bool fresh = false;      // start a new game even if there is a save
    bool autopilot = false;  // the autopilot plays from the start, watched from behind
    int speed = 1;           // game seconds per second of real time
    // In a browser the page owns the pointer lock and the canvas size; Esc
    // never quits.
    bool browser = false;
    bool sound = true;       // open the sound card (it runs silent if there is none)
    // Called after every save written (the browser flushes it to IndexedDB).
    std::function<void()> afterSave;
};

class Session {
public:
    Session(strata::Engine& engine, const SessionOptions& options);

    // An event from the engine.
    void handle(const StrataEvent& e);
    // Advances by `dt` seconds of real time and draws a frame (not presented).
    void frame(double dt);
    bool running() const { return running_; }

    // Writes the save now; `note` is shown on success.
    bool save(const char* note);
    // Loads the last save, if it can.
    void load();
    // Starts over: a new world (the next save replaces the current one, which
    // is then kept as the previous save).
    void restart();

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
    // Sound on or off (M); it fades rather than stops. `announce` says so on screen.
    void setMuted(bool muted, bool announce = true);
    bool muted() const { return sound_.muted(); }
    // No words on the canvas (a narrow page shows them itself).
    void setCompactHud(bool on) { compact_ = on; }
    // The part of the canvas the page covers with its HUD (canvas pixels from the top left).
    void setCover(float right, float bottom) { cover_ = {right, bottom}; }
    // Whether the codex names its keys (a touch screen has the page's buttons instead).
    void setKeyHints(bool on) { pages_.setKeyHints(on); }
    // The HUD's text size, 0 (as at first) to kTextSizes - 1 (F2), and
    // captions for the sounds that matter (F3). Both are kept in the slot's
    // settings file, so they outlast a new game. `announce` says so on screen.
    static constexpr int kTextSizes = 3;
    void setTextSize(int size, bool announce = true);
    void setCaptions(bool on, bool announce = true);
    int textSize() const { return textSize_; }
    bool captions() const { return captions_; }
    // The sounds that matter, in words, while captions are on (or the sound is off).
    const std::string& caption() const { return caption_; }
    // Two villages, each its own save: 1 in the slot the game was started
    // with, 2 in the slot beside it ("<slot>-2"). Switching (F6) saves the
    // village being left, then loads the other, or starts it new.
    int village() const { return village_; }
    void setVillage(int village);

    const World& world() const { return w_; }
    bool captured() const { return captured_; }
    bool help() const { return help_; }
    bool watching() const { return watching_; }
    bool codexOpen() const { return pages_.isOpen(); }
    const std::string& typing() const { return typing_; }
    // The newest message (saved, loaded, refused), while it shows.
    const std::string& message() const { return message_; }

private:
    void capture(bool on);
    void onKeyDown(int scancode, bool repeat);
    void step();
    void say(const std::string& text, double seconds = 5);
    void noticeChanges();  // tells the player of a new codex page or the after-mode beginning
    void adopt(const World& w);  // takes a loaded or new world as the one being played

    void listen(double dt);
    void openCodex();  // the sounds of this frame's change, and of the place

    strata::Engine& engine_;
    SessionOptions options_;
    audio::Sound sound_;
    World w_;
    World heard_;  // the world as it last sounded, for this frame's cues
    bridge::Scene scene_;
    CodexScreen pages_;
    Autopilot pilot_;
    bool watching_ = false;
    bool following_ = false;  // the camera follows the avatar until the player flies it
    FreeCamera cam_;
    static constexpr int kMaxScancode = 512;
    bool keys_[kMaxScancode] = {};
    bool running_ = true, captured_ = false, help_ = true, compact_ = false;
    Cover cover_;
    float yaw_ = 0, pitch_ = 0;
    float stickForward_ = 0, stickStrafe_ = 0;
    Input once_;                // one-shot actions from key presses, for the next tick
    std::string typing_, named_;  // the name being typed, and the one just entered
    double pending_ = 0;
    double clock_ = 0, lastSave_ = 0;  // seconds of play, for the autosave
    int pagesSeen_ = 0;     // codex pages the player has been told of
    bool fourthSeen_ = false;  // whether waking from the deep sleep has been announced
    int32_t daySeen_ = 0;      // the day whose morning has been announced
    std::string message_;
    double messageUntil_ = 0;
    int village_ = 1;
    bool hudShown_ = true;
    std::array<bool, kCodexCount> codexRead_{};  // the pages open when the codex was last opened
    std::string homeSlot_;  // the slot the game was started with: village 1, and the settings
    int textSize_ = 0;
    bool captions_ = false;
    std::string caption_;

    void writeSettings();
};

}  // namespace atman::play
