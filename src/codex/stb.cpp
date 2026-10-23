#include "codex/stb.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_STATIC  // Strata links its own copy; keep this one private
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#include "stb_image.h"

namespace atman::codex {

bool decodePng(const unsigned char* data, size_t size, int& width, int& height, std::vector<uint8_t>& rgba) {
    int channels = 0;
    unsigned char* pixels = stbi_load_from_memory(data, static_cast<int>(size), &width, &height, &channels, 4);
    if (!pixels) return false;
    rgba.assign(pixels, pixels + static_cast<size_t>(width) * static_cast<size_t>(height) * 4);
    stbi_image_free(pixels);
    return true;
}

}  // namespace atman::codex
