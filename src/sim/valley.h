// The valley: its ground, its river, and where everything in the village
// stands. One fixed layout, shared by the rules (walking, collision) and the
// render bridge (meshes), so what the player sees is what they walk on.
//
// World frame (Strata's): +X east, +Y up, +Z south, in metres.
#pragma once

#include <array>

#include "sim/math.h"

namespace atman::valley {

// The terrain grid the ground is sampled on.
constexpr float kMin = -80.0f;          // west and north edge
constexpr float kMax = 80.0f;           // east and south edge
constexpr float kCell = 1.0f;           // metres between samples
constexpr int kSamples = 161;           // per side: (kMax - kMin) / kCell + 1
constexpr float kWalkLimit = 74.0f;     // the player stays within |x|, |z| <= this

// The river runs north to south east of the village.
constexpr float kRiverWest = 30.0f;     // west bank's edge of the water
constexpr float kRiverEast = 46.0f;
constexpr float kWaterY = -1.0f;
constexpr float kRiverbedY = -2.6f;

constexpr float kEyeHeight = 1.6f;      // above the ground under the player
constexpr float kPlayerRadius = 0.3f;

// Height of the bare ground (no buildings, no ghat steps) at (x, z).
float groundHeight(float x, float z);

// Height of whatever the player stands on at (x, z): the ground, or a ghat step.
float walkHeight(float x, float z);

// The ghat: stone steps down the west bank to the water, facing east.
struct Ghat {
    float x0 = 23.0f, x1 = 31.0f;  // west edge (top step) to east edge (in the water)
    float z0 = -7.0f, z1 = 7.0f;
    int steps = 8;
    float top = 0.1f;               // height of the top step
    float rise = 0.14f;             // each step down; the lowest is just above the water
    float stepHeight(int i) const { return top - rise * static_cast<float>(i); }
    float stepDepth() const { return (x1 - x0) / static_cast<float>(steps); }
};
constexpr Ghat kGhat{};

struct House {
    const char* name;
    Box2 footprint;
    float wallHeight;
    float roofRise;      // ridge above the wall top
    bool ridgeAlongX;    // ridge runs east-west (else north-south)
    Vec2 door;           // centre of the door on the wall it faces
    int doorFace;        // 0 north, 1 east, 2 south, 3 west
};

constexpr int kHouseCount = 5;
extern const std::array<House, kHouseCount> kHouses;

// Index of each named place in kHouses.
enum HouseId { YourHouse = 0, PotterHouse = 1, NeighbourWest = 2, NeighbourEast = 3, Shrine = 4 };

// Other solid things in the village, by their footprints.
constexpr Vec2 kWell{2.0f, 1.0f};
constexpr float kWellRadius = 1.1f;
constexpr float kWellHeight = 0.9f;

constexpr Vec2 kWheel{8.0f, -13.0f};     // the potter's wheel, in the yard east of their house
constexpr float kWheelRadius = 0.6f;
constexpr Box2 kKiln{10.5f, -18.5f, 13.0f, -16.0f};
constexpr float kKilnHeight = 1.8f;

// The drying board beside the wheel, where pots the player throws are set
// down, one per slot from west to east. Solid, like the wheel.
constexpr Box2 kBoard{9.6f, -11.35f, 15.0f, -10.65f};
constexpr float kBoardHeight = 0.4f;     // the plank's top, above the ground
constexpr int kBoardSlots = 6;
inline Vec2 boardSlot(int i) { return {10.1f + 0.9f * static_cast<float>(i), -11.0f}; }
// The height of the plank's top: level, kBoardHeight above the highest ground under it.
float boardTop();

// The space over the board, where the motes of light hang (chapter 2).
constexpr Box3 kMoteSpace{{9.5f, 0.42f, -11.5f}, {15.1f, 1.2f, -10.5f}};  // y is above the ground
constexpr int kMoteCount = 360;

constexpr Vec2 kBirdTree{20.0f, 11.0f};  // the tree by the ghat where the two birds sit
constexpr float kTrunkRadius = 0.55f;

// Shade trees around the village and along the river path (trunks are solid).
constexpr int kTreeCount = 9;
constexpr std::array<Vec2, kTreeCount> kTrees = {{
    {-24.0f, 14.0f}, {-13.0f, 24.0f}, {4.0f, 22.0f}, {17.0f, 24.0f}, {-26.0f, -14.0f},
    {-11.0f, -20.0f}, {16.0f, -24.0f}, {22.0f, -12.0f}, {-30.0f, 2.0f},
}};
constexpr float kTreeTrunkRadius = 0.4f;

// Where the coils lie at dusk: on the worn path from the village to the ghat
// (the path runs along z = 4 - 0.2 (x - 20)). From the ghat they come in the
// order 2, 1, 0.
constexpr int kCoilCount = 3;
constexpr std::array<Vec2, kCoilCount> kCoils = {{{12.0f, 5.6f}, {17.0f, 4.6f}, {21.5f, 3.7f}}};
constexpr float kCoilRadius = 0.4f;

// Where a new game starts: outside your house, looking east toward the river.
constexpr Vec2 kStart{-8.0f, 15.0f};
constexpr float kStartYaw = kPi / 2;

// True if a player-sized circle at (x, z) overlaps something solid or the water.
bool blocked(float x, float z);

}  // namespace atman::valley
