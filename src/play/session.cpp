#include "play/session.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>

#include "save/save.h"
#include "sim/days.h"

namespace atman::play {

namespace {

// SDL scancodes.
constexpr int kW = 26, kA = 4, kS = 22, kD = 7, kE = 8, kF = 9, kB = 5, kC = 6, kV = 25, kOne = 30, kFour = 33,
              kEnter = 40, kKeypadEnter = 88, kBackspace = 42, kTab = 43, kUp = 82, kDown = 81, kLShift = 225,
              kRShift = 229, kEscape = 41, kF1 = 58;
constexpr int kF2 = 59, kF3 = 60, kF5 = 62, kF6 = 63, kF9 = 66, kM = 16;
constexpr float kTextScales[Session::kTextSizes] = {1.0f, 1.25f, 1.5f};
constexpr const char* kTextSizeNames[Session::kTextSizes] = {"normal", "large", "larger"};
constexpr float kStride = 0.7f;  // metres a step
constexpr float kLookPerPixel = 0.0025f;
constexpr double kAutosaveSeconds = 120;
constexpr double kAfterSeconds = 14;  // the after-mode's first words stay a while
constexpr double kGreetingSeconds = 10;
// A new game's first words: where you are, and where to look for what to do.
constexpr const char* kGreeting =
    "Your house is beside you; its door, the indigo one, faces the well.\n"
    "The NEXT line, top left, says what comes next.";

int pagesOpen(const World& w) {
    int n = 0;
    for (bool c : w.codex) n += c ? 1 : 0;
    return n;
}
constexpr float kHurryStick = 0.95f;

// The slot's settings file: "text <0..2>" and "captions <0|1>", one a line.
std::string settingsPath(const std::string& slot) { return slot + "/settings.txt"; }

}  // namespace

Session::Session(strata::Engine& engine, const SessionOptions& options)
    : engine_(engine), options_(options), w_(newWorld(options.seed)), scene_(engine), pages_(engine) {
    if (options_.sound) sound_.start();
    homeSlot_ = options_.slot;
    if (!options_.slot.empty()) {
        std::ifstream f(settingsPath(options_.slot));
        std::string key;
        int value = 0;
        while (f >> key >> value) {
            if (key == "text") setTextSize(value, false);
            else if (key == "captions") setCaptions(value != 0, false);
            else if (key == "village" && value == 2) {  // the game was left in the second village
                options_.slot = homeSlot_ + "-2";
                village_ = 2;
            }
        }
    }
    watching_ = following_ = options_.autopilot;
    World start = w_;
    if (!options_.slot.empty() && !options_.fresh && hasSave(options_.slot)) {
        World loaded;
        const LoadResult r = loadSave(options_.slot, loaded);
        if (r.ok) start = loaded;
        say(r.ok ? r.message : r.message + " A new game was started.");
    } else if (!options_.autopilot) {
        say(kGreeting, kGreetingSeconds);
    }
    adopt(start);
    if (!options_.browser) capture(true);
}

void Session::adopt(const World& w) {
    w_ = w;
    heard_ = w;  // a loaded or new world is not a change to be heard
    cam_ = cameraAtEyes(w_);
    yaw_ = w_.player.yaw;
    pitch_ = w_.player.pitch;
    pilot_ = Autopilot{};
    once_ = Input{};
    typing_.clear();
    pending_ = 0;
    pagesSeen_ = pagesOpen(w_);  // a loaded game's pages and after-mode are not news
    fourthSeen_ = w_.codex[static_cast<int>(CodexId::Fourth)];
    daySeen_ = w_.day;
    if (pages_.isOpen()) pages_.close();
}

void Session::say(const std::string& text, double seconds) {
    message_ = text;
    messageUntil_ = clock_ + seconds;
}

void Session::noticeChanges() {
    const int pages = pagesOpen(w_);
    if (pages > pagesSeen_ && !watching_) say(pages - pagesSeen_ > 1 ? "New pages in the codex (Tab)" : "A new page in the codex (Tab)");
    pagesSeen_ = pages;
    const bool fourth = w_.codex[static_cast<int>(CodexId::Fourth)];
    if (fourth && !fourthSeen_)
        // Waking from the deep sleep: the teachings are done, though a save
        // from before chapter 3 may have been in the after-mode already.
        say("Waking, dream and deep sleep: you were there for all three.\n"
            "The teachings are done. Nothing more is asked of you.\n"
            "Live the day as it comes, or step back and watch it lived (V).",
            kAfterSeconds);
    fourthSeen_ = fourth;
    // A lamp night is told of in its morning, so the player can be at the ghat for it.
    if (w_.day != daySeen_ && dayKind(w_) == DayKind::LampNight && !watching_)
        say("This evening the village sets lamps on the ghat steps, and floats more on the river.", 10);
    daySeen_ = w_.day;
}

bool Session::save(const char* note) {
    if (options_.slot.empty()) return false;
    std::string why;
    if (!writeSave(options_.slot, w_, &why)) {
        say("Could not save: " + why);
        return false;
    }
    lastSave_ = clock_;
    if (note) say(note);
    if (options_.afterSave) options_.afterSave();
    return true;
}

void Session::load() {
    if (options_.slot.empty()) return;
    World loaded;
    const LoadResult r = loadSave(options_.slot, loaded);
    if (r.ok) adopt(loaded);
    say(r.message);
}

void Session::restart() {
    adopt(newWorld(options_.seed));
    watching_ = following_ = false;
    say(kGreeting, kGreetingSeconds);
}

void Session::setTextSize(int size, bool announce) {
    textSize_ = std::clamp(size, 0, kTextSizes - 1);
    setTextScale(kTextScales[textSize_]);
    if (announce) {
        say(std::string("Text size: ") + kTextSizeNames[textSize_] + " (F2)");
        writeSettings();
    }
}

void Session::setCaptions(bool on, bool announce) {
    captions_ = on;
    if (announce) {
        say(on ? "Captions on (F3)" : "Captions off (F3)");
        writeSettings();
    }
}

void Session::setVillage(int village) {
    if (homeSlot_.empty() || village == village_ || village < 1 || village > 2) return;
    save(nullptr);  // the village being left keeps its day
    village_ = village;
    options_.slot = village == 1 ? homeSlot_ : homeSlot_ + "-2";
    writeSettings();
    watching_ = following_ = false;
    World next;
    if (hasSave(options_.slot) && loadSave(options_.slot, next).ok) {
        adopt(next);
        say(village == 1 ? "Your first village (F6 for the second)" : "Your second village (F6 for the first)");
    } else {
        adopt(newWorld(options_.seed));
        say("A second village, new; your first keeps its day (F6 goes back).\n" + std::string(kGreeting), kGreetingSeconds);
    }
}

void Session::writeSettings() {
    if (homeSlot_.empty()) return;
    std::error_code ec;
    std::filesystem::create_directories(homeSlot_, ec);
    std::ofstream(settingsPath(homeSlot_)) << "text " << textSize_ << "\ncaptions " << (captions_ ? 1 : 0) << "\nvillage "
                                           << village_ << "\n";
    if (options_.afterSave) options_.afterSave();  // the browser keeps the file in IndexedDB
}

void Session::setMuted(bool muted, bool announce) {
    sound_.setMuted(muted);
    if (announce) say(muted ? "Sound off (M turns it on)" : "Sound on");
}

void Session::listen(double dt) {
    for (audio::Cue c : audio::cues(heard_, w_)) sound_.play(c);
    // Footsteps from how far the body went this frame.
    const float dx = w_.player.x - heard_.player.x, dz = w_.player.z - heard_.player.z;
    const float speed = dt > 0 ? std::sqrt(dx * dx + dz * dz) / static_cast<float>(dt) : 0.0f;
    const float steps = speed > 0.3f && speed < 20.0f ? speed / kStride : 0.0f;
    const Vec3 ears = watching_ ? cam_.eye : cameraAtEyes(w_).eye;
    const audio::Ambience a = audio::ambience(w_, ears, steps);
    sound_.setAmbience(a);
    caption_ = captions_ || sound_.muted() ? audio::captions(w_, a) : std::string();
    heard_ = w_;
}

void Session::capture(bool on) {
    if (!options_.browser) engine_.setRelativeMouseMode(on);
    captured_ = on;
}

void Session::setCaptured(bool on) { captured_ = on; }

void Session::look(float dx, float dy) {
    if (watching_) {
        following_ = false;
        cam_.yaw += dx * kLookPerPixel;
        cam_.pitch = clampf(cam_.pitch - dy * kLookPerPixel, -1.45f, 1.45f);
    } else {
        yaw_ += dx * kLookPerPixel;
        pitch_ = clampf(pitch_ - dy * kLookPerPixel, -1.45f, 1.45f);
    }
}

void Session::setStick(float forward, float strafe) {
    stickForward_ = clampf(forward, -1.0f, 1.0f);
    stickStrafe_ = clampf(strafe, -1.0f, 1.0f);
}

void Session::press(int scancode) { onKeyDown(scancode, false); }

void Session::name(const std::string& text) {
    if (w_.naming < 0) return;
    named_ = text;
    once_.name = named_.c_str();
    typing_.clear();
}

void Session::onKeyDown(int scancode, bool repeat) {
    if (pages_.isOpen()) {
        // Reading the codex: the valley waits.
        if ((scancode == kTab || scancode == kEscape) && !repeat) pages_.close();
        else if (scancode == kUp || scancode == kW) pages_.move(w_, -1);
        else if (scancode == kDown || scancode == kS) pages_.move(w_, 1);
        return;
    }
    if (scancode == kEscape) {
        // In a browser the page frees the mouse; there is no quit.
        if (options_.browser) return;
        if (captured_) capture(false);
        else running_ = false;
        return;
    }
    if (scancode == kF1) {
        help_ = !help_;
        return;
    }
    if (scancode == kF2 && !repeat) {
        setTextSize((textSize_ + 1) % kTextSizes);
        return;
    }
    if (scancode == kF3 && !repeat) {
        setCaptions(!captions_);
        return;
    }
    if (scancode == kF6 && !repeat) {
        setVillage(3 - village_);
        return;
    }
    if (scancode == kF5 && !repeat) {
        if (options_.slot.empty()) say("This game does not save (the autopilot, or a test harness, is playing).");
        else save("Saved.");
        return;
    }
    if (scancode == kF9 && !repeat) {
        load();
        return;
    }
    if (w_.naming >= 0) {
        // While naming a pot, keys type; Enter keeps the name.
        if (scancode == kBackspace && !typing_.empty()) typing_.pop_back();
        else if ((scancode == kEnter || scancode == kKeypadEnter) && !repeat) name(typing_);
        return;
    }
    if (repeat) return;
    if (scancode == kM) setMuted(!sound_.muted());  // after naming: an m in a name is a letter
    else if (scancode == kTab) pages_.open(w_);
    else if (scancode == kV && (w_.after || options_.autopilot)) {
        watching_ = !watching_;
        following_ = false;
        if (watching_) cam_ = cameraAtEyes(w_);
        else {
            yaw_ = w_.player.yaw;  // step back in, looking where the body looks
            pitch_ = w_.player.pitch;
        }
    } else if (watching_) return;  // the body's keys belong to the autopilot
    else if (scancode == kE) once_.use = true;
    else if (scancode == kF) once_.toggleLamp = true;
    else if (scancode == kB) once_.breakPot = true;
    else if (scancode == kC) once_.toggleLens = true;
    else if (scancode >= kOne && scancode <= kFour) once_.throwForm = scancode - kOne;
}

void Session::handle(const StrataEvent& e) {
    switch (e.type) {
        case STRATA_EVENT_QUIT: running_ = false; break;
        case STRATA_EVENT_KEY_DOWN:
            if (e.scancode >= 0 && e.scancode < kMaxScancode) keys_[e.scancode] = true;
            onKeyDown(e.scancode, e.repeat != 0);
            break;
        case STRATA_EVENT_KEY_UP:
            if (e.scancode >= 0 && e.scancode < kMaxScancode) keys_[e.scancode] = false;
            break;
        case STRATA_EVENT_TEXT_INPUT:
            if (w_.naming >= 0 && !pages_.isOpen())
                for (const char* c = e.text; *c; ++c)
                    if (*c >= 32 && *c <= 126 && typing_.size() < static_cast<size_t>(kNameLength)) typing_ += *c;
            break;
        case STRATA_EVENT_MOUSE_BUTTON_DOWN:
            if (!captured_ && !options_.browser) capture(true);
            break;
        case STRATA_EVENT_MOUSE_MOTION:
            if (captured_ && !options_.browser) look(e.dx, e.dy);
            break;
        case STRATA_EVENT_RESIZED:
            if (!options_.browser) engine_.resize(e.width, e.height);  // the page sizes the canvas itself
            break;
        default: break;
    }
}

void Session::step() {
    if (watching_) {
        once_ = Input{};
        tick(w_, pilot_.next(w_));
        return;
    }
    Input in = once_;
    once_ = Input{};
    if (w_.naming < 0) {  // no walking while typing a name
        const float f = (keys_[kW] ? 1.0f : 0.0f) - (keys_[kS] ? 1.0f : 0.0f) + stickForward_;
        const float s = (keys_[kD] ? 1.0f : 0.0f) - (keys_[kA] ? 1.0f : 0.0f) + stickStrafe_;
        in.forward = clampf(f, -1.0f, 1.0f);
        in.strafe = clampf(s, -1.0f, 1.0f);
        in.hurry = keys_[kLShift] || keys_[kRShift] ||
                   stickForward_ * stickForward_ + stickStrafe_ * stickStrafe_ > kHurryStick * kHurryStick;
    }
    in.yaw = yaw_;
    in.pitch = pitch_;
    tick(w_, in);
}

void Session::frame(double dt) {
    clock_ += dt;
    if (!message_.empty() && clock_ > messageUntil_) message_.clear();
    if (!options_.slot.empty() && clock_ - lastSave_ > kAutosaveSeconds) save(nullptr);
    const double speed = options_.speed;
    pending_ += dt * speed;
    if (pending_ > 0.25 * speed) pending_ = 0.25 * speed;  // after a stall, do not try to catch up
    if (pages_.isOpen()) {
        pending_ = 0;  // no time passes while the codex is open (the autosave still runs)
        pages_.render(w_);
        listen(0);  // the valley goes on sounding, but nobody walks
        return;
    }
    if (watching_) {
        const float f = clampf((keys_[kW] ? 1.0f : 0.0f) - (keys_[kS] ? 1.0f : 0.0f) + stickForward_, -1.0f, 1.0f);
        const float s = clampf((keys_[kD] ? 1.0f : 0.0f) - (keys_[kA] ? 1.0f : 0.0f) + stickStrafe_, -1.0f, 1.0f);
        if (f != 0 || s != 0) following_ = false;
        fly(cam_, f, s, keys_[kLShift] || keys_[kRShift], static_cast<float>(dt < 0.1 ? dt : 0.1));
    }
    while (pending_ >= 1.0 / kTickRate) {
        pending_ -= 1.0 / kTickRate;
        step();
    }
    noticeChanges();
    if (watching_ && following_) cam_ = cameraBehind(w_);
    listen(dt);
    scene_.update(w_, watching_);
    render(engine_, w_, help_ && !compact_, typing_, watching_ ? &cam_ : nullptr, compact_, message_, cover_, caption_);
}

}  // namespace atman::play
