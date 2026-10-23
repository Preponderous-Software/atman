// Chapter 2, Clay and Pots (ghatakasha). The player throws pots at the wheel
// and names them. The clay lens shows pots, bowls, bricks and tiles as one
// material: "by one lump of clay all that is made of clay is known; the
// modification is only a name" [P1]. Over the drying board hang motes of
// light, the space the pots will hold. They are there before any pot is
// thrown, a pot only hides the ones inside it, and breaking it moves none of
// them [C2].
#pragma once

#include <array>
#include <string>
#include <vector>

#include "sim/world.h"

namespace atman {

constexpr float kWheelReach = 2.0f;   // from the wheel's centre, to throw
constexpr float kThrowSeconds = 3.0f;
constexpr float kPotReach = 2.5f;     // to break a pot you are looking at
constexpr int kPotsForLens = 3;       // pots thrown before the clay lens comes

// A form's name and the size of the pot it makes, in metres.
struct FormShape {
    const char* name;
    float height;
    float radius;  // the widest
};
const FormShape& formShape(Form f);

// Where pot `slot` stands: the centre of its base, on the board.
Vec3 potBase(int slot);

bool atWheel(const World& w);
bool lensAvailable(const World& w);
// The board slot of the unbroken pot the player is looking at within reach, or -1.
int lookedAtPot(const World& w);
// The names of the pots on the board that are still whole, west to east.
std::vector<std::string> inventory(const World& w);

// The motes of light over the board. Nothing in the World moves them: the
// same positions before the first pot, while pots stand, and after any break.
const std::array<Vec3, valley::kMoteCount>& motes();

// One tick of the chapter; tick() calls it.
void clayTick(World& w, const Input& in);

}  // namespace atman
