#include "sim/valley.h"

namespace atman::valley {

namespace {

float smoothstep(float e0, float e1, float x) {
    const float t = clampf((x - e0) / (e1 - e0), 0.0f, 1.0f);
    return t * t * (3 - 2 * t);
}

bool circleHitsBox(float x, float z, float r, const Box2& b) {
    const float cx = clampf(x, b.x0, b.x1), cz = clampf(z, b.z0, b.z1);
    const float dx = x - cx, dz = z - cz;
    return dx * dx + dz * dz < r * r;
}

bool circleHitsCircle(float x, float z, float r, Vec2 c, float rc) {
    const float dx = x - c.x, dz = z - c.z, rr = r + rc;
    return dx * dx + dz * dz < rr * rr;
}

}  // namespace

// Doors face the well, roughly: the village is a loose ring around it.
const std::array<House, kHouseCount> kHouses = {{
    {"Your house", {-10.5f, 8.0f, -5.5f, 12.0f}, 2.6f, 1.4f, true, {-8.0f, 8.0f}, 0},
    {"The potter's house", {-1.0f, -15.5f, 5.0f, -10.5f}, 2.8f, 1.5f, true, {2.0f, -10.5f}, 2},
    {"West neighbour", {-19.0f, -6.5f, -14.0f, -1.5f}, 2.5f, 1.3f, false, {-14.0f, -4.0f}, 1},
    {"East neighbour", {8.0f, 9.0f, 13.0f, 13.5f}, 2.6f, 1.4f, true, {10.5f, 9.0f}, 0},
    {"The shrine", {-4.0f, -26.0f, -1.0f, -23.0f}, 2.2f, 1.8f, false, {-2.5f, -23.0f}, 2},
}};

float groundHeight(float x, float z) {
    // A broad, nearly flat terrace for the village, with a gentle roll.
    float h = 0.25f * detSin(x * 0.13f) * detCos(z * 0.11f) + 0.15f * detSin((x + z) * 0.05f);
    // The terrace flattens toward the village centre so houses sit level.
    const float d2 = (x * x + z * z) / (28.0f * 28.0f);
    h *= clampf(d2, 0.15f, 1.0f);
    // Hills to the west and across the river, rising north and south.
    h += smoothstep(-30.0f, -75.0f, x) * 14.0f;
    h += smoothstep(50.0f, 78.0f, x) * 9.0f;
    h += smoothstep(55.0f, 80.0f, z < 0 ? -z : z) * 6.0f;
    // The river channel: banks fall to the bed between kRiverWest and kRiverEast.
    const float inWest = smoothstep(kRiverWest - 6.0f, kRiverWest + 1.0f, x);
    const float inEast = 1.0f - smoothstep(kRiverEast - 1.0f, kRiverEast + 5.0f, x);
    const float channel = inWest * inEast;
    return h + (kRiverbedY - h) * channel;
}

float walkHeight(float x, float z) {
    const float g = groundHeight(x, z);
    const Ghat& gh = kGhat;
    if (x >= gh.x0 && x < gh.x1 && z >= gh.z0 && z <= gh.z1) {
        int i = static_cast<int>((x - gh.x0) / gh.stepDepth());
        if (i >= gh.steps) i = gh.steps - 1;
        const float s = gh.stepHeight(i);
        return s > g ? s : g;
    }
    return g;
}

bool blocked(float x, float z) {
    const float r = kPlayerRadius;
    if (x < -kWalkLimit || x > kWalkLimit || z < -kWalkLimit || z > kWalkLimit) return true;
    // The water: anything standing lower than just above the surface.
    if (walkHeight(x, z) < kWaterY + 0.05f) return true;
    for (const House& h : kHouses)
        if (circleHitsBox(x, z, r, h.footprint)) return true;
    if (circleHitsCircle(x, z, r, kWell, kWellRadius)) return true;
    if (circleHitsCircle(x, z, r, kWheel, kWheelRadius)) return true;
    if (circleHitsBox(x, z, r, kKiln)) return true;
    if (circleHitsCircle(x, z, r, kBirdTree, kTrunkRadius)) return true;
    for (Vec2 t : kTrees)
        if (circleHitsCircle(x, z, r, t, kTreeTrunkRadius)) return true;
    return false;
}

}  // namespace atman::valley
