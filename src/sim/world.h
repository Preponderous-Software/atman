// The whole game state, and the one function that advances it. No engine
// types: the simulation builds and tests without a GPU, and the same inputs
// always give the same world (see tests/test_determinism.cpp).
#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "sim/math.h"
#include "sim/valley.h"

namespace atman {

constexpr int kTickRate = 30;                 // ticks per second of play
constexpr double kGameMinutesPerSecond = 1.0;  // one game minute per second of play
constexpr int kStartMinute = 16 * 60 + 30;     // the game opens at 4:30 in the afternoon
constexpr int kDuskMinute = 18 * 60 + 15;      // the sun is down and the light is dim (see play/view.h)
constexpr int kDawnMinute = 6 * 60 + 15;       // and light enough again the next morning

struct Player {
    float x = 0, z = 0;
    float yaw = 0, pitch = 0;  // yaw 0 faces north (-z), positive turns right
};

// What the body wants. Each runs from 0 (content) to 1 (desperate). Nothing in
// M1 satisfies them yet; they are on the HUD because the prologue is a
// deliberately ordinary game, and later chapters take the HUD apart.
struct Needs {
    float thirst = 0.15f;
    float hunger = 0.25f;
    float tiredness = 0.3f;
};

enum class QuestId : int32_t { CarryWater = 0, FetchClay = 1, Count = 2 };
constexpr int kQuestCount = static_cast<int>(QuestId::Count);

// One of the coils on the path (chapter 1, The Rope). At dusk each is seen as
// a snake until the player looks at it closely by lamplight; then it is a rope
// and stays one. It was always a rope: only `seenThrough` changes.
struct Coil {
    bool seenThrough = false;
    float gaze = 0;  // seconds of lamplit looking so far
};

// Codex entries the player has met. M2 only records them; the codex screen
// comes in M4.
enum class CodexId : int32_t { RopeAndSnake = 0, Count = 1 };
constexpr int kCodexCount = static_cast<int>(CodexId::Count);

struct World {
    uint64_t seed = 0;
    int64_t tick = 0;
    Player player;
    Needs needs;
    int32_t standing = 0;  // the village's regard for you; 0 is a newcomer
    std::array<bool, kQuestCount> questDone{};
    bool lampLit = false;
    float fear = 0;            // 0..1
    bool fearRetired = false;  // the fear meter has left the HUD for good
    std::array<Coil, valley::kCoilCount> coils{};
    std::array<bool, kCodexCount> codex{};
};

// One tick's worth of player intent, already resolved from keys and mouse.
struct Input {
    float forward = 0;  // -1..1, + walks the way the player faces
    float strafe = 0;   // -1..1, + steps right
    bool hurry = false;
    float yaw = 0, pitch = 0;  // absolute view angles for this tick
    bool toggleLamp = false;   // light the lamp, or put it out
};

World newWorld(uint64_t seed);
void tick(World& w, const Input& in);

// The game clock: minutes since midnight of the first day (may pass 1440).
double gameMinutes(const World& w);
// "4:30 pm", for the HUD.
std::string clockText(const World& w);

// True from dusk until dawn, when the light is dim enough to mistake a rope.
bool isDusk(const World& w);
// True while coil i is seen as a snake.
bool snakeVisible(const World& w, int i);
// True while the fear meter is on the HUD.
bool fearShown(const World& w);
// The coil the player is looking at within lamp reach, or -1. Lit or not.
int lookedAtCoil(const World& w);

const char* questText(QuestId q);
const char* standingText(int32_t standing);

// A hash of everything in the World, for determinism tests and playthroughs.
uint64_t fingerprint(const World& w);

}  // namespace atman
