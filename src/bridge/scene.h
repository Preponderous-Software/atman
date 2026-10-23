// Uploads the valley to Strata and keeps it in step with the world: the
// terrain tile, the village and the river (again when the clay lens turns on
// or off), the coils on the path as they are seen (nothing, a snake, a rope),
// the pots on the drying board (whole or in shards), the pot rising on the
// wheel while the player throws it, and, while the player watches in the
// after-mode, the avatar.
#pragma once

#include <array>
#include <cstddef>
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

    // Brings the meshes up to date with the world. `avatar` shows the player's
    // body where it stands (for watching it from outside).
    void update(const World& w, bool avatar = false);

    size_t meshCount() const { return meshes_; }

private:
    enum class Seen { Nothing, Snake, Rope };
    enum class Board { Empty, Pot, Shards };

    uint32_t upload(const MeshData& m);
    void remove(uint32_t& id);
    void buildValley(bool lens);

    strata::Engine& engine_;
    bool lens_ = false;
    uint32_t village_ = 0, water_ = 0;
    std::array<uint32_t, valley::kCoilCount> coilMesh_{};
    std::array<Seen, valley::kCoilCount> coilSeen_{};
    std::array<uint32_t, valley::kBoardSlots> potMesh_{};
    std::array<Board, valley::kBoardSlots> potSeen_{};
    uint32_t wheelPot_ = 0;
    uint32_t avatar_ = 0;
    Form wheelForm_ = Form::WaterJar;
    size_t meshes_ = 0;
};

}  // namespace atman::bridge
