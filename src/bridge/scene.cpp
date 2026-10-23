#include "bridge/scene.h"

namespace atman::bridge {

Scene::Scene(strata::Engine& engine) : engine_(engine) { buildValley(false); }

void Scene::buildValley(bool lens) {
    const TerrainData t = buildTerrain(lens);
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

    remove(village_);
    remove(water_);
    village_ = upload(buildVillage(lens));
    water_ = upload(buildWater(lens));
    lens_ = lens;
}

void Scene::update(const World& w) {
    const bool lensChanged = w.clayLens != lens_;
    if (lensChanged) buildValley(w.clayLens);

    for (int i = 0; i < valley::kCoilCount; ++i) {
        const Seen seen = snakeVisible(w, i) ? Seen::Snake : isDusk(w) ? Seen::Rope : Seen::Nothing;
        if (seen == coilSeen_[i]) continue;
        remove(coilMesh_[i]);
        if (seen == Seen::Snake) coilMesh_[i] = upload(buildSnake(i));
        else if (seen == Seen::Rope) coilMesh_[i] = upload(buildRope(i));
        coilSeen_[i] = seen;
    }

    for (int i = 0; i < valley::kBoardSlots; ++i) {
        const Pot& p = w.pots[i];
        const Board seen = !p.made ? Board::Empty : p.broken ? Board::Shards : Board::Pot;
        if (seen == potSeen_[i] && !lensChanged) continue;
        remove(potMesh_[i]);
        if (seen == Board::Pot) potMesh_[i] = upload(buildPot(i, p.form, w.clayLens));
        else if (seen == Board::Shards) potMesh_[i] = upload(buildShards(i, p.form, w.clayLens));
        potSeen_[i] = seen;
    }

    // The pot on the wheel rises from a low lump to its full height.
    if (!w.throwing.active) {
        remove(wheelPot_);
        return;
    }
    if (wheelPot_ && (wheelForm_ != w.throwing.form || lensChanged)) remove(wheelPot_);
    if (!wheelPot_) {
        wheelPot_ = upload(buildForm(w.throwing.form, {0, 0, 0}, w.clayLens));
        wheelForm_ = w.throwing.form;
    }
    const float k = clampf(w.throwing.seconds / kThrowSeconds, 0.0f, 1.0f);
    const float wide = 1.3f - 0.3f * k, tall = 0.25f + 0.75f * k;
    const float top = valley::groundHeight(valley::kWheel.x, valley::kWheel.z) + 0.33f;
    const float model[16] = {wide, 0, 0, 0, 0, tall, 0, 0, 0, 0, wide, 0, valley::kWheel.x, top, valley::kWheel.z, 1};
    engine_.setMeshTransform(wheelPot_, model);
}

void Scene::remove(uint32_t& id) {
    if (!id) return;
    engine_.removeMesh(id);
    id = 0;
    --meshes_;
}

Scene::~Scene() {
    try {
        for (uint32_t& id : coilMesh_) remove(id);
        for (uint32_t& id : potMesh_) remove(id);
        remove(wheelPot_);
        remove(village_);
        remove(water_);
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
