#include "play/codex_screen.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace atman::play {

namespace {

// The plate hangs far above the valley, in front of a camera of its own, so
// nothing of the valley is in view.
constexpr float kFovY = 70.0f;
constexpr float kAway = 5000.0f;
constexpr float kDistance = 1.0f;
constexpr float kPlateAspect = static_cast<float>(codex::kPlateWidth) / codex::kPlateHeight;
// The plate without its menu column: the page alone.
constexpr float kPageAspect = static_cast<float>(codex::kPlateWidth - codex::kMenuWidth) / codex::kPlateHeight;
// The plate's dark column, as the clear colour, so a window wider or taller
// than the plate is framed in it.
constexpr float kPanel[4] = {35 / 255.0f, 26 / 255.0f, 19 / 255.0f, 1.0f};

// How much of the world is in view from top to bottom, in world units.
float visibleHeight() { return 2 * kDistance * std::tan(kFovY * 0.5f * 3.14159265f / 180); }

// How tall a picture of this shape is drawn, in world units, to fit the window.
float fitHeight(float aspect, float shape) {
    return aspect >= shape ? visibleHeight() : visibleHeight() * aspect / shape;
}

}  // namespace

CodexScreen::CodexScreen(strata::Engine& engine) : engine_(engine) {}

CodexScreen::~CodexScreen() {
    if (mesh_) engine_.removeMesh(mesh_);
    for (uint32_t t : textures_)
        if (t) engine_.destroyTexture(t);
}

void CodexScreen::open(const World& w) {
    open_ = true;
    choose(w, chosen_);
}

void CodexScreen::close() {
    open_ = false;
    if (mesh_) engine_.removeMesh(mesh_);
    mesh_ = 0;
}

void CodexScreen::show(const World& w, int entry) {
    open_ = true;
    choose(w, entry);
}

void CodexScreen::choose(const World& w, int entry) {
    const std::vector<int> list = codex::openEntries(w);
    if (std::find(list.begin(), list.end(), entry) == list.end()) entry = list.empty() ? -1 : list.front();
    chosen_ = entry;
}

void CodexScreen::move(const World& w, int step) {
    const std::vector<int> list = codex::openEntries(w);
    if (list.empty()) return;
    auto at = std::find(list.begin(), list.end(), chosen_);
    const int i = at == list.end() ? 0 : static_cast<int>(at - list.begin());
    const int n = static_cast<int>(list.size());
    chosen_ = list[static_cast<size_t>(((i + step) % n + n) % n)];
}

void CodexScreen::render(const World& w) {
    int32_t width = 0, height = 0;
    engine_.drawableSize(width, height);
    const float aspect = height > 0 ? static_cast<float>(width) / static_cast<float>(height) : 1.0f;
    const float eye[3] = {0, kAway, 0}, target[3] = {0, kAway, -kDistance}, up[3] = {0, 1, 0};
    engine_.setCamera(strata::cameraLookAt(eye, target, up, kFovY, aspect, 0.05f, 10.0f));

    // A window taller than it is wide (a phone held upright) shows the page
    // alone across its width, with the menu above it: the whole plate would
    // fit only at a size too small to read.
    const bool upright = aspect < 1;
    const float shape = upright ? kPageAspect : kPlateAspect;
    const float h = fitHeight(aspect, shape), wq = h * shape;
    const float pixelsPerUnit = static_cast<float>(height) / visibleHeight();
    const int listed = static_cast<int>(codex::openEntries(w).size());

    // Where the menu goes, how large its text is, and how far the quad is
    // moved up or down from the middle of the window (world units).
    float size = 0, x = 0, y = 0, offset = 0;
    if (upright) {
        size = std::clamp(std::floor(static_cast<float>(width) / 40), 8.0f, 24.0f);
        x = std::round(static_cast<float>(width) * 0.05f);
        y = size;
        const float pagePixels = h * pixelsPerUnit;
        const float menuBottom = y + static_cast<float>(listed + 2) * size * 1.6f;
        const float pageTop = std::max(menuBottom + size, (static_cast<float>(height) - pagePixels) / 2);
        offset = (static_cast<float>(height) / 2 - (pageTop + pagePixels / 2)) / pixelsPerUnit;
    }

    // Upload the chosen plate the first time it is shown, and put it on the
    // quad. The quad is rebuilt when the window's shape or the plate changes.
    if (chosen_ >= 0 && chosen_ < static_cast<int>(textures_.size()) &&
        (!mesh_ || meshEntry_ != chosen_ || meshAspect_ != aspect || meshOffset_ != offset)) {
        uint32_t& tex = textures_[static_cast<size_t>(chosen_)];
        if (!tex) {
            const codex::Image img = codex::decodePlate(codex::kEntries[chosen_]);
            if (!img.rgba.empty())
                tex = engine_.createTextureRgba8(img.width, img.height, img.rgba.data(),
                                                 STRATA_TEXTURE_MIPMAPS | STRATA_TEXTURE_CLAMP_U | STRATA_TEXTURE_CLAMP_V);
        }
        if (mesh_) engine_.removeMesh(mesh_);
        const float x0 = -wq / 2, x1 = wq / 2, y0 = kAway + offset - h / 2, y1 = kAway + offset + h / 2, z = -kDistance;
        const float u0 = upright ? static_cast<float>(codex::kMenuWidth) / codex::kPlateWidth : 0;
        const float positions[] = {x0, y0, z, x1, y0, z, x1, y1, z, x0, y1, z};
        const float normals[] = {0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1};
        const float uvs[] = {u0, 1, 1, 1, 1, 0, u0, 0};
        const uint32_t indices[] = {0, 1, 2, 0, 2, 3};
        StrataTexturedMesh m{};
        m.mesh.positions = positions;
        m.mesh.normals = normals;
        m.mesh.indices = indices;
        m.mesh.vertex_count = 4;
        m.mesh.index_count = 6;
        for (float& c : m.mesh.color) c = 1;
        m.uvs = uvs;
        m.texture = tex;
        m.flags = STRATA_MESH_UNLIT;
        mesh_ = engine_.addTexturedMesh(m);
        meshEntry_ = chosen_;
        meshAspect_ = aspect;
        meshOffset_ = offset;
    }

    StrataFrameDesc fd{};
    fd.clear_flags = STRATA_CLEAR_COLOR | STRATA_CLEAR_DEPTH;
    for (int i = 0; i < 4; ++i) fd.clear_color[i] = kPanel[i];
    engine_.beginFrame(fd);

    // Otherwise the menu goes in the plate's dark column, sized to fit it.
    const float plateLeft = static_cast<float>(width) / 2 - wq / 2 * pixelsPerUnit;
    const float platePixels = wq * pixelsPerUnit / codex::kPlateWidth;  // screen pixels per plate pixel
    if (!upright) {
        const float column = codex::kMenuWidth * platePixels;
        size = std::clamp(std::floor((column - 64 * platePixels) / 24), 8.0f, 24.0f);
        x = plateLeft + 32 * platePixels;
        y = static_cast<float>(height) / 2 - 450 * platePixels + 56 * platePixels;
    }
    const float line = size * 1.6f;

    std::vector<std::string> keep;
    std::vector<StrataText> texts;
    keep.reserve(16);
    const float ink[4] = {0.93f, 0.89f, 0.80f, 1.0f};
    const float dim[4] = {0.62f, 0.55f, 0.46f, 1.0f};
    const float lamp[4] = {1.0f, 0.62f, 0.22f, 1.0f};
    auto add = [&](float ty, const std::string& s, const float* colour) {
        keep.push_back(s);
        StrataText t{};
        t.x = x;
        t.y = ty;
        t.pixel_size = size;
        for (int i = 0; i < 4; ++i) t.color[i] = colour[i];
        texts.push_back(t);
    };
    add(y, "CODEX", lamp);
    y += 2 * line;
    for (int i : codex::openEntries(w)) {
        add(y, std::string(i == chosen_ ? "> " : "  ") + codex::kEntries[i].menu, i == chosen_ ? ink : dim);
        y += line;
    }
    // Upright, the keys go just under the page.
    const float bottom = upright ? static_cast<float>(height) / 2 - offset * pixelsPerUnit + h * pixelsPerUnit / 2 + 3 * line
                                 : static_cast<float>(height) / 2 + 450 * platePixels - 56 * platePixels;
    if (keys_) {
        add(bottom - 2 * line, "Up/Down  choose", dim);
        add(bottom - line, "Tab  back to the valley", dim);
    }
    for (size_t i = 0; i < texts.size(); ++i) texts[i].utf8 = keep[i].c_str();
    engine_.submitText(texts.data(), texts.size());
    engine_.endFrame();
}

}  // namespace atman::play
