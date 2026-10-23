// atman: the game on the Strata engine.
//
//   atman [--seed N] [--width W --height H]
//   atman --headless --tour --frames DIR [--width W --height H]
//
// Controls: mouse to look, W A S D to walk, Shift to hurry, F1 for help, Esc
// to free the mouse (Esc again quits). --tour walks a fixed route through the
// valley into the night and, headless, writes a screenshot at each stop to DIR.
#include <chrono>
#include <cmath>
#include <cinttypes>
#include <cstdio>
#include <cstdlib>
#include <string>

#include "bridge/scene.h"
#include "play/view.h"
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

// One stop: walk to (x, z) (skipped when `walk` is false), turn to the view,
// wait `waitSeconds` (time passes; the sun moves), and take a shot.
struct TourStop {
    bool walk;
    float x, z;
    float yaw, pitch;
    int waitSeconds;
    const char* name;  // screenshot file name, or nullptr for no shot
};

// From your door past the well and the potter's yard, down the path to the
// ghat, then sit on the steps while the sun goes down.
const TourStop kTour[] = {
    {false, 0, 0, 0.9f, -0.03f, 0, "01-your-door"},
    {true, -1.0f, 4.5f, 0.75f, -0.22f, 0, "02-well"},
    {true, -1.5f, -3.0f, 0, 0, 0, nullptr},
    {true, 6.0f, -6.0f, 0.2f, -0.12f, 0, "03-potters-yard"},
    {true, 14.0f, -1.0f, 0, 0, 0, nullptr},
    {true, 23.5f, 0.0f, kPi / 2, -0.15f, 0, "04-ghat"},
    {false, 0, 0, -2.95f, 0.28f, 0, "05-bird-tree"},
    {false, 0, 0, -kPi / 2, -0.04f, 0, "06-village-from-ghat"},
    {true, 29.5f, 0.0f, kPi / 2 - 0.35f, 0.03f, 0, nullptr},
    {false, 0, 0, kPi / 2 - 0.35f, 0.03f, -1, "07-sunset"},
    {false, 0, 0, kPi / 2 - 0.35f, 0.05f, -2, "08-night"},
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
        // Turn to the view over a second, then wait.
        const float yaw0 = w.player.yaw, pitch0 = w.player.pitch;
        float dyaw = stop.yaw - yaw0;
        while (dyaw > kPi) dyaw -= 2 * kPi;
        while (dyaw < -kPi) dyaw += 2 * kPi;
        const int turn = kTickRate, wait = waitTicks(w, stop.waitSeconds);
        for (int t = 0; t < turn + wait; ++t) {
            const float k = t < turn ? static_cast<float>(t + 1) / turn : 1.0f;
            Input in;
            in.yaw = yaw0 + dyaw * k;
            in.pitch = pitch0 + (stop.pitch - pitch0) * k;
            tick(w, in);
        }
        if (!stop.name) continue;
        play::render(engine, w, false);
        engine.present();
        const float elevation = play::setSky(engine, w);
        std::printf("%-22s player (%6.2f, %6.2f) at %s, sun %5.1f deg\n", stop.name, w.player.x, w.player.z,
                    clockText(w).c_str(), elevation);
        if (!args.framesDir.empty()) engine.saveScreenshotPng(args.framesDir + "/" + stop.name + ".png");
    }
    std::printf("%zu meshes, fingerprint %016" PRIx64 "\n", scene.meshCount(), fingerprint(w));
    return 0;
}

// ---- playing in a window

constexpr int kW = 26, kA = 4, kS = 22, kD = 7, kLShift = 225, kRShift = 229, kEscape = 41, kF1 = 58;

int runGame(strata::Engine& engine, const Args& args) {
    World w = newWorld(args.seed);
    bridge::Scene scene(engine);
    bool keys[512] = {};
    bool running = true, captured = false, help = true;
    float yaw = w.player.yaw, pitch = w.player.pitch;
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
            switch (e.type) {
                case STRATA_EVENT_QUIT: running = false; break;
                case STRATA_EVENT_KEY_DOWN:
                    if (e.scancode >= 0 && e.scancode < 512) keys[e.scancode] = true;
                    if (e.scancode == kEscape) {
                        if (captured) capture(false);
                        else running = false;
                    } else if (e.scancode == kF1) help = !help;
                    break;
                case STRATA_EVENT_KEY_UP:
                    if (e.scancode >= 0 && e.scancode < 512) keys[e.scancode] = false;
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
            Input in;
            in.forward = (keys[kW] ? 1.0f : 0.0f) - (keys[kS] ? 1.0f : 0.0f);
            in.strafe = (keys[kD] ? 1.0f : 0.0f) - (keys[kA] ? 1.0f : 0.0f);
            in.hurry = keys[kLShift] || keys[kRShift];
            in.yaw = yaw;
            in.pitch = pitch;
            tick(w, in);
        }
        play::render(engine, w, help);
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
