// Uploads the valley to Strata: the terrain tile, the village and the river.
// In M1 the valley never changes, so this happens once.
#pragma once

#include <cstddef>
#include <cstdint>

#include "bridge/geometry.h"
#include "strata/strata.hpp"

namespace atman::bridge {

class Scene {
public:
    explicit Scene(strata::Engine& engine);
    ~Scene();
    Scene(const Scene&) = delete;
    Scene& operator=(const Scene&) = delete;

    size_t meshCount() const { return meshes_; }

private:
    uint32_t upload(const MeshData& m);

    strata::Engine& engine_;
    uint32_t village_ = 0, water_ = 0;
    size_t meshes_ = 0;
};

}  // namespace atman::bridge
