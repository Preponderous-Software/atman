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

// What the body wants. Each runs from 0 (content) to 1 (desperate). Water from
// the well, a meal at home and a night's sleep answer them. They are on the
// HUD because the prologue is a deliberately ordinary game, and later
// chapters take the HUD apart.
struct Needs {
    float thirst = 0.15f;
    float hunger = 0.25f;
    float tiredness = 0.3f;
};

// The day's chores. Each morning they are on the list again.
enum class QuestId : int32_t { CarryWater = 0, FetchClay = 1, Count = 2 };
constexpr int kQuestCount = static_cast<int>(QuestId::Count);

// What the player has in their hands for a chore.
enum class Carry : int32_t { Nothing = 0, Water = 1, Clay = 2 };

// One of the coils on the path (chapter 1, The Rope). At dusk each is seen as
// a snake until the player looks at it closely by lamplight; then it is a rope
// and stays one. It was always a rope: only `seenThrough` changes.
struct Coil {
    bool seenThrough = false;
    float gaze = 0;  // seconds of lamplit looking so far
};

// What the player can throw at the wheel (chapter 2, Clay and Pots). Each is
// clay in a different shape, and the player gives it a name.
enum class Form : int32_t { WaterJar = 0, Bowl = 1, Cup = 2, Lamp = 3, Count = 4 };
constexpr int kFormCount = static_cast<int>(Form::Count);
constexpr int kNameLength = 20;  // printable ASCII, so the HUD can draw it

// A pot on the drying board, by slot. Breaking one leaves its shards and its
// slot; the space it held was never its own (see clay.h).
struct Pot {
    bool made = false;
    bool broken = false;
    Form form = Form::WaterJar;
    std::array<char, kNameLength + 1> name{};
};

// The pot on the wheel, while the player is throwing it.
struct Throwing {
    bool active = false;
    Form form = Form::WaterJar;
    float seconds = 0;
};

// Codex entries the player has met; each one opens a page of the codex
// (src/codex/).
enum class CodexId : int32_t { RopeAndSnake = 0, ClayAndPots = 1, PotSpace = 2, Dream = 3, Fourth = 4, Count = 5 };

// Chapter 3, Three States. The first night after the rope and the clay, sleep
// is not a cut to morning: the player dreams the valley, a little wrong, then
// sleeps deeply, where there is nothing at all, then wakes (Māṇḍūkya 3-7
// [P3]). Every other night is the cut to morning.
enum class Sleep : int32_t { Awake = 0, Dream = 1, Deep = 2 };
constexpr int kDreamSeconds = 75;      // of play, walking the dream valley
constexpr int kDeepSleepSeconds = 12;  // of play, with nothing on the screen
// In the dream the sun runs backwards from just before dusk, this many game
// minutes a second, and the clock with it. Nothing in the dream finds that
// strange.
constexpr int kDreamStartMinute = kDuskMinute - 5;  // 6:10 pm
constexpr double kDreamMinutesPerSecond = 6.0;
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
    std::array<Pot, valley::kBoardSlots> pots{};
    int32_t potsMade = 0;  // including any broken since
    Throwing throwing;
    int32_t naming = -1;   // the slot whose pot is waiting for a name, or -1
    bool clayLens = false;
    Carry carry = Carry::Nothing;
    int32_t day = 0;          // 0 is the first afternoon; each sleep adds one
    int64_t wokeTick = -1;    // the tick the player last woke, or -1
    // After: the morning after both teachings. Nothing ends; the chores come
    // round again, nothing is scored, and the avatar can walk them alone.
    bool after = false;
    Sleep sleep = Sleep::Awake;
    int64_t sleptTick = -1;  // the tick the dream or the deep sleep began, or -1
};

// One tick's worth of player intent, already resolved from keys and mouse.
struct Input {
    float forward = 0;  // -1..1, + walks the way the player faces
    float strafe = 0;   // -1..1, + steps right
    bool hurry = false;
    float yaw = 0, pitch = 0;  // absolute view angles for this tick
    bool toggleLamp = false;   // light the lamp, or put it out
    int32_t throwForm = -1;    // at the wheel: start throwing this Form
    const char* name = nullptr;  // the name typed for the pot being named
    bool breakPot = false;     // break the pot the player is looking at
    bool toggleLens = false;   // the clay lens, once the player has it
    bool use = false;          // fill, give, dig, drink, eat or sleep, whichever is here
};

World newWorld(uint64_t seed);
void tick(World& w, const Input& in);

// The game clock: minutes since midnight of the first day (may pass 1440).
double gameMinutes(const World& w);
// The minute the sky shows and the HUD's clock reads: the game clock, except
// in a dream, where it runs backwards from dusk.
double skyMinutes(const World& w);
// "4:30 pm", for the HUD (the sky's minute).
std::string clockText(const World& w);
// Seconds of play since the dream or the deep sleep began; 0 when awake.
float sleptSeconds(const World& w);

// True from dusk until dawn, when the light is dim enough to mistake a rope
// (by the sky's minute, so never in the dream).
bool isDusk(const World& w);
// How far the lamp reaches for looking at a snake closely, in metres.
constexpr float kLampReach = 3.5f;

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
