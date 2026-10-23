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

// Village palette (linear RGB): lime-washed walls, fired-tile roofs, laterite
// stone, river clay.
constexpr Rgb kLimewash{0.62f, 0.55f, 0.42f};
constexpr Rgb kLimewashShaded{0.50f, 0.44f, 0.33f};
constexpr Rgb kTile{0.36f, 0.12f, 0.06f};
constexpr Rgb kDoor{0.16f, 0.09f, 0.05f};
constexpr Rgb kPlinth{0.30f, 0.24f, 0.18f};
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
constexpr Rgb kSnake{0.05f, 0.05f, 0.025f};
constexpr Rgb kSnakeBand{0.13f, 0.11f, 0.05f};
constexpr Rgb kRope{0.44f, 0.35f, 0.17f};
constexpr Rgb kRopeTwist{0.36f, 0.28f, 0.13f};

void house(MeshData& m, const V::House& h, Rgb wash) {
    const Box2& f = h.footprint;
    const float x0 = f.x0, x1 = f.x1, z0 = f.z0, z1 = f.z1;
    // Sit the house on the lowest ground under it, on a plinth.
    float base = V::groundHeight(x0, z0);
    for (float x : {x0, x1})
        for (float z : {z0, z1}) base = std::fmin(base, V::groundHeight(x, z));
    const float plinth = base + 0.35f;
    m.box({x0 - 0.2f, base - 0.5f, z0 - 0.2f}, {x1 + 0.2f, plinth, z1 + 0.2f}, kPlinth);
    const float top = plinth + h.wallHeight;
    m.box({x0, plinth, z0}, {x1, top, z1}, wash);

    // The door: a dark panel just proud of the wall it faces.
    const float dw = 0.55f, dh = 1.9f, e = 0.02f;
    const Vec2 d = h.door;
    switch (h.doorFace) {
        case 0: m.quad({d.x + dw, plinth, z0 - e}, {d.x - dw, plinth, z0 - e}, {d.x - dw, plinth + dh, z0 - e}, {d.x + dw, plinth + dh, z0 - e}, kDoor); break;
        case 1: m.quad({x1 + e, plinth, d.z + dw}, {x1 + e, plinth, d.z - dw}, {x1 + e, plinth + dh, d.z - dw}, {x1 + e, plinth + dh, d.z + dw}, kDoor); break;
        case 2: m.quad({d.x - dw, plinth, z1 + e}, {d.x + dw, plinth, z1 + e}, {d.x + dw, plinth + dh, z1 + e}, {d.x - dw, plinth + dh, z1 + e}, kDoor); break;
        default: m.quad({x0 - e, plinth, d.z - dw}, {x0 - e, plinth, d.z + dw}, {x0 - e, plinth + dh, d.z + dw}, {x0 - e, plinth + dh, d.z - dw}, kDoor); break;
    }

    // A gable roof with eaves, its ridge along x or z.
    const float o = 0.45f, ridge = top + h.roofRise;
    const float ex0 = x0 - o, ex1 = x1 + o, ez0 = z0 - o, ez1 = z1 + o, eave = top - 0.2f;
    if (h.ridgeAlongX) {
        const float zm = (z0 + z1) / 2;
        m.quad({ex1, eave, ez0}, {ex0, eave, ez0}, {ex0, ridge, zm}, {ex1, ridge, zm}, kTile);    // north slope
        m.quad({ex0, eave, ez1}, {ex1, eave, ez1}, {ex1, ridge, zm}, {ex0, ridge, zm}, kTile);    // south slope
        m.triangle({x0, top, z0}, {x0, top, z1}, {x0, ridge - 0.05f, zm}, kLimewashShaded);    // west gable
        m.triangle({x1, top, z1}, {x1, top, z0}, {x1, ridge - 0.05f, zm}, kLimewashShaded);    // east gable
    } else {
        const float xm = (x0 + x1) / 2;
        m.quad({ex0, eave, ez0}, {ex0, eave, ez1}, {xm, ridge, ez1}, {xm, ridge, ez0}, kTile);    // west slope
        m.quad({ex1, eave, ez1}, {ex1, eave, ez0}, {xm, ridge, ez0}, {xm, ridge, ez1}, kTile);    // east slope
        m.triangle({x1, top, z0}, {x0, top, z0}, {xm, ridge - 0.05f, z0}, kLimewashShaded);    // north gable
        m.triangle({x0, top, z1}, {x1, top, z1}, {xm, ridge - 0.05f, z1}, kLimewashShaded);    // south gable
    }
}

void well(MeshData& m) {
    const float g = V::groundHeight(V::kWell.x, V::kWell.z);
    const Vec3 base{V::kWell.x, g - 0.2f, V::kWell.z};
    m.cylinder(base, V::kWellRadius, V::kWellRadius, V::kWellHeight + 0.2f, 14, kStone);
    // The water inside, dark, just below the rim.
    m.cylinder({base.x, base.y, base.z}, V::kWellRadius - 0.2f, V::kWellRadius - 0.2f, V::kWellHeight, 14, kWellWater);
    // Two posts and a beam for the rope.
    const float r = V::kWellRadius - 0.1f, top = g + V::kWellHeight + 1.3f;
    m.box({base.x - r - 0.08f, g, base.z - 0.08f}, {base.x - r + 0.08f, top, base.z + 0.08f}, kWood);
    m.box({base.x + r - 0.08f, g, base.z - 0.08f}, {base.x + r + 0.08f, top, base.z + 0.08f}, kWood);
    m.box({base.x - r - 0.1f, top - 0.12f, base.z - 0.07f}, {base.x + r + 0.1f, top, base.z + 0.07f}, kWood);
}

void potterYard(MeshData& m) {
    // The wheel: a low stone base and a wide clay-stained wooden disc.
    const float g = V::groundHeight(V::kWheel.x, V::kWheel.z);
    m.cylinder({V::kWheel.x, g, V::kWheel.z}, 0.25f, 0.2f, 0.25f, 10, kStone);
    m.cylinder({V::kWheel.x, g + 0.25f, V::kWheel.z}, V::kWheelRadius, V::kWheelRadius, 0.08f, 16, kWood);
    m.cylinder({V::kWheel.x, g + 0.33f, V::kWheel.z}, 0.16f, 0.12f, 0.22f, 10, kClay);  // a pot half thrown

    // The kiln: a squat clay dome over a brick base.
    const Box2& k = V::kKiln;
    const float kg = V::groundHeight((k.x0 + k.x1) / 2, (k.z0 + k.z1) / 2);
    m.box({k.x0, kg - 0.3f, k.z0}, {k.x1, kg + 0.6f, k.z1}, kFiredClay);
    m.blob({(k.x0 + k.x1) / 2, kg + 0.6f, (k.z0 + k.z1) / 2}, (k.x1 - k.x0) / 2, 0.9f, kClay);

    // Finished pots drying in rows beside the house.
    for (int row = 0; row < 2; ++row)
        for (int i = 0; i < 5; ++i) {
            const float x = 6.0f + 0.7f * static_cast<float>(i), z = -9.2f + 0.7f * static_cast<float>(row);
            const float pg = V::groundHeight(x, z);
            m.cylinder({x, pg, z}, 0.16f, 0.24f, 0.22f, 10, kFiredClay);
            m.cylinder({x, pg + 0.22f, z}, 0.24f, 0.1f, 0.18f, 10, kFiredClay);
        }
}

void birdTree(MeshData& m) {
    const Vec2 t = V::kBirdTree;
    const float g = V::groundHeight(t.x, t.z);
    m.cylinder({t.x, g - 0.2f, t.z}, V::kTrunkRadius, 0.35f, 4.4f, 9, kBark);
    // Two limbs, one for each bird (the birds themselves come later).
    m.box({t.x - 2.2f, g + 3.6f, t.z - 0.1f}, {t.x, g + 3.8f, t.z + 0.1f}, kBark);
    m.box({t.x, g + 3.0f, t.z - 0.1f}, {t.x + 2.0f, g + 3.2f, t.z + 0.1f}, kBark);
    m.blob({t.x, g + 5.6f, t.z}, 3.4f, 0.7f, kLeaves);
    m.blob({t.x - 2.0f, g + 4.9f, t.z + 1.0f}, 2.2f, 0.7f, kLeavesLight);
    m.blob({t.x + 2.2f, g + 5.0f, t.z - 0.8f}, 2.3f, 0.7f, kLeaves);
}

void shadeTree(MeshData& m, Vec2 t, int i) {
    const float g = V::groundHeight(t.x, t.z);
    const float h = 3.2f + 0.4f * static_cast<float>(i % 3);
    m.cylinder({t.x, g - 0.2f, t.z}, V::kTreeTrunkRadius, 0.25f, h, 7, kBark);
    m.blob({t.x, g + h + 1.2f, t.z}, 2.6f + 0.3f * static_cast<float>(i % 2), 0.75f, i % 2 ? kLeaves : kLeavesLight);
}

void ghat(MeshData& m) {
    const V::Ghat& gh = V::kGhat;
    for (int i = 0; i < gh.steps; ++i) {
        const float x0 = gh.x0 + gh.stepDepth() * static_cast<float>(i);
        const float top = gh.stepHeight(i);
        m.box({x0, V::kRiverbedY - 0.2f, gh.z0}, {x0 + gh.stepDepth(), top, gh.z1}, i >= gh.steps - 2 ? kStoneWet : kStone);
    }
    // Low walls at either end of the steps.
    for (float z : {gh.z0 - 0.5f, gh.z1}) m.box({gh.x0, V::kRiverbedY - 0.2f, z}, {gh.x1, gh.top + 0.5f, z + 0.5f}, kStone);
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

TerrainData buildTerrain() {
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
            t.colors[i * 3 + 0] = c.r;
            t.colors[i * 3 + 1] = c.g;
            t.colors[i * 3 + 2] = c.b;
        }
    return t;
}

MeshData buildVillage() {
    MeshData m;
    for (int i = 0; i < V::kHouseCount; ++i) house(m, V::kHouses[i], i == V::Shrine ? kShrineWash : kLimewash);
    well(m);
    potterYard(m);
    birdTree(m);
    for (int i = 0; i < V::kTreeCount; ++i) shadeTree(m, V::kTrees[i], i);
    ghat(m);
    return m;
}

MeshData buildWater() {
    MeshData m;
    const float y = V::kWaterY;
    // A strip of flat quads, so the surface is not one huge triangle pair.
    for (float z = V::kMin; z < V::kMax; z += 10.0f)
        m.quad({V::kRiverWest - 3.0f, y, z + 10.0f}, {V::kRiverEast + 3.0f, y, z + 10.0f}, {V::kRiverEast + 3.0f, y, z},
               {V::kRiverWest - 3.0f, y, z}, kWater);
    return m;
}

}  // namespace atman::bridge

namespace atman::bridge {

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

}  // namespace atman::bridge
