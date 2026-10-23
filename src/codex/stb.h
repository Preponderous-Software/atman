// The one use of stb_image: decoding a PNG held in memory. Kept to its own
// source file, which is built without the project's warning flags.
#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

namespace atman::codex {

// Decodes to RGBA8, top row first. False if the bytes are not a PNG.
bool decodePng(const unsigned char* data, size_t size, int& width, int& height, std::vector<uint8_t>& rgba);

}  // namespace atman::codex
