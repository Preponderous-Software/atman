#include "bridge/geometry.h"

#include <cmath>

#include "sim/valley.h"

namespace atman::bridge {

namespace V = atman::valley;

namespace {

Vec3 cross(Vec3 a, Vec3 b) { return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x}; }

Vec3 normalize(Vec3 v) {
    const float l = length(v);
    return l > 0 ? v * (1.0f / l) : Vec3{0, 1, 0};
}

// Village palette (linear RGB): lime-washed walls, fired-tile roofs, brick
// plinths, laterite stone, river clay.
constexpr Rgb kLimewash{0.62f, 0.55f, 0.42f};
constexpr Rgb kLimewashShaded{0.50f, 0.44f, 0.33f};
constexpr Rgb kTile{0.36f, 0.12f, 0.06f};
constexpr Rgb kDoor{0.16f, 0.09f, 0.05f};
constexpr Rgb kDoorIndigo{0.06f, 0.10f, 0.30f};  // your house
constexpr Rgb kDoorOchre{0.55f, 0.25f, 0.06f};   // the potter's
constexpr Rgb kSaffron{0.85f, 0.38f, 0.04f};     // the shrine's door and pennant
constexpr Rgb kPlinth{0.33f, 0.17f, 0.09f};  // fired brick
constexpr Rgb kStone{0.38f, 0.33f, 0.27f};
constexpr Rgb kStoneWet{0.22f, 0.20f, 0.17f};
constexpr Rgb kClay{0.42f, 0.20f, 0.09f};
constexpr Rgb kFiredClay{0.52f, 0.24f, 0.10f};
constexpr Rgb kWood{0.20f, 0.12f, 0.06f};
constexpr Rgb kBark{0.14f, 0.10f, 0.07f};
constexpr Rgb kLeaves{0.06f, 0.16f, 0.04f};
constexpr Rgb kLeavesLight{0.09f, 0.21f, 0.05f};
constexpr Rgb kShrineWash{0.70f, 0.66f, 0.58f};
constexpr Rgb kWater{0.11f, 0.19f, 0.21f};
constexpr Rgb kWellWater{0.04f, 0.07f, 0.08f};
constexpr Rgb kBrick{0.40f, 0.17f, 0.08f};
constexpr Rgb kClayDark{0.12f, 0.05f, 0.02f};
constexpr Rgb kLensClay{0.58f, 0.27f, 0.11f};
constexpr Rgb kSnake{0.05f, 0.05f, 0.025f};
constexpr Rgb kSnakeBand{0.13f, 0.11f, 0.05f};
constexpr Rgb kRope{0.44f, 0.35f, 0.17f};
constexpr Rgb kRopeTwist{0.36f, 0.28f, 0.13f};
constexpr Rgb kWetClay{0.30f, 0.15f, 0.07f};
constexpr Rgb kCotton{0.78f, 0.74f, 0.64f};
constexpr Rgb kCottonShade{0.55f, 0.42f, 0.24f};
constexpr Rgb kSkin{0.36f, 0.22f, 0.14f};
constexpr Rgb kHair{0.04f, 0.03f, 0.03f};

// How the valley is painted: as it looks, or through the clay lens, where
// everything made of clay (pots, bricks, tiles, the riverbank) is one colour
// and everything else is grey.
struct Look {
    bool lens = false;
    Rgb clay(Rgb c) const { return lens ? kLensClay : c; }
    Rgb other(Rgb c) const {
        if (!lens) return c;
        const float l = 0.8f * (0.3f * c.r + 0.59f * c.g + 0.11f * c.b);
        return {l, l, l};
    }
};

void house(MeshData& m, const V::House& h, Rgb wash, Rgb door, const Look& L) {
    const Box2& f = h.footprint;
    const float x0 = f.x0, x1 = f.x1, z0 = f.z0, z1 = f.z1;
    // Sit the house on the lowest ground under it, on a plinth.
    float base = V::groundHeight(x0, z0);
    for (float x : {x0, x1})
        for (float z : {z0, z1}) base = std::fmin(base, V::groundHeight(x, z));
    const float plinth = base + 0.35f;
    m.box({x0 - 0.2f, base - 0.5f, z0 - 0.2f}, {x1 + 0.2f, plinth, z1 + 0.2f}, L.clay(kPlinth));
    const float top = plinth + h.wallHeight;
    m.box({x0, plinth, z0}, {x1, top, z1}, L.other(wash));

    // The door: a panel just proud of the wall it faces, dark wood unless the
    // house is one the player needs to find.
    const float dw = 0.55f, dh = 1.9f, e = 0.02f;
    const Vec2 d = h.door;
    switch (h.doorFace) {
        case 0: m.quad({d.x + dw, plinth, z0 - e}, {d.x - dw, plinth, z0 - e}, {d.x - dw, plinth + dh, z0 - e}, {d.x + dw, plinth + dh, z0 - e}, L.other(door)); break;
        case 1: m.quad({x1 + e, plinth, d.z + dw}, {x1 + e, plinth, d.z - dw}, {x1 + e, plinth + dh, d.z - dw}, {x1 + e, plinth + dh, d.z + dw}, L.other(door)); break;
        case 2: m.quad({d.x - dw, plinth, z1 + e}, {d.x + dw, plinth, z1 + e}, {d.x + dw, plinth + dh, z1 + e}, {d.x - dw, plinth + dh, z1 + e}, L.other(door)); break;
        default: m.quad({x0 - e, plinth, d.z - dw}, {x0 - e, plinth, d.z + dw}, {x0 - e, plinth + dh, d.z + dw}, {x0 - e, plinth + dh, d.z - dw}, L.other(door)); break;
    }

    // A gable roof with eaves, its ridge along x or z.
    const float o = 0.45f, ridge = top + h.roofRise;
    const float ex0 = x0 - o, ex1 = x1 + o, ez0 = z0 - o, ez1 = z1 + o, eave = top - 0.2f;
    if (h.ridgeAlongX) {
        const float zm = (z0 + z1) / 2;
        m.quad({ex1, eave, ez0}, {ex0, eave, ez0}, {ex0, ridge, zm}, {ex1, ridge, zm}, L.clay(kTile));    // north slope
        m.quad({ex0, eave, ez1}, {ex1, eave, ez1}, {ex1, ridge, zm}, {ex0, ridge, zm}, L.clay(kTile));    // south slope
        m.triangle({x0, top, z0}, {x0, top, z1}, {x0, ridge - 0.05f, zm}, L.other(kLimewashShaded));    // west gable
        m.triangle({x1, top, z1}, {x1, top, z0}, {x1, ridge - 0.05f, zm}, L.other(kLimewashShaded));    // east gable
    } else {
        const float xm = (x0 + x1) / 2;
        m.quad({ex0, eave, ez0}, {ex0, eave, ez1}, {xm, ridge, ez1}, {xm, ridge, ez0}, L.clay(kTile));    // west slope
        m.quad({ex1, eave, ez1}, {ex1, eave, ez0}, {xm, ridge, ez0}, {xm, ridge, ez1}, L.clay(kTile));    // east slope
        m.triangle({x1, top, z0}, {x0, top, z0}, {xm, ridge - 0.05f, z0}, L.other(kLimewashShaded));    // north gable
        m.triangle({x0, top, z1}, {x1, top, z1}, {xm, ridge - 0.05f, z1}, L.other(kLimewashShaded));    // south gable
    }
}

void well(MeshData& m, const Look& L) {
    const float g = V::groundHeight(V::kWell.x, V::kWell.z);
    const Vec3 base{V::kWell.x, g - 0.2f, V::kWell.z};
    m.cylinder(base, V::kWellRadius, V::kWellRadius, V::kWellHeight + 0.2f, 14, L.other(kStone));
    // The water inside, dark, just below the rim.
    m.cylinder({base.x, base.y, base.z}, V::kWellRadius - 0.2f, V::kWellRadius - 0.2f, V::kWellHeight, 14, L.other(kWellWater));
    // Two posts and a beam for the rope.
    const float r = V::kWellRadius - 0.1f, top = g + V::kWellHeight + 1.3f;
    m.box({base.x - r - 0.08f, g, base.z - 0.08f}, {base.x - r + 0.08f, top, base.z + 0.08f}, L.other(kWood));
    m.box({base.x + r - 0.08f, g, base.z - 0.08f}, {base.x + r + 0.08f, top, base.z + 0.08f}, L.other(kWood));
    m.box({base.x - r - 0.1f, top - 0.12f, base.z - 0.07f}, {base.x + r + 0.1f, top, base.z + 0.07f}, L.other(kWood));
}

void potterYard(MeshData& m, const Look& L) {
    // The wheel: a low stone base and a wide clay-stained wooden disc. The pot
    // on it is drawn by the scene while the player throws one.
    const float g = V::groundHeight(V::kWheel.x, V::kWheel.z);
    m.cylinder({V::kWheel.x, g, V::kWheel.z}, 0.25f, 0.2f, 0.25f, 10, L.other(kStone));
    m.cylinder({V::kWheel.x, g + 0.25f, V::kWheel.z}, V::kWheelRadius, V::kWheelRadius, 0.08f, 16, L.other(kWood));

    // The kiln: a squat clay dome over a brick base.
    const Box2& k = V::kKiln;
    const float kg = V::groundHeight((k.x0 + k.x1) / 2, (k.z0 + k.z1) / 2);
    m.box({k.x0, kg - 0.3f, k.z0}, {k.x1, kg + 0.6f, k.z1}, L.clay(kFiredClay));
    m.blob({(k.x0 + k.x1) / 2, kg + 0.6f, (k.z0 + k.z1) / 2}, (k.x1 - k.x0) / 2, 0.9f, L.clay(kClay));

    // Finished pots drying in rows beside the house.
    for (int row = 0; row < 2; ++row)
        for (int i = 0; i < 5; ++i) {
            const float x = 6.0f + 0.7f * static_cast<float>(i), z = -9.2f + 0.7f * static_cast<float>(row);
            const float pg = V::groundHeight(x, z);
            m.cylinder({x, pg, z}, 0.16f, 0.24f, 0.22f, 10, L.clay(kFiredClay));
            m.cylinder({x, pg + 0.22f, z}, 0.24f, 0.1f, 0.18f, 10, L.clay(kFiredClay));
        }

    // The drying board for the player's own pots: a plank on brick piers.
    const Box2& b = V::kBoard;
    const float top = V::boardTop(), low = V::groundHeight((b.x0 + b.x1) / 2, (b.z0 + b.z1) / 2) - 0.2f;
    m.box({b.x0, top - 0.06f, b.z0}, {b.x1, top, b.z1}, L.other(kWood));
    for (float x : {b.x0 + 0.1f, (b.x0 + b.x1) / 2 - 0.15f, b.x1 - 0.4f})
        m.box({x, low, b.z0 + 0.05f}, {x + 0.3f, top - 0.06f, b.z1 - 0.05f}, L.clay(kBrick));
}

void birdTree(MeshData& m, const Look& L) {
    const Vec2 t = V::kBirdTree;
    const float g = V::groundHeight(t.x, t.z);
    m.cylinder({t.x, g - 0.2f, t.z}, V::kTrunkRadius, 0.35f, 4.4f, 9, L.other(kBark));
    // Two limbs, one for each bird (the birds themselves come later).
    m.box({t.x - 2.2f, g + 3.6f, t.z - 0.1f}, {t.x, g + 3.8f, t.z + 0.1f}, L.other(kBark));
    m.box({t.x, g + 3.0f, t.z - 0.1f}, {t.x + 2.0f, g + 3.2f, t.z + 0.1f}, L.other(kBark));
    m.blob({t.x, g + 5.6f, t.z}, 3.4f, 0.7f, L.other(kLeaves));
    m.blob({t.x - 2.0f, g + 4.9f, t.z + 1.0f}, 2.2f, 0.7f, L.other(kLeavesLight));
    m.blob({t.x + 2.2f, g + 5.0f, t.z - 0.8f}, 2.3f, 0.7f, L.other(kLeaves));
}

void shadeTree(MeshData& m, Vec2 t, int i, const Look& L) {
    const float g = V::groundHeight(t.x, t.z);
    const float h = 3.2f + 0.4f * static_cast<float>(i % 3);
    m.cylinder({t.x, g - 0.2f, t.z}, V::kTreeTrunkRadius, 0.25f, h, 7, L.other(kBark));
    m.blob({t.x, g + h + 1.2f, t.z}, 2.6f + 0.3f * static_cast<float>(i % 2), 0.75f, i % 2 ? L.other(kLeaves) : L.other(kLeavesLight));
}

void ghat(MeshData& m, const Look& L) {
    const V::Ghat& gh = V::kGhat;
    for (int i = 0; i < gh.steps; ++i) {
        const float x0 = gh.x0 + gh.stepDepth() * static_cast<float>(i);
        const float top = gh.stepHeight(i);
        m.box({x0, V::kRiverbedY - 0.2f, gh.z0}, {x0 + gh.stepDepth(), top, gh.z1}, i >= gh.steps - 2 ? L.other(kStoneWet) : L.other(kStone));
    }
    // Low walls at either end of the steps.
    for (float z : {gh.z0 - 0.5f, gh.z1}) m.box({gh.x0, V::kRiverbedY - 0.2f, z}, {gh.x1, gh.top + 0.5f, z + 0.5f}, L.other(kStone));
}

// The coil's centre line: a flat spiral a little over two turns, from the
// centre outward. `head` lifts the last few points, as a snake raises its head.
std::vector<Vec3> spiral(int i, float head) {
    const Vec2 c = V::kCoils[i];
    const float g = V::groundHeight(c.x, c.z);
    const int n = 40, lift = 7;
    std::vector<Vec3> pts;
    for (int k = 0; k <= n; ++k) {
        const float t = static_cast<float>(k) / static_cast<float>(n);
        const float a = 0.35f * static_cast<float>(k) + 0.9f * static_cast<float>(i);
        const float r = 0.08f + (V::kCoilRadius - 0.1f) * t;
        float y = g + 0.04f;
        if (k > n - lift) y += head * static_cast<float>(k - (n - lift)) / static_cast<float>(lift);
        pts.push_back({c.x + r * std::cos(a), y, c.z + r * std::sin(a)});
    }
    return pts;
}

// A square tube along `pts`, one oriented box per segment, alternating colours.
void tube(MeshData& m, const std::vector<Vec3>& pts, float half, Rgb a, Rgb b) {
    for (size_t k = 0; k + 1 < pts.size(); ++k) {
        const Vec3 d = pts[k + 1] - pts[k];
        const float len = length(d);
        if (len <= 0) continue;
        const Vec3 f = d * (1.0f / len);
        const Vec3 s = normalize(cross({0, 1, 0}, f)), u = cross(f, s);
        const Vec3 o = pts[k] - s * half - u * half;
        m.orientedBox(o, s * (2 * half), u * (2 * half), f * (len + half), (k / 2) % 2 ? b : a);
    }
}

// Each form's profile as it turns on the wheel: (radius, height) from the
// foot up to the rim. The sim's FormShape gives the same height and widest
// radius (tests/test_geometry.cpp checks).
const std::vector<Vec2>& profile(Form f) {
    static const std::vector<Vec2> kProfiles[kFormCount] = {
        {{0.13f, 0}, {0.22f, 0.08f}, {0.26f, 0.2f}, {0.22f, 0.32f}, {0.12f, 0.39f}, {0.1f, 0.42f}, {0.12f, 0.46f}},
        {{0.09f, 0}, {0.18f, 0.05f}, {0.23f, 0.11f}, {0.24f, 0.14f}},
        {{0.06f, 0}, {0.08f, 0.08f}, {0.09f, 0.16f}},
        {{0.04f, 0}, {0.09f, 0.03f}, {0.11f, 0.06f}},
    };
    return kProfiles[static_cast<int>(f)];
}

// A disc facing up, counter-clockwise seen from above.
void disc(MeshData& m, Vec3 c, float radius, int segments, Rgb color) {
    const float twoPi = 2.0f * kPi;
    for (int i = 0; i < segments; ++i) {
        const float a0 = twoPi * static_cast<float>(i) / static_cast<float>(segments);
        const float a1 = twoPi * static_cast<float>(i + 1) / static_cast<float>(segments);
        m.triangle(c, {c.x + radius * std::cos(a1), c.y, c.z + radius * std::sin(a1)},
                   {c.x + radius * std::cos(a0), c.y, c.z + radius * std::sin(a0)}, color);
    }
}

}  // namespace

void MeshData::triangle(Vec3 a, Vec3 b, Vec3 c, Rgb color) {
    const Vec3 n = normalize(cross(b - a, c - a));
    const auto base = static_cast<uint32_t>(vertexCount());
    for (Vec3 p : {a, b, c}) {
        positions.insert(positions.end(), {p.x, p.y, p.z});
        normals.insert(normals.end(), {n.x, n.y, n.z});
        colors.insert(colors.end(), {color.r, color.g, color.b});
    }
    indices.insert(indices.end(), {base, base + 1, base + 2});
}

void MeshData::quad(Vec3 a, Vec3 b, Vec3 c, Vec3 d, Rgb color) {
    const Vec3 n = normalize(cross(b - a, c - a));
    const auto base = static_cast<uint32_t>(vertexCount());
    for (Vec3 p : {a, b, c, d}) {
        positions.insert(positions.end(), {p.x, p.y, p.z});
        normals.insert(normals.end(), {n.x, n.y, n.z});
        colors.insert(colors.end(), {color.r, color.g, color.b});
    }
    indices.insert(indices.end(), {base, base + 1, base + 2, base, base + 2, base + 3});
}

void MeshData::box(Vec3 lo, Vec3 hi, Rgb color) {
    orientedBox(lo, {hi.x - lo.x, 0, 0}, {0, hi.y - lo.y, 0}, {0, 0, hi.z - lo.z}, color);
}

void MeshData::orientedBox(Vec3 o, Vec3 ex, Vec3 ey, Vec3 ez, Rgb color) {
    auto p = [&](int a, int b, int c) {
        return o + ex * static_cast<float>(a) + ey * static_cast<float>(b) + ez * static_cast<float>(c);
    };
    quad(p(0, 0, 1), p(1, 0, 1), p(1, 1, 1), p(0, 1, 1), color);  // +ez
    quad(p(1, 0, 0), p(0, 0, 0), p(0, 1, 0), p(1, 1, 0), color);  // -ez
    quad(p(1, 0, 1), p(1, 0, 0), p(1, 1, 0), p(1, 1, 1), color);  // +ex
    quad(p(0, 0, 0), p(0, 0, 1), p(0, 1, 1), p(0, 1, 0), color);  // -ex
    quad(p(0, 1, 1), p(1, 1, 1), p(1, 1, 0), p(0, 1, 0), color);  // +ey
    quad(p(0, 0, 0), p(1, 0, 0), p(1, 0, 1), p(0, 0, 1), color);  // -ey
}

void MeshData::cylinder(Vec3 base, float radiusBottom, float radiusTop, float height, int segments, Rgb color) {
    const float twoPi = 2.0f * kPi;
    for (int i = 0; i < segments; ++i) {
        const float a0 = twoPi * static_cast<float>(i) / static_cast<float>(segments);
        const float a1 = twoPi * static_cast<float>(i + 1) / static_cast<float>(segments);
        const float c0 = std::cos(a0), s0 = std::sin(a0), c1 = std::cos(a1), s1 = std::sin(a1);
        const Vec3 b0{base.x + radiusBottom * c0, base.y, base.z + radiusBottom * s0};
        const Vec3 b1{base.x + radiusBottom * c1, base.y, base.z + radiusBottom * s1};
        const Vec3 t0{base.x + radiusTop * c0, base.y + height, base.z + radiusTop * s0};
        const Vec3 t1{base.x + radiusTop * c1, base.y + height, base.z + radiusTop * s1};
        // Angles run from +x toward +z, which is clockwise seen from above,
        // so the outward face is b1, b0, t0, t1.
        quad(b1, b0, t0, t1, color);
        triangle({base.x, base.y + height, base.z}, t1, t0, color);
    }
}

void MeshData::blob(Vec3 c, float radius, float squash, Rgb color) {
    const int rings = 5, segments = 9;
    auto at = [&](int ring, int seg) {
        const float phi = kPi * static_cast<float>(ring) / static_cast<float>(rings);           // 0 top .. pi bottom
        const float theta = 2.0f * kPi * static_cast<float>(seg) / static_cast<float>(segments);
        return Vec3{c.x + radius * std::sin(phi) * std::cos(theta), c.y + radius * squash * std::cos(phi),
                    c.z + radius * std::sin(phi) * std::sin(theta)};
    };
    for (int r = 0; r < rings; ++r)
        for (int s = 0; s < segments; ++s) {
            const Vec3 a = at(r, s), b = at(r, s + 1), d = at(r + 1, s), e = at(r + 1, s + 1);
            // Seen from outside, (r, s) -> (r+1, s) -> (r+1, s+1) runs counter-clockwise.
            if (r > 0) triangle(a, b, d, color);
            if (r < rings - 1) triangle(b, e, d, color);
        }
}

TerrainData buildTerrain(bool lens) {
    const Look L{lens};
    TerrainData t;
    t.samples = V::kSamples;
    t.origin[0] = V::kMin;
    t.origin[1] = 0;
    t.origin[2] = V::kMin;
    t.cell = V::kCell;
    const size_t n = static_cast<size_t>(t.samples) * static_cast<size_t>(t.samples);
    t.heights.resize(n);
    t.colors.resize(n * 3);
    for (int iz = 0; iz < t.samples; ++iz)
        for (int ix = 0; ix < t.samples; ++ix) {
            const float x = V::kMin + V::kCell * static_cast<float>(ix), z = V::kMin + V::kCell * static_cast<float>(iz);
            const float h = V::groundHeight(x, z);
            const size_t i = static_cast<size_t>(iz) * static_cast<size_t>(t.samples) + static_cast<size_t>(ix);
            t.heights[i] = h;
            // Grass on the terrace, dry ochre on the hills, river mud toward the water.
            Rgb c{0.12f, 0.17f, 0.05f};
            const float hill = clampf((h - 1.5f) / 8.0f, 0.0f, 1.0f);
            c = {c.r + (0.30f - c.r) * hill, c.g + (0.22f - c.g) * hill, c.b + (0.09f - c.b) * hill};
            const float mud = clampf((0.2f - h) / 1.2f, 0.0f, 1.0f);
            c = {c.r + (0.17f - c.r) * mud, c.g + (0.12f - c.g) * mud, c.b + (0.07f - c.b) * mud};
            // A worn path from your house past the well to the ghat.
            const float pathZ = 4.0f + (x - 20.0f) * -0.2f;
            if (x > -8.0f && x < 24.0f && std::fabs(z - pathZ) < 1.2f) c = {0.26f, 0.19f, 0.11f};
            // Through the lens, the riverbank is the clay the pots came from.
            if (lens) {
                const Rgb g = L.other(c);
                c = {g.r + (kLensClay.r - g.r) * mud, g.g + (kLensClay.g - g.g) * mud, g.b + (kLensClay.b - g.b) * mud};
            }
            t.colors[i * 3 + 0] = c.r;
            t.colors[i * 3 + 1] = c.g;
            t.colors[i * 3 + 2] = c.b;
        }
    return t;
}

// A saffron pennant on a bamboo pole beside the shrine, seen over the roofs.
void shrinePennant(MeshData& m, const Look& L) {
    const V::House& h = V::kHouses[V::Shrine];
    const float x = h.footprint.x1 + 0.12f, z = h.footprint.z1 + 0.12f;  // at the corner, too close to walk into
    const float g = V::groundHeight(x, z), top = g + 6.5f;
    m.cylinder({x, g, z}, 0.06f, 0.05f, 6.5f, 6, L.other(kWood));
    // Both faces, so it shows from either side.
    const Vec3 a{x, top, z}, b{x, top - 1.0f, z}, c{x + 1.6f, top - 0.45f, z + 0.2f};
    m.triangle(a, b, c, L.other(kSaffron));
    m.triangle(a, c, b, L.other(kSaffron));
}

// The clay bank: a heap of wet clay where the bank runs down to the water,
// and the hollow it was dug from.
void clayBank(MeshData& m, const Look& L) {
    const Vec2 c = V::kClayBank;
    const float g = V::groundHeight(c.x + 0.8f, c.z);
    m.blob({c.x + 0.8f, g, c.z + 0.3f}, 0.45f, 0.45f, L.clay(kWetClay));
    m.blob({c.x + 0.9f, g, c.z - 0.5f}, 0.3f, 0.4f, L.clay(kClay));
}

MeshData buildVillage(bool lens) {
    const Look L{lens};
    MeshData m;
    for (int i = 0; i < V::kHouseCount; ++i) {
        const Rgb door = i == V::YourHouse ? kDoorIndigo : i == V::PotterHouse ? kDoorOchre : i == V::Shrine ? kSaffron : kDoor;
        house(m, V::kHouses[i], i == V::Shrine ? kShrineWash : kLimewash, door, L);
    }
    shrinePennant(m, L);
    well(m, L);
    potterYard(m, L);
    birdTree(m, L);
    for (int i = 0; i < V::kTreeCount; ++i) shadeTree(m, V::kTrees[i], i, L);
    ghat(m, L);
    clayBank(m, L);
    return m;
}

MeshData buildAvatar() {
    MeshData m;
    m.cylinder({0, 0, 0}, 0.16f, 0.2f, 0.8f, 12, kCottonShade);     // dhoti
    m.cylinder({0, 0.8f, 0}, 0.2f, 0.17f, 0.62f, 12, kCotton);      // kurta
    m.blob({0, 1.55f, 0}, 0.13f, 1.15f, kSkin);                     // head
    m.blob({0, 1.62f, 0.03f}, 0.125f, 0.8f, kHair);
    return m;
}

MeshData buildWater(bool lens) {
    const Look L{lens};
    MeshData m;
    const float y = V::kWaterY;
    // A strip of flat quads, so the surface is not one huge triangle pair.
    for (float z = V::kMin; z < V::kMax; z += 10.0f)
        m.quad({V::kRiverWest - 3.0f, y, z + 10.0f}, {V::kRiverEast + 3.0f, y, z + 10.0f}, {V::kRiverEast + 3.0f, y, z},
               {V::kRiverWest - 3.0f, y, z}, L.other(kWater));
    return m;
}


MeshData buildSnake(int i) {
    MeshData m;
    const std::vector<Vec3> pts = spiral(i, 0.32f);
    tube(m, pts, 0.055f, kSnake, kSnakeBand);
    // The hood: a flattened blob just behind the raised head.
    m.blob(pts.back(), 0.09f, 0.55f, kSnake);
    return m;
}

MeshData buildRope(int i) {
    MeshData m;
    tube(m, spiral(i, 0.0f), 0.04f, kRope, kRopeTwist);
    return m;
}


MeshData buildForm(Form form, Vec3 base, bool lens) {
    const Look L{lens};
    MeshData m;
    const std::vector<Vec2>& p = profile(form);
    for (size_t i = 0; i + 1 < p.size(); ++i)
        m.cylinder({base.x, base.y + p[i].z, base.z}, p[i].x, p[i + 1].x, p[i + 1].z - p[i].z, 14, L.clay(kFiredClay));
    // The mouth: a dark disc just inside the rim, where the hollow begins.
    const Vec2 rim = p.back();
    disc(m, {base.x, base.y + rim.z + 0.002f, base.z}, rim.x * 0.82f, 14, L.clay(kClayDark));
    return m;
}

MeshData buildPot(int slot, Form form, bool lens) { return buildForm(form, potBase(slot), lens); }

MeshData buildShards(int slot, Form form, bool lens) {
    const Look L{lens};
    MeshData m;
    const Vec3 b = potBase(slot);
    const std::vector<Vec2>& p = profile(form);
    // The foot stays where it stood; the rest lies around it in pieces.
    m.cylinder(b, p[0].x, p[1].x * 0.9f, 0.03f, 12, L.clay(kFiredClay));
    for (int i = 0; i < 7; ++i) {
        const float a = 0.9f * static_cast<float>(i) + 0.7f * static_cast<float>(slot);
        const float r = 0.16f + 0.06f * static_cast<float>(i % 3);
        const Vec3 c{b.x + r * std::cos(a), b.y, b.z + 0.6f * r * std::sin(a)};
        const Vec3 f{std::cos(a + 1.1f), 0, std::sin(a + 1.1f)};
        const Vec3 s = normalize(cross({0, 1, 0}, f)), u = cross(f, s);
        const float len = 0.09f + 0.03f * static_cast<float>(i % 3);
        m.orientedBox(c - s * 0.035f, s * 0.07f + u * 0.012f, u * 0.016f, f * len, L.clay(i % 2 ? kFiredClay : kClay));
    }
    return m;
}


void formExtent(Form form, float& radius, float& height) {
    radius = 0;
    for (Vec2 p : profile(form)) radius = std::fmax(radius, p.x);
    height = profile(form).back().z;
}

}  // namespace atman::bridge
