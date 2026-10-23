#include "codex/codex.h"

#include <cstring>

#include "codex/stb.h"

namespace atman::codex {

bool isOpen(const World& w, const Entry& e) { return e.always || w.codex[static_cast<int>(e.unlock)]; }

int find(const char* id) {
    for (int i = 0; i < kEntryCount; ++i)
        if (std::strcmp(kEntries[i].id, id) == 0) return i;
    return -1;
}

std::vector<int> openEntries(const World& w) {
    std::vector<int> open;
    for (int i = 0; i < kEntryCount; ++i)
        if (isOpen(w, kEntries[i])) open.push_back(i);
    return open;
}

Image decodePlate(const Entry& e) {
    Image img;
    if (!decodePng(e.plate, e.plateSize, img.width, img.height, img.rgba)) img = Image{};
    return img;
}

}  // namespace atman::codex
