// Triangle meshes for the valley, built from the layout in sim/valley.h. No
// engine here either: Scene (bridge/scene.h) uploads what this builds, and
// tests check it without a GPU.
#pragma once

#include <cstdint>
#include <vector>

#include "sim/clay.h"
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
    // A box from corner `o` along edges ex, ey, ez (a right-handed set:
    // ex x ey points along ez).
    void orientedBox(Vec3 o, Vec3 ex, Vec3 ey, Vec3 ez, Rgb color);
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

// `lens` paints it as the clay lens shows it (chapter 2): clay in one colour,
// everything else grey. The same goes for the builders below.
TerrainData buildTerrain(bool lens = false);

// The village's static buildings and props: houses, the well, the potter's
// yard and its drying board, the shrine, the bird tree and the ghat steps.
MeshData buildVillage(bool lens = false);

// The river's surface.
MeshData buildWater(bool lens = false);

// Coil i on the path (valley::kCoils), as it is seen: a snake with its head
// raised, or the rope it always was. Both follow the same spiral.
MeshData buildSnake(int i);
MeshData buildRope(int i);

// A pot of `form` standing on `base`, turned from its profile, with a dark
// mouth. buildPot puts it in a board slot; buildShards is what is left there
// once it is broken.
MeshData buildForm(Form form, Vec3 base, bool lens = false);
MeshData buildPot(int slot, Form form, bool lens = false);
MeshData buildShards(int slot, Form form, bool lens = false);

// The largest radius and the height of a form's profile, for tests.
void formExtent(Form form, float& radius, float& height);

}  // namespace atman::bridge
