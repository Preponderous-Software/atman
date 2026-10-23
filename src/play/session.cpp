#include "play/session.h"

#include <cmath>

#include "save/save.h"

namespace atman::play {

namespace {

// SDL scancodes.
constexpr int kW = 26, kA = 4, kS = 22, kD = 7, kE = 8, kF = 9, kB = 5, kC = 6, kV = 25, kOne = 30, kFour = 33,
              kEnter = 40, kKeypadEnter = 88, kBackspace = 42, kTab = 43, kUp = 82, kDown = 81, kLShift = 225,
              kRShift = 229, kEscape = 41, kF1 = 58;
constexpr int kF5 = 62, kF9 = 66, kM = 16;
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

}  // namespace

Session::Session(strata::Engine& engine, const SessionOptions& options)
    : engine_(engine), options_(options), w_(newWorld(options.seed)), scene_(engine), pages_(engine) {
    if (options_.sound) sound_.start();
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
    afterSeen_ = w_.after;
    if (pages_.isOpen()) pages_.close();
}

void Session::say(const std::string& text, double seconds) {
    message_ = text;
    messageUntil_ = clock_ + seconds;
}

void Session::noticeChanges() {
    const int pages = pagesOpen(w_);
    if (pages > pagesSeen_ && !watching_) say("A new page in the codex (Tab)");
    pagesSeen_ = pages;
    if (w_.after && !afterSeen_)
        say("The teachings are done. Nothing more is asked of you.\n"
            "Live the day as it comes, or step back and watch it lived (V).",
            kAfterSeconds);
    afterSeen_ = w_.after;
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
    sound_.setAmbience(audio::ambience(w_, ears, steps));
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
    if (scancode == kF5 && !repeat) {
        if (options_.slot.empty()) say("This game does not save (it was started with --autopilot).");
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
    render(engine_, w_, help_ && !compact_, typing_, watching_ ? &cam_ : nullptr, compact_, message_);
}

}  // namespace atman::play
