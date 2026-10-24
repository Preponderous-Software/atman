// Writes the format-2 save fixtures in tests/fixtures/ by playing the
// autopilot on seed 1 with the current rules and saving through the game's
// own writeSave (the same call the game makes on F5 and on quit). Nothing is
// written by hand. See tests/fixtures/README.md for how it is built and run.
//
//   v2-mid-dream.save        halfway through the first night's dream
//   v2-after-the-fourth.save ten game minutes after the fourth's codex page
//                            opened, in the after-mode
#include <cstdio>
#include <filesystem>
#include <string>

#include "save/save.h"
#include "sim/autopilot.h"
#include "sim/world.h"

using namespace atman;
namespace fs = std::filesystem;

namespace {

bool save(const World& w, const fs::path& out, const fs::path& scratch) {
    std::error_code ec;
    fs::remove_all(scratch, ec);
    std::string why;
    if (!writeSave(scratch.string(), w, &why)) {
        std::fprintf(stderr, "writeSave failed: %s\n", why.c_str());
        return false;
    }
    fs::copy_file(savePath(scratch.string()), out, fs::copy_options::overwrite_existing, ec);
    if (ec) {
        std::fprintf(stderr, "copy to %s failed: %s\n", out.string().c_str(), ec.message().c_str());
        return false;
    }
    std::printf("%s  tick %lld  day %d  sleep %d  after %d  fingerprint %016llx\n", out.filename().string().c_str(),
                static_cast<long long>(w.tick), w.day, static_cast<int>(w.sleep), w.after ? 1 : 0,
                static_cast<unsigned long long>(fingerprint(w)));
    return true;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc != 2) {
        std::fprintf(stderr, "usage: make_save_fixtures <tests/fixtures directory>\n");
        return 2;
    }
    const fs::path dir = argv[1];
    const fs::path scratch = fs::temp_directory_path() / "atman_make_save_fixtures";
    const int64_t minute = 60 * kTickRate;
    const int64_t limit = 3LL * 24 * 60 * minute;

    World w = newWorld(1);
    Autopilot pilot;
    while (w.sleep != Sleep::Dream && w.tick < limit) tick(w, pilot.next(w));
    if (w.sleep != Sleep::Dream) return std::fprintf(stderr, "the autopilot never dreamed\n"), 1;
    const int64_t dreamt = w.tick;
    while (w.tick < dreamt + kDreamSeconds * kTickRate / 2) tick(w, pilot.next(w));
    if (w.sleep != Sleep::Dream || !save(w, dir / "v2-mid-dream.save", scratch)) return 1;

    while (!w.codex[static_cast<int>(CodexId::Fourth)] && w.tick < limit) tick(w, pilot.next(w));
    if (!w.codex[static_cast<int>(CodexId::Fourth)]) return std::fprintf(stderr, "the fourth never opened\n"), 1;
    const int64_t opened = w.tick;
    while (w.tick < opened + 10 * minute) tick(w, pilot.next(w));
    if (!w.after || w.sleep != Sleep::Awake || !save(w, dir / "v2-after-the-fourth.save", scratch)) return 1;

    std::error_code ec;
    fs::remove_all(scratch, ec);
    return 0;
}
