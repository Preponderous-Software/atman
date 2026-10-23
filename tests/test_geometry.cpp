// The valley's meshes and terrain: valid for Strata (index ranges, counts),
// faces pointing outward (Strata culls back faces), the terrain matching the
// ground the rules walk on, and the river below the village.
#include <cmath>

#include "bridge/geometry.h"
#include "check.h"
#include "sim/valley.h"

using namespace atman;
using namespace atman::bridge;
namespace V = atman::valley;

namespace {

bool valid(const MeshData& m) {
    if (m.positions.size() % 3 != 0 || m.normals.size() != m.positions.size() || m.colors.size() != m.positions.size())
        return false;
    if (m.indices.empty() || m.indices.size() % 3 != 0) return false;
    for (uint32_t i : m.indices)
        if (i >= m.vertexCount()) return false;
    for (float f : m.positions)
        if (!std::isfinite(f)) return false;
    for (size_t v = 0; v < m.vertexCount(); ++v) {
        const float x = m.normals[v * 3], y = m.normals[v * 3 + 1], z = m.normals[v * 3 + 2];
        if (std::fabs(std::sqrt(x * x + y * y + z * z) - 1) > 1e-3f) return false;
    }
    for (float c : m.colors)
        if (c < 0 || c > 1) return false;
    return true;
}

// Fraction of triangles whose normal points away from `centre`.
float outward(const MeshData& m, Vec3 centre) {
    int out = 0, total = 0;
    for (size_t t = 0; t < m.indices.size(); t += 3) {
        Vec3 mid{0, 0, 0};
        for (int k = 0; k < 3; ++k) {
            const uint32_t i = m.indices[t + k];
            mid = mid + Vec3{m.positions[i * 3], m.positions[i * 3 + 1], m.positions[i * 3 + 2]} * (1.0f / 3);
        }
        const uint32_t i0 = m.indices[t];
        const Vec3 n{m.normals[i0 * 3], m.normals[i0 * 3 + 1], m.normals[i0 * 3 + 2]};
        if (dot(n, mid - centre) > 0) ++out;
        ++total;
    }
    return total ? static_cast<float>(out) / static_cast<float>(total) : 0;
}

}  // namespace

int main() {
    // Each building block faces outward on its own.
    {
        MeshData box;
        box.box({0, 0, 0}, {2, 1, 3}, {});
        CHECK(valid(box));
        CHECK(outward(box, {1, 0.5f, 1.5f}) == 1.0f);
        MeshData cyl;
        cyl.cylinder({0, 0, 0}, 1, 0.5f, 2, 12, {});
        CHECK(valid(cyl));
        CHECK(outward(cyl, {0, 0.5f, 0}) == 1.0f);
        MeshData blob;
        blob.blob({0, 5, 0}, 2, 0.7f, {});
        CHECK(valid(blob));
        CHECK(outward(blob, {0, 5, 0}) == 1.0f);
    }

    // A house faces outward too (its roof and gables included).
    {
        const MeshData village = buildVillage();
        CHECK(valid(village));
        CHECK(village.vertexCount() < 200000);
        // Your house: every triangle inside its footprint (with eaves).
        const Box2 f = V::kHouses[V::YourHouse].footprint;
        const Vec3 centre{(f.x0 + f.x1) / 2, V::groundHeight((f.x0 + f.x1) / 2, (f.z0 + f.z1) / 2) + 1.5f,
                          (f.z0 + f.z1) / 2};
        int inside = 0, outwardCount = 0;
        for (size_t t = 0; t < village.indices.size(); t += 3) {
            Vec3 mid{0, 0, 0};
            for (int k = 0; k < 3; ++k) {
                const uint32_t i = village.indices[t + k];
                mid = mid + Vec3{village.positions[i * 3], village.positions[i * 3 + 1], village.positions[i * 3 + 2]} *
                                (1.0f / 3);
            }
            if (mid.x < f.x0 - 1 || mid.x > f.x1 + 1 || mid.z < f.z0 - 1 || mid.z > f.z1 + 1) continue;
            if (mid.y < centre.y - 1.0f) continue;  // the plinth's top faces up toward the walls; skip it
            const uint32_t i0 = village.indices[t];
            const Vec3 n{village.normals[i0 * 3], village.normals[i0 * 3 + 1], village.normals[i0 * 3 + 2]};
            ++inside;
            if (dot(n, mid - centre) > 0) ++outwardCount;
        }
        CHECK(inside > 10);
        CHECK(outwardCount == inside);
    }

    // The river's surface faces up and lies at the water line.
    {
        const MeshData water = buildWater();
        CHECK(valid(water));
        for (size_t v = 0; v < water.vertexCount(); ++v) {
            CHECK(water.normals[v * 3 + 1] > 0.99f);
            CHECK(water.positions[v * 3 + 1] == V::kWaterY);
        }
    }

    // The terrain grid is the ground the rules walk on.
    {
        const TerrainData t = buildTerrain();
        CHECK(t.samples == V::kSamples);
        CHECK(V::kMin + V::kCell * static_cast<float>(t.samples - 1) == V::kMax);
        CHECK(t.heights.size() == static_cast<size_t>(t.samples * t.samples));
        CHECK(t.colors.size() == t.heights.size() * 3);
        for (int iz = 0; iz < t.samples; iz += 17)
            for (int ix = 0; ix < t.samples; ix += 13) {
                const float x = V::kMin + V::kCell * static_cast<float>(ix), z = V::kMin + V::kCell * static_cast<float>(iz);
                CHECK(t.heights[static_cast<size_t>(iz * t.samples + ix)] == V::groundHeight(x, z));
            }
        for (float c : t.colors) CHECK(c >= 0 && c <= 1);
    }

    // The village stands above the river; the riverbed is under the water.
    {
        for (const V::House& h : V::kHouses) {
            const float cx = (h.footprint.x0 + h.footprint.x1) / 2, cz = (h.footprint.z0 + h.footprint.z1) / 2;
            CHECK(V::groundHeight(cx, cz) > V::kWaterY + 0.5f);
            CHECK(std::fabs(V::groundHeight(cx, cz)) < 1.0f);  // the terrace is nearly flat
        }
        CHECK(V::groundHeight((V::kRiverWest + V::kRiverEast) / 2, 0) < V::kWaterY - 1.0f);
        CHECK(V::groundHeight(V::kWell.x, V::kWell.z) > V::kWaterY);
    }

    TEST_MAIN_END();
}
