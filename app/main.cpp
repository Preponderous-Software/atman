// atman: the game on the Strata engine.
//
//   atman [--slot DIR] [--new] [--seed N] [--width W --height H]
//   atman --autopilot [--speed N] [--width W --height H]
//   atman --headless --tour --frames DIR [--width W --height H]
//   atman --headless --autopilot [--frames DIR] [--width W --height H]
//   atman --version
//
// Controls: mouse to look, W A S D to walk, Shift to hurry, E to use what is
// here (fill, give, dig, drink, eat, sleep), F for the lamp, 1-4 at the wheel
// to throw a pot (then type its name and press Enter), B to break the pot in
// view, C for the clay lens, Tab for the codex (Up and Down to choose a
// page), V in the after-mode to step out and watch (and back in), F5 to save,
// F9 to load the last save, F1 for help, Esc to free the mouse (Esc again
// quits).
//
// The game continues from its save in DIR (by default %APPDATA%\Atman\default
// on Windows, ~/.local/share/atman/default elsewhere), saves every two minutes
// and on quit, and keeps the save before last; --new starts over (the old save
// is kept as the previous one). --autopilot never saves.
//
// --tour walks a fixed route through the valley into the night, back up the
// path past the snakes with the lamp, opens the codex, sleeps, and watches the
// avatar the next morning; headless, it writes a screenshot at each stop to
// DIR. --autopilot lets the autopilot play the whole MVP (at N times speed in
// a window); headless, it runs to the end of the playthrough, writes a
// screenshot each time the autopilot turns to something new if DIR is given,
// and fails unless it ends on the pinned fingerprint.
#include <chrono>
#include <cmath>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "bridge/scene.h"
#include "codex/codex.h"
#include "play/codex_screen.h"
#include "play/session.h"
#include "play/view.h"
#include "save/save.h"
#include "sim/autopilot.h"
#include "sim/chores.h"
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
    bool autopilot = false;
    int speed = 1;
    std::string slot;
    bool fresh = false;
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
        else if (s == "--autopilot") a.autopilot = true;
        else if (s == "--slot") a.slot = next("--slot");
        else if (s == "--new") a.fresh = true;
        else if (s == "--speed") a.speed = std::atoi(next("--speed"));
        else if (s == "--frames") a.framesDir = next("--frames");
        else if (s == "--width") a.width = std::atoi(next("--width"));
        else if (s == "--height") a.height = std::atoi(next("--height"));
        else if (s == "--version") {
            std::printf("atman %s\n", ATMAN_VERSION);
            std::exit(0);
        } else if (s == "--help" || s == "-h") return false;
        else {
            std::fprintf(stderr, "atman: unknown argument '%s'\n", s.c_str());
            return false;
        }
    }
    if (a.headless && !a.tour && !a.autopilot) {
        std::fprintf(stderr, "atman: --headless needs --tour or --autopilot (there is no one to play)\n");
        return false;
    }
    if (a.speed < 1 || a.speed > 120) {
        std::fprintf(stderr, "atman: --speed is 1 to 120\n");
        return false;
    }
    return true;
}

void usage() {
    std::puts("usage: atman [--slot DIR] [--new] [--seed N] [--width W --height H]\n"
              "       atman --autopilot [--speed N] [--width W --height H]\n"
              "       atman --headless --tour --frames DIR [--width W --height H]\n"
              "       atman --headless --autopilot [--frames DIR] [--width W --height H]\n"
              "       atman --version");
}

// ---- the tour: a fixed walk for screenshots and CI smoke runs

// One stop, in order: light the lamp; walk to (x, z); turn to the view, or to
// a coil or a pot; start throwing a form; wait (time passes, the sun moves);
// name the pot; toggle the clay lens; break the pot in view; open the codex
// on an entry; use what is here (E); let the autopilot live for a while and
// watch it from behind; take a shot.
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
    const char* codexPage = nullptr;  // a codex entry's id
    bool use = false;
    int pilotSeconds = 0;             // the autopilot lives this long, watched from outside
    const char* shot = nullptr;       // screenshot file name

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
    constexpr TourStop codex(const char* id) const { TourStop s = *this; s.codexPage = id; return s; }
    constexpr TourStop useHere() const { TourStop s = *this; s.use = true; return s; }
    constexpr TourStop watch(int seconds) const { TourStop s = *this; s.pilotSeconds = seconds; return s; }
    constexpr TourStop shoot(const char* name) const { TourStop s = *this; s.shot = name; return s; }
};

constexpr TourStop kStop{};

// From your door past the well to the potter's yard. Throw and name three
// pots at the wheel, look through the clay lens, break the water jar. Down the
// path to the ghat, then sit on the steps while the sun goes down. At night,
// walk back up the path: see the first snake in the dark, then light the lamp
// and look at each one closely until it is a rope. Then read the codex, go
// home and sleep, and in the after-mode's morning step out and watch the
// avatar set off on its chores.
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
    kStop.codex("rope-and-snake").shoot("17-codex-rope"),
    kStop.codex("pot-space").shoot("18-codex-pot-space"),
    kStop.codex("other-schools").shoot("19-other-schools"),
    kStop.to(-8.0f, 7.0f).look(kPi, 0.0f).useHere(),
    kStop.look(-0.35f, 0.02f).wait(4).shoot("20-morning"),
    kStop.watch(26).shoot("21-watching"),
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
    play::CodexScreen pages(engine);
    for (const TourStop& stop : kTour) {
        if (stop.codexPage) {
            const int entry = codex::find(stop.codexPage);
            if (entry < 0 || !codex::isOpen(w, codex::kEntries[entry])) {
                std::fprintf(stderr, "atman: the tour found codex page '%s' closed\n", stop.codexPage);
                return 1;
            }
            pages.show(w, entry);
            pages.render(w);
            engine.present();
            std::printf("%-22s codex page %s\n", stop.shot, stop.codexPage);
            if (!args.framesDir.empty()) engine.saveScreenshotPng(args.framesDir + "/" + stop.shot + ".png");
            pages.close();
            continue;
        }
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
        if (stop.breakPot || stop.use) {
            Input in;
            in.yaw = w.player.yaw;
            in.pitch = w.player.pitch;
            in.breakPot = stop.breakPot;
            in.use = stop.use;
            tick(w, in);
        }
        // Step out of the body and let the autopilot live; watch from behind
        // and above it.
        play::FreeCamera cam;
        if (stop.pilotSeconds > 0) {
            Autopilot pilot;
            for (int t = 0; t < stop.pilotSeconds * kTickRate; ++t) tick(w, pilot.next(w));
            cam = play::cameraBehind(w);
        }
        if (!stop.shot) continue;
        scene.update(w, stop.pilotSeconds > 0);
        play::render(engine, w, false, "", stop.pilotSeconds > 0 ? &cam : nullptr);
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
    if (codex::openEntries(w).size() != static_cast<size_t>(codex::kEntryCount)) {
        std::fprintf(stderr, "atman: the tour ended with codex pages still closed\n");
        return 1;
    }
    if (!w.after || w.day != 1) {
        std::fprintf(stderr, "atman: the tour did not sleep into the after-mode\n");
        return 1;
    }
    if (w.potsMade != 3 || !w.pots[0].broken || w.pots[1].broken || w.pots[2].broken || w.naming >= 0) {
        std::fprintf(stderr, "atman: the tour did not throw, name and break its pots\n");
        return 1;
    }
    return 0;
}

// ---- the autopilot's playthrough, headless

int runPlaythrough(strata::Engine& engine, const Args& args) {
    World w = newWorld(args.seed);
    bridge::Scene scene(engine);
    Autopilot pilot;
    std::string last;
    int shots = 0;
    const int64_t limit = 3LL * 24 * 60 * 60 * kTickRate;
    for (int64_t t = 0; t < limit && !playthroughDone(w); ++t) {
        tick(w, pilot.next(w));
        if (pilot.doing() == last) continue;
        last = pilot.doing();
        std::printf("%-9s day %d  (%6.2f, %6.2f)  %s\n", clockText(w).c_str(), w.day, w.player.x, w.player.z,
                    last.c_str());
        if (args.framesDir.empty()) continue;
        scene.update(w);
        play::render(engine, w, false);
        engine.present();
        char name[96];
        std::snprintf(name, sizeof name, "/%02d-%s.png", ++shots, last.c_str());
        for (char* c = name + 1; *c; ++c)
            if (*c == ' ') *c = '-';
        engine.saveScreenshotPng(args.framesDir + name);
    }
    const uint64_t fp = fingerprint(w);
    std::printf("playthrough ends at %s on day %d, fingerprint %016" PRIx64 " (pinned %016" PRIx64 ")\n",
                clockText(w).c_str(), w.day, fp, kPlaythroughFingerprint);
    if (!playthroughDone(w) || fp != kPlaythroughFingerprint) {
        std::fprintf(stderr, "atman: the playthrough did not end on the pinned fingerprint\n");
        return 1;
    }
    return 0;
}

// ---- playing in a window

int runGame(strata::Engine& engine, const Args& args) {
    play::SessionOptions o;
    o.seed = args.seed;
    o.autopilot = args.autopilot;
    o.speed = args.speed;
    // Watching the autopilot never touches the player's save.
    if (!args.autopilot) o.slot = args.slot.empty() ? defaultSlot() : args.slot;
    o.fresh = args.fresh;
    play::Session session(engine, o);
    using Clock = std::chrono::steady_clock;
    auto last = Clock::now();
    while (session.running()) {
        StrataEvent e;
        while (engine.pollEvent(e)) session.handle(e);
        const auto now = Clock::now();
        session.frame(std::chrono::duration<double>(now - last).count());
        last = now;
        engine.present();
    }
    session.save(nullptr);
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
        if (args.headless) return runPlaythrough(*engine, args);
        return runGame(*engine, args);
    } catch (const strata::Error& e) {
        std::fprintf(stderr, "atman: %s\n", e.what());
        return 1;
    }
}
