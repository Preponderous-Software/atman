#include "bridge/scene.h"

namespace atman::bridge {

Scene::Scene(strata::Engine& engine) : engine_(engine) {
    const TerrainData t = buildTerrain();
    StrataTerrainTile tile{};
    tile.origin[0] = t.origin[0];
    tile.origin[1] = t.origin[1];
    tile.origin[2] = t.origin[2];
    tile.cell_size = t.cell;
    tile.samples_x = t.samples;
    tile.samples_z = t.samples;
    tile.heights = t.heights.data();
    tile.colors = t.colors.data();
    tile.color[3] = 1;
    engine_.setTerrain(&tile);

    village_ = upload(buildVillage());
    water_ = upload(buildWater());
}

Scene::~Scene() {
    try {
        if (village_) engine_.removeMesh(village_);
        if (water_) engine_.removeMesh(water_);
        engine_.setTerrain(nullptr);
    } catch (const strata::Error&) {
        // The engine is going away with us; nothing to tidy.
    }
}

uint32_t Scene::upload(const MeshData& m) {
    if (m.empty()) return 0;
    StrataMesh mesh{};
    mesh.positions = m.positions.data();
    mesh.normals = m.normals.data();
    mesh.colors = m.colors.data();
    mesh.indices = m.indices.data();
    mesh.vertex_count = static_cast<int32_t>(m.vertexCount());
    mesh.index_count = static_cast<int32_t>(m.indices.size());
    mesh.color[3] = 1;
    ++meshes_;
    return engine_.addMesh(mesh);
}

}  // namespace atman::bridge
