// The whole game state, and the one function that advances it. No engine
// types: the simulation builds and tests without a GPU, and the same inputs
// always give the same world (see tests/test_determinism.cpp).
#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "sim/math.h"

namespace atman {

constexpr int kTickRate = 30;                 // ticks per second of play
constexpr double kGameMinutesPerSecond = 1.0;  // one game minute per second of play
constexpr int kStartMinute = 16 * 60 + 30;     // the game opens at 4:30 in the afternoon

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

struct World {
    uint64_t seed = 0;
    int64_t tick = 0;
    Player player;
    Needs needs;
    int32_t standing = 0;  // the village's regard for you; 0 is a newcomer
    std::array<bool, kQuestCount> questDone{};
};

// One tick's worth of player intent, already resolved from keys and mouse.
struct Input {
    float forward = 0;  // -1..1, + walks the way the player faces
    float strafe = 0;   // -1..1, + steps right
    bool hurry = false;
    float yaw = 0, pitch = 0;  // absolute view angles for this tick
};

World newWorld(uint64_t seed);
void tick(World& w, const Input& in);

// The game clock: minutes since midnight of the first day (may pass 1440).
double gameMinutes(const World& w);
// "4:30 pm", for the HUD.
std::string clockText(const World& w);

const char* questText(QuestId q);
const char* standingText(int32_t standing);

// A hash of everything in the World, for determinism tests and playthroughs.
uint64_t fingerprint(const World& w);

}  // namespace atman
