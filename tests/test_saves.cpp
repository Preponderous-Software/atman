// Saves: a save round-trips the World exactly; any one byte flipped, or the
// file cut short, is refused; writing keeps the save it replaces, and a
// damaged current save gives way to the previous one; and a playthrough
// saved and loaded halfway ends on the same pinned fingerprint.
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "check.h"
#include "save/save.h"
#include "sim/autopilot.h"
#include "sim/world.h"

using namespace atman;
namespace fs = std::filesystem;

namespace {

// The autopilot's world after `ticks`.
World played(int64_t ticks) {
    World w = newWorld(1);
    Autopilot pilot;
    for (int64_t t = 0; t < ticks; ++t) tick(w, pilot.next(w));
    return w;
}

bool roundTrips(const World& w) {
    World back;
    std::string why;
    if (!decodeSave(encodeSave(w), back, &why)) {
        std::printf("refused: %s\n", why.c_str());
        return false;
    }
    return fingerprint(back) == fingerprint(w);
}

std::vector<uint8_t> readAll(const std::string& path) {
    std::ifstream f(path, std::ios::binary);
    return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

void writeAll(const std::string& path, const std::vector<uint8_t>& bytes) {
    std::ofstream f(path, std::ios::binary | std::ios::trunc);
    f.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
}

}  // namespace

int main(int argc, char** argv) {
    const fs::path dir = fs::path(argc > 1 ? argv[1] : ".") / "test_saves_slot";
    std::error_code ec;
    fs::remove_all(dir, ec);
    const std::string slot = dir.string();

    // Round trips: a new game, mid-afternoon at the wheel with a pot waiting
    // for its name, at dusk among the snakes, and in the after-mode.
    const int64_t minute = 60 * kTickRate;
    CHECK(roundTrips(newWorld(1)));
    for (int64_t t : {35 * minute + 7, 41 * minute, 110 * minute, 200 * minute}) CHECK(roundTrips(played(t)));
    {
        World naming = newWorld(1);
        Autopilot pilot;
        while (naming.naming < 0 && naming.tick < 120 * minute) tick(naming, pilot.next(naming));
        CHECK(naming.naming >= 0);
        CHECK(roundTrips(naming));
    }

    // One byte flipped anywhere, or the file cut short, is refused.
    {
        const std::vector<uint8_t> good = encodeSave(played(110 * minute));
        int refused = 0;
        for (size_t i = 0; i < good.size(); ++i) {
            std::vector<uint8_t> bad = good;
            bad[i] ^= 0x10;
            World w;
            if (!decodeSave(bad, w, nullptr)) ++refused;
        }
        CHECK(refused == static_cast<int>(good.size()));
        World w;
        std::vector<uint8_t> cut(good.begin(), good.end() - 1);
        CHECK(!decodeSave(cut, w, nullptr));
        std::string why;
        std::vector<uint8_t> other = good;
        other[8] = 99;  // a version this game does not know
        CHECK(!decodeSave(other, w, &why));
        CHECK(why.find("different version") != std::string::npos);
    }

    // A checksummed save of an impossible world is still refused.
    {
        World odd = newWorld(1);
        odd.naming = 3;  // no pot was made
        World w;
        std::string why;
        CHECK(!decodeSave(encodeSave(odd), w, &why));
        CHECK(!why.empty());
    }

    // Writing keeps the previous save; a damaged current save gives way to it.
    {
        CHECK(!hasSave(slot));
        World none;
        CHECK(!loadSave(slot, none).ok);
        const World first = played(30 * minute), second = played(60 * minute);
        std::string why;
        CHECK(writeSave(slot, first, &why));
        CHECK(writeSave(slot, second, &why));
        CHECK(hasSave(slot));
        World loaded;
        LoadResult r = loadSave(slot, loaded);
        CHECK(r.ok && !r.fromPrevious);
        CHECK(fingerprint(loaded) == fingerprint(second));
        World kept;
        CHECK(decodeSave(readAll(previousSavePath(slot)), kept, nullptr));
        CHECK(fingerprint(kept) == fingerprint(first));
        std::vector<uint8_t> bytes = readAll(savePath(slot));
        bytes[bytes.size() / 2] ^= 0x01;
        writeAll(savePath(slot), bytes);
        r = loadSave(slot, loaded);
        CHECK(r.ok && r.fromPrevious);
        CHECK(fingerprint(loaded) == fingerprint(first));
        CHECK(r.message.find("refused") != std::string::npos);
        // Saving over a damaged save keeps the good previous one.
        const World third = played(90 * minute);
        CHECK(writeSave(slot, third, &why));
        r = loadSave(slot, loaded);
        CHECK(r.ok && fingerprint(loaded) == fingerprint(third));
        World prev;
        CHECK(decodeSave(readAll(previousSavePath(slot)), prev, nullptr));
        CHECK(fingerprint(prev) == fingerprint(first));
    }

    // Saved and loaded halfway, the playthrough ends where it always does.
    {
        World w = newWorld(1);
        Autopilot pilot;
        const int64_t limit = 3LL * 24 * 60 * 60 * kTickRate;
        bool reloaded = false;
        for (int64_t t = 0; t < limit && !playthroughDone(w); ++t) {
            tick(w, pilot.next(w));
            if (!reloaded && w.after) {
                std::string why;
                CHECK(writeSave(slot, w, &why));
                World back;
                CHECK(loadSave(slot, back).ok);
                w = back;
                pilot = Autopilot{};  // a new session's autopilot knows only the World
                reloaded = true;
            }
        }
        CHECK(reloaded);
        CHECK(fingerprint(w) == kPlaythroughFingerprint);
    }

    fs::remove_all(dir, ec);
    TEST_MAIN_END();
}
