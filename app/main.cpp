// atman: the game on the Strata engine.
//
//   atman [--seed N] [--width W --height H]
//   atman --headless --tour --frames DIR [--width W --height H]
//
// Controls: mouse to look, W A S D to walk, Shift to hurry, F for the lamp,
// 1-4 at the wheel to throw a pot (then type its name and press Enter), B to
// break the pot in view, C for the clay lens, F1 for help, Esc to free the
// mouse (Esc again quits). --tour walks a fixed route
// through the valley into the night, back up the path past the snakes with the
// lamp, and, headless, writes a screenshot at each stop to DIR.
#include <chrono>
#include <cmath>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "bridge/scene.h"
#include "play/view.h"
#include "sim/clay.h"
#include "sim/valley.h"
#include "sim/world.h"
#include "strata/strata.hpp"

using namespace atman;

namespace {

struct Args {
    uint64_t seed = 1;
    bool headless = false;
    bool tour = false;
    std::string framesDir;
    int width = 1280, height = 720;
};

bool parseArgs(int argc, char** argv, Args& a) {
    for (int i = 1; i < argc; ++i) {
        const std::string s = argv[i];
        auto next = [&](const char* what) -> const char* {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "atman: %s needs a value\n", what);
                std::exit(2);
            }
            return argv[++i];
        };
        if (s == "--seed") a.seed = std::strtoull(next("--seed"), nullptr, 10);
        else if (s == "--headless") a.headless = true;
        else if (s == "--tour") a.tour = true;
        else if (s == "--frames") a.framesDir = next("--frames");
        else if (s == "--width") a.width = std::atoi(next("--width"));
        else if (s == "--height") a.height = std::atoi(next("--height"));
        else if (s == "--help" || s == "-h") return false;
        else {
            std::fprintf(stderr, "atman: unknown argument '%s'\n", s.c_str());
            return false;
        }
    }
    if (a.headless && !a.tour) {
        std::fprintf(stderr, "atman: --headless needs --tour (there is no one to play)\n");
        return false;
    }
    return true;
}

void usage() {
    std::puts("usage: atman [--seed N] [--width W --height H]\n"
              "       atman --headless --tour --frames DIR [--width W --height H]");
}

// ---- the tour: a fixed walk for screenshots and CI smoke runs

// One stop, in order: light the lamp; walk to (x, z); turn to the view, or to
// a coil or a pot; start throwing a form; wait (time passes, the sun moves);
// name the pot; toggle the clay lens; break the pot in view; take a shot.
// Built with the setters below, so each stop says only what it does.
struct TourStop {
    bool walk = false;
    float x = 0, z = 0;
    float yaw = 0, pitch = 0;
    int coil = -1, pot = -1;  // aim at this coil or pot instead of yaw/pitch
    int waitSeconds = 0;      // or -1 "until 6:00 pm", -2 "until 7:20 pm"
    bool lamp = false;
    int throwForm = -1;
    const char* potName = nullptr;
    bool lens = false;
    bool breakPot = false;
    const char* shot = nullptr;  // screenshot file name

    constexpr TourStop to(float px, float pz) const { TourStop s = *this; s.walk = true; s.x = px; s.z = pz; return s; }
    constexpr TourStop look(float y, float p) const { TourStop s = *this; s.yaw = y; s.pitch = p; return s; }
    constexpr TourStop atCoil(int i) const { TourStop s = *this; s.coil = i; return s; }
    constexpr TourStop atPot(int i) const { TourStop s = *this; s.pot = i; return s; }
    constexpr TourStop wait(int seconds) const { TourStop s = *this; s.waitSeconds = seconds; return s; }
    constexpr TourStop lightLamp() const { TourStop s = *this; s.lamp = true; return s; }
    constexpr TourStop throwing(Form f) const { TourStop s = *this; s.throwForm = static_cast<int>(f); return s; }
    constexpr TourStop named(const char* n) const { TourStop s = *this; s.potName = n; return s; }
    constexpr TourStop toggleLens() const { TourStop s = *this; s.lens = true; return s; }
    constexpr TourStop breaking() const { TourStop s = *this; s.breakPot = true; return s; }
    constexpr TourStop shoot(const char* name) const { TourStop s = *this; s.shot = name; return s; }
};

constexpr TourStop kStop{};

// From your door past the well to the potter's yard. Throw and name three
// pots at the wheel, look through the clay lens, break the water jar. Down the
// path to the ghat, then sit on the steps while the sun goes down. At night,
// walk back up the path: see the first snake in the dark, then light the lamp
// and look at each one closely until it is a rope.
const TourStop kTour[] = {
    kStop.look(0.9f, -0.03f).shoot("01-your-door"),
    kStop.to(-1.0f, 4.5f).look(0.75f, -0.22f).shoot("02-well"),
    kStop.to(-1.5f, -3.0f),
    kStop.to(6.0f, -6.0f).look(0.2f, -0.12f).shoot("03-potters-yard"),
    kStop.to(9.0f, -12.2f).look(-0.9f, -0.75f).throwing(Form::WaterJar).wait(1).shoot("04-at-the-wheel"),
    kStop.look(-0.9f, -0.75f).wait(2).named("Ganga water"),
    kStop.look(-0.9f, -0.75f).throwing(Form::Bowl).wait(3).named("Rice for Amma"),
    kStop.look(-0.9f, -0.75f).throwing(Form::Lamp).wait(3).named("Evening lamp"),
    kStop.to(11.5f, -12.4f).look(kPi, -0.6f).shoot("05-named-pots"),
    kStop.look(-2.45f, -0.3f).toggleLens().shoot("06-clay-lens"),
    kStop.look(-2.45f, -0.3f).toggleLens(),
    kStop.to(10.1f, -12.3f).atPot(0).breaking().wait(1).shoot("07-pot-space"),
    kStop.to(9.0f, -12.2f),
    kStop.to(8.8f, -10.0f),
    kStop.to(14.0f, -1.0f),
    kStop.to(23.5f, 0.0f).look(kPi / 2, -0.15f).shoot("08-ghat"),
    kStop.look(-2.95f, 0.28f).shoot("09-bird-tree"),
    kStop.look(-kPi / 2, -0.04f).shoot("10-village-from-ghat"),
    kStop.to(29.5f, 0.0f).look(kPi / 2 - 0.35f, 0.03f),
    kStop.look(kPi / 2 - 0.35f, 0.03f).wait(-1).shoot("11-sunset"),
    kStop.look(kPi / 2 - 0.35f, 0.05f).wait(-2).shoot("12-night"),
    kStop.to(26.5f, 2.6f).atCoil(2).wait(3).shoot("13-snake-on-the-path"),
    kStop.lightLamp().to(24.0f, 3.2f).atCoil(2).shoot("14-lamp-on-the-snake"),
    kStop.atCoil(2).wait(2).shoot("15-rope"),
    kStop.to(19.4f, 4.1f).atCoil(1).wait(3),
    kStop.to(14.4f, 5.1f).atCoil(0).wait(3),
    kStop.look(1.75f, -0.3f).wait(1).shoot("16-fear-gone"),
};

// Waits are given as seconds, or as -1 "until 6:00 pm" and -2 "until 7:20 pm".
int waitTicks(const World& w, int waitSeconds) {
    if (waitSeconds >= 0) return waitSeconds * kTickRate;
    const double until = waitSeconds == -1 ? 18 * 60 : 19 * 60 + 20;
    const double minutes = until - gameMinutes(w);
    return minutes > 0 ? static_cast<int>(minutes / kGameMinutesPerSecond * kTickRate) : 0;
}

int runTour(strata::Engine& engine, const Args& args) {
    World w = newWorld(args.seed);
    bridge::Scene scene(engine);
    for (const TourStop& stop : kTour) {
        if (stop.lamp && !w.lampLit) {
            Input in;
            in.yaw = w.player.yaw;
            in.pitch = w.player.pitch;
            in.toggleLamp = true;
            tick(w, in);
        }
        if (stop.walk) {
            bool arrived = false;
            for (int t = 0; t < 120 * kTickRate && !arrived; ++t) {
                const float dx = stop.x - w.player.x, dz = stop.z - w.player.z;
                arrived = dx * dx + dz * dz < 0.04f;
                if (arrived) break;
                Input in;
                in.forward = 1;
                in.yaw = std::atan2(dx, -dz);  // forward is (sin yaw, -cos yaw)
                tick(w, in);
            }
            if (!arrived) {
                std::fprintf(stderr, "atman: the tour got stuck at (%.2f, %.2f) on the way to (%.2f, %.2f)\n", w.player.x,
                             w.player.z, stop.x, stop.z);
                return 1;
            }
        }
        // Turn to the view (or the coil, or the pot) over a second, then wait.
        float yaw = stop.yaw, pitch = stop.pitch;
        if (stop.coil >= 0 || stop.pot >= 0) {
            Vec3 target;
            if (stop.coil >= 0) {
                const Vec2 c = valley::kCoils[stop.coil];
                target = {c.x, valley::groundHeight(c.x, c.z) + 0.1f, c.z};
            } else {
                const Vec3 b = potBase(stop.pot);
                target = {b.x, b.y + formShape(w.pots[stop.pot].form).height / 2, b.z};
            }
            const float dx = target.x - w.player.x, dz = target.z - w.player.z;
            const float dy = target.y - (valley::walkHeight(w.player.x, w.player.z) + valley::kEyeHeight);
            yaw = std::atan2(dx, -dz);
            pitch = std::atan2(dy, std::sqrt(dx * dx + dz * dz));
        }
        const float yaw0 = w.player.yaw, pitch0 = w.player.pitch;
        float dyaw = yaw - yaw0;
        while (dyaw > kPi) dyaw -= 2 * kPi;
        while (dyaw < -kPi) dyaw += 2 * kPi;
        const int turn = kTickRate, wait = waitTicks(w, stop.waitSeconds);
        for (int t = 0; t < turn + wait; ++t) {
            const float k = t < turn ? static_cast<float>(t + 1) / turn : 1.0f;
            Input in;
            in.yaw = yaw0 + dyaw * k;
            in.pitch = pitch0 + (pitch - pitch0) * k;
            if (t == 0) in.throwForm = stop.throwForm;
            if (t == turn + wait - 1) {
                in.name = stop.potName;
                in.toggleLens = stop.lens;
            }
            tick(w, in);
        }
        if (stop.breakPot) {
            Input in;
            in.yaw = w.player.yaw;
            in.pitch = w.player.pitch;
            in.breakPot = true;
            tick(w, in);
        }
        if (!stop.shot) continue;
        scene.update(w);
        play::render(engine, w, false);
        engine.present();
        const float elevation = play::setSky(engine, w);
        std::printf("%-22s player (%6.2f, %6.2f) at %s, sun %5.1f deg, fear %.2f\n", stop.shot, w.player.x,
                    w.player.z, clockText(w).c_str(), elevation, w.fear);
        if (!args.framesDir.empty()) engine.saveScreenshotPng(args.framesDir + "/" + stop.shot + ".png");
    }
    std::printf("%zu meshes, fingerprint %016" PRIx64 "\n", scene.meshCount(), fingerprint(w));
    for (const Coil& c : w.coils)
        if (!c.seenThrough) {
            std::fprintf(stderr, "atman: the tour ended with a snake still on the path\n");
            return 1;
        }
    if (!w.fearRetired) {
        std::fprintf(stderr, "atman: the tour ended with the fear meter still on the HUD\n");
        return 1;
    }
    if (w.potsMade != 3 || !w.pots[0].broken || w.pots[1].broken || w.pots[2].broken || w.naming >= 0) {
        std::fprintf(stderr, "atman: the tour did not throw, name and break its pots\n");
        return 1;
    }
    return 0;
}

// ---- playing in a window

constexpr int kW = 26, kA = 4, kS = 22, kD = 7, kF = 9, kB = 5, kC = 6, kOne = 30, kFour = 33, kEnter = 40,
              kKeypadEnter = 88, kBackspace = 42, kLShift = 225, kRShift = 229, kEscape = 41, kF1 = 58;

int runGame(strata::Engine& engine, const Args& args) {
    World w = newWorld(args.seed);
    bridge::Scene scene(engine);
    bool keys[512] = {};
    bool running = true, captured = false, help = true;
    float yaw = w.player.yaw, pitch = w.player.pitch;
    // One-shot actions from key presses, handed to the next tick.
    Input once;
    std::string typing, named;  // the name being typed, and the one just entered
    auto capture = [&](bool on) {
        engine.setRelativeMouseMode(on);
        captured = on;
    };
    capture(true);

    using Clock = std::chrono::steady_clock;
    auto last = Clock::now();
    double pending = 0;
    while (running) {
        StrataEvent e;
        while (engine.pollEvent(e)) {
            const bool naming = w.naming >= 0;
            switch (e.type) {
                case STRATA_EVENT_QUIT: running = false; break;
                case STRATA_EVENT_KEY_DOWN:
                    if (e.scancode >= 0 && e.scancode < 512) keys[e.scancode] = true;
                    if (e.scancode == kEscape) {
                        if (captured) capture(false);
                        else running = false;
                    } else if (e.scancode == kF1) help = !help;
                    else if (naming) {
                        // While naming a pot, keys type; Enter keeps the name.
                        if (e.scancode == kBackspace && !typing.empty()) typing.pop_back();
                        else if ((e.scancode == kEnter || e.scancode == kKeypadEnter) && !e.repeat) {
                            named = typing;
                            once.name = named.c_str();
                            typing.clear();
                        }
                    } else if (e.repeat) break;
                    else if (e.scancode == kF) once.toggleLamp = true;
                    else if (e.scancode == kB) once.breakPot = true;
                    else if (e.scancode == kC) once.toggleLens = true;
                    else if (e.scancode >= kOne && e.scancode <= kFour) once.throwForm = e.scancode - kOne;
                    break;
                case STRATA_EVENT_KEY_UP:
                    if (e.scancode >= 0 && e.scancode < 512) keys[e.scancode] = false;
                    break;
                case STRATA_EVENT_TEXT_INPUT:
                    if (naming)
                        for (const char* c = e.text; *c; ++c)
                            if (*c >= 32 && *c <= 126 && typing.size() < static_cast<size_t>(kNameLength)) typing += *c;
                    break;
                case STRATA_EVENT_MOUSE_BUTTON_DOWN:
                    if (!captured) capture(true);
                    break;
                case STRATA_EVENT_MOUSE_MOTION:
                    if (captured) {
                        yaw += e.dx * 0.0025f;
                        pitch = clampf(pitch - e.dy * 0.0025f, -1.45f, 1.45f);
                    }
                    break;
                case STRATA_EVENT_RESIZED: engine.resize(e.width, e.height); break;
                default: break;
            }
        }
        const auto now = Clock::now();
        pending += std::chrono::duration<double>(now - last).count();
        last = now;
        if (pending > 0.25) pending = 0.25;  // after a stall, do not try to catch up
        while (pending >= 1.0 / kTickRate) {
            pending -= 1.0 / kTickRate;
            Input in = once;
            once = Input{};
            if (w.naming < 0) {  // no walking while typing a name
                in.forward = (keys[kW] ? 1.0f : 0.0f) - (keys[kS] ? 1.0f : 0.0f);
                in.strafe = (keys[kD] ? 1.0f : 0.0f) - (keys[kA] ? 1.0f : 0.0f);
                in.hurry = keys[kLShift] || keys[kRShift];
            }
            in.yaw = yaw;
            in.pitch = pitch;
            tick(w, in);
        }
        scene.update(w);
        play::render(engine, w, help, typing);
        engine.present();
    }
    return 0;
}

}  // namespace

int main(int argc, char** argv) {
    Args args;
    if (!parseArgs(argc, argv, args)) {
        usage();
        return 2;
    }
    try {
        strata::EngineDesc desc;
        desc.surface = args.headless ? STRATA_SURFACE_HEADLESS : STRATA_SURFACE_WINDOW;
        desc.width = args.width;
        desc.height = args.height;
        desc.flags = args.headless ? 0u : static_cast<uint32_t>(STRATA_FLAG_VSYNC);
        desc.title = "Atman";
        auto engine = strata::Engine::create(desc);
        play::setLighting(*engine);
        if (args.tour) return runTour(*engine, args);
        return runGame(*engine, args);
    } catch (const strata::Error& e) {
        std::fprintf(stderr, "atman: %s\n", e.what());
        return 1;
    }
}
