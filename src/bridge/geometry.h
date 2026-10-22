// Triangle meshes for the valley, built from the layout in sim/valley.h. No
// engine here either: Scene (bridge/scene.h) uploads what this builds, and
// tests check it without a GPU.
#pragma once

#include <cstdint>
#include <vector>

#include "sim/math.h"

namespace atman::bridge {

// Linear RGB.
struct Rgb {
    float r = 1, g = 1, b = 1;
};

// An indexed triangle list with a normal and a colour per vertex. Faces do not
// share vertices, so shading is flat. Triangles wind counter-clockwise seen
// from the side their normal points to (Strata culls back faces).
struct MeshData {
    std::vector<float> positions, normals, colors;
    std::vector<uint32_t> indices;

    size_t vertexCount() const { return positions.size() / 3; }
    bool empty() const { return indices.empty(); }

    // A flat quad a-b-c-d, counter-clockwise seen from the front.
    void quad(Vec3 a, Vec3 b, Vec3 c, Vec3 d, Rgb color);
    void triangle(Vec3 a, Vec3 b, Vec3 c, Rgb color);
    // An axis-aligned box with all six faces.
    void box(Vec3 lo, Vec3 hi, Rgb color);
    // An upright cylinder (or a cone, if the radii differ) with a top cap.
    void cylinder(Vec3 base, float radiusBottom, float radiusTop, float height, int segments, Rgb color);
    // A low-poly blob: a sphere of `radius`, squashed vertically by `squash`.
    void blob(Vec3 centre, float radius, float squash, Rgb color);
};

// The terrain grid: heights and colours for Strata's terrain tile, sampled
// from valley::groundHeight.
struct TerrainData {
    int samples = 0;
    float origin[3] = {0, 0, 0};
    float cell = 1;
    std::vector<float> heights;  // samples * samples, row by row along +Z
    std::vector<float> colors;   // samples * samples linear RGB triples
};

TerrainData buildTerrain();

// The village's static buildings and props: houses, the well, the potter's
// yard, the shrine, the bird tree and the ghat steps.
MeshData buildVillage();

// The river's surface.
MeshData buildWater();

}  // namespace atman::bridge
