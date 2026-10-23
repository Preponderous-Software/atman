// Uploads the valley to Strata: the terrain tile, the village and the river
// once, and the coils on the path as they are seen (nothing, a snake, a rope)
// whenever that changes.
#pragma once

#include <cstddef>
#include <array>
#include <cstdint>

#include "bridge/geometry.h"
#include "sim/world.h"
#include "strata/strata.hpp"

namespace atman::bridge {

class Scene {
public:
    explicit Scene(strata::Engine& engine);
    ~Scene();
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    // Brings the coils up to date with the world.
    void update(const World& w);

    size_t meshCount() const { return meshes_; }

private:
    enum class Seen { Nothing, Snake, Rope };

    uint32_t upload(const MeshData& m);
    void remove(uint32_t& id);

    strata::Engine& engine_;
    uint32_t village_ = 0, water_ = 0;
    std::array<uint32_t, valley::kCoilCount> coilMesh_{};
    std::array<Seen, valley::kCoilCount> coilSeen_{};
    size_t meshes_ = 0;
};

}  // namespace atman::bridge
