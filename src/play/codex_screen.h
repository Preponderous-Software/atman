// The codex screen: the open entries listed in the plate's dark left column,
// and the chosen entry's plate filling the screen (in a window taller than it
// is wide, the page alone across its width, with the list above it). Plates are uploaded the
// first time they are shown. While the screen is open the valley is not drawn
// (the game pauses; see app/main.cpp).
#pragma once

#include <array>
#include <cstdint>

#include "codex/codex.h"
#include "sim/world.h"
#include "strata/strata.hpp"

namespace atman::play {

class CodexScreen {
public:
    explicit CodexScreen(strata::Engine& engine);
    ~CodexScreen();
    CodexScreen(const CodexScreen&) = delete;
    CodexScreen& operator=(const CodexScreen&) = delete;

    bool isOpen() const { return open_; }
    // Opens on the entry chosen last time, or on the first open one.
    void open(const World& w);
    void close();
    // Moves the choice up (-1) or down (+1) the list of open entries.
    void move(const World& w, int step);
    // Opens on a given entry (an index into codex::kEntries), for the tour.
    void show(const World& w, int entry);
    // Whether the keys are named under the list (off on a touch screen,
    // where the page has buttons for them).
    void setKeyHints(bool on) { keys_ = on; }

    // Draws one frame (not presented).
    void render(const World& w);

private:
    void choose(const World& w, int entry);

    strata::Engine& engine_;
    bool open_ = false;
    bool keys_ = true;
    int chosen_ = -1;  // index into codex::kEntries
    uint32_t mesh_ = 0;
    int meshEntry_ = -1;  // what the quad shows, and the window shape it was built for
    float meshAspect_ = 0;
    float meshOffset_ = 0;
    std::array<uint32_t, 16> textures_{};
};

}  // namespace atman::play
