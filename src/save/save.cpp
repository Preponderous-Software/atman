#include "save/save.h"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <type_traits>

#include "save/sha256.h"

namespace fs = std::filesystem;

namespace atman {

namespace {

constexpr char kMagic[8] = {'A', 'T', 'M', 'A', 'N', 'S', 'A', 'V'};

// Writes little-endian.
struct Writer {
    std::vector<uint8_t> out;
    void raw(uint64_t v, int bytes) {
        for (int i = 0; i < bytes; ++i) out.push_back(static_cast<uint8_t>(v >> (8 * i)));
    }
    template <class T>
    void value(const T& v) {
        if constexpr (std::is_same_v<T, bool>) {
            raw(v ? 1 : 0, 1);
        } else if constexpr (std::is_same_v<T, float>) {
            uint32_t bits;
            std::memcpy(&bits, &v, 4);
            raw(bits, 4);
        } else if constexpr (std::is_enum_v<T>) {
            raw(static_cast<uint64_t>(static_cast<int64_t>(v)), 4);
        } else {
            raw(static_cast<uint64_t>(v), static_cast<int>(sizeof(T)));
        }
    }
};

// Reads little-endian, refusing to read past the end.
struct Reader {
    const uint8_t* p;
    size_t n, at = 0;
    bool ok = true;
    uint64_t raw(int bytes) {
        if (!ok || n - at < static_cast<size_t>(bytes)) {
            ok = false;
            return 0;
        }
        uint64_t v = 0;
        for (int i = 0; i < bytes; ++i) v |= static_cast<uint64_t>(p[at + i]) << (8 * i);
        at += static_cast<size_t>(bytes);
        return v;
    }
    template <class T>
    void value(T& v) {
        if constexpr (std::is_same_v<T, bool>) {
            const uint64_t b = raw(1);
            if (b > 1) ok = false;
            v = b != 0;
        } else if constexpr (std::is_same_v<T, float>) {
            const auto bits = static_cast<uint32_t>(raw(4));
            std::memcpy(&v, &bits, 4);
        } else if constexpr (std::is_enum_v<T>) {
            v = static_cast<T>(static_cast<int32_t>(raw(4)));
        } else {
            v = static_cast<T>(raw(static_cast<int>(sizeof(T))));
        }
    }
};

// The codex pages a format version holds: version 1 had the first three.
constexpr int kVersion1Codex = 3;

// One description of the save's fields, used both ways. `version` is the
// format being read; writing is always kSaveVersion.
template <class IO>
struct Fields {
    IO& io;
    uint32_t version = kSaveVersion;

    template <class T>
    void v(T& x) { io.value(x); }
    void world(World& w) {
        v(w.seed);
        v(w.tick);
        v(w.player.x);
        v(w.player.z);
        v(w.player.yaw);
        v(w.player.pitch);
        v(w.needs.thirst);
        v(w.needs.hunger);
        v(w.needs.tiredness);
        v(w.standing);
        for (bool& d : w.questDone) v(d);
        v(w.lampLit);
        v(w.fear);
        v(w.fearRetired);
        for (Coil& c : w.coils) {
            v(c.seenThrough);
            v(c.gaze);
        }
        const int pages = version >= 2 ? kCodexCount : kVersion1Codex;
        for (int i = 0; i < pages; ++i) v(w.codex[static_cast<size_t>(i)]);
        for (Pot& p : w.pots) {
            v(p.made);
            v(p.broken);
            v(p.form);
            for (char& ch : p.name) v(ch);
        }
        v(w.potsMade);
        v(w.throwing.active);
        v(w.throwing.form);
        v(w.throwing.seconds);
        v(w.naming);
        v(w.clayLens);
        v(w.carry);
        v(w.day);
        v(w.wokeTick);
        v(w.after);
        if (version < 2) return;  // a version 1 save was made awake, before chapter 3 existed
        v(w.sleep);
        v(w.sleptTick);
    }
};

// What a decoded world must look like before the rules may run on it.
bool plausible(const World& w, std::string* error) {
    auto fail = [error](const char* why) {
        if (error) *error = why;
        return false;
    };
    const float floats[] = {w.player.x, w.player.z, w.player.yaw, w.player.pitch, w.needs.thirst, w.needs.hunger,
                            w.needs.tiredness, w.fear, w.throwing.seconds};
    for (float f : floats)
        if (!std::isfinite(f)) return fail("a number in the save is not finite");
    if (w.tick < 0 || w.day < 0 || w.wokeTick < -1 || w.wokeTick > w.tick) return fail("the clock is impossible");
    if (w.potsMade < 0 || w.potsMade > valley::kBoardSlots) return fail("more pots than the board holds");
    if (w.naming < -1 || w.naming >= w.potsMade) return fail("naming a pot that was never made");
    auto formOk = [](Form f) { return static_cast<int>(f) >= 0 && static_cast<int>(f) < kFormCount; };
    if (!formOk(w.throwing.form)) return fail("an unknown form on the wheel");
    for (int i = 0; i < valley::kBoardSlots; ++i) {
        const Pot& p = w.pots[static_cast<size_t>(i)];
        if (!formOk(p.form)) return fail("a pot of an unknown form");
        if (p.made != (i < w.potsMade)) return fail("the drying board does not match the pots made");
        if (p.name.back() != 0) return fail("a pot's name has no end");
        for (char c : p.name)
            if (c != 0 && (c < 32 || c > 126)) return fail("a pot's name is not plain text");
    }
    if (static_cast<int>(w.carry) < 0 || static_cast<int>(w.carry) > static_cast<int>(Carry::Clay))
        return fail("carrying something unknown");
    if (std::fabs(w.player.x) > valley::kWalkLimit + 1 || std::fabs(w.player.z) > valley::kWalkLimit + 1)
        return fail("the player is outside the valley");
    const int sleep = static_cast<int>(w.sleep);
    if (sleep < 0 || sleep > static_cast<int>(Sleep::Deep)) return fail("an unknown kind of sleep");
    if ((w.sleep == Sleep::Awake) != (w.sleptTick == -1) || w.sleptTick > w.tick) return fail("the sleep's clock is impossible");
    return true;
}

bool readFile(const std::string& path, std::vector<uint8_t>& out) {
    std::ifstream f(path, std::ios::binary);
    if (!f) return false;
    out.assign(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
    return true;
}

}  // namespace

std::vector<uint8_t> encodeSave(const World& w) {
    Writer payload;
    Fields<Writer> fields{payload};
    fields.world(const_cast<World&>(w));  // the Writer only reads

    Writer file;
    file.out.insert(file.out.end(), kMagic, kMagic + 8);
    file.value(kSaveVersion);
    file.value(static_cast<uint64_t>(payload.out.size()));
    file.out.insert(file.out.end(), payload.out.begin(), payload.out.end());
    const std::array<uint8_t, 32> digest = sha256(payload.out.data(), payload.out.size());
    file.out.insert(file.out.end(), digest.begin(), digest.end());
    return file.out;
}

bool decodeSave(const std::vector<uint8_t>& bytes, World& out, std::string* error) {
    auto fail = [error](const std::string& why) {
        if (error) *error = why;
        return false;
    };
    if (bytes.size() < 8 + 4 + 8 + 32 || std::memcmp(bytes.data(), kMagic, 8) != 0) return fail("not an Atman save");
    Reader head{bytes.data(), bytes.size()};
    head.at = 8;
    uint32_t version = 0;
    uint64_t length = 0;
    head.value(version);
    head.value(length);
    // Older formats still load: each later version only added fields.
    if (version < 1 || version > kSaveVersion)
        return fail("saved by a different version of Atman (format " + std::to_string(version) + ")");
    if (length != bytes.size() - 8 - 4 - 8 - 32) return fail("the save is truncated or has extra bytes");
    const uint8_t* payload = bytes.data() + 20;
    const std::array<uint8_t, 32> digest = sha256(payload, static_cast<size_t>(length));
    if (std::memcmp(digest.data(), payload + length, 32) != 0) return fail("the save is damaged (its checksum does not match)");

    Reader r{payload, static_cast<size_t>(length)};
    Fields<Reader> fields{r, version};
    World w;
    fields.world(w);
    if (!r.ok || r.at != r.n) return fail("the save's contents do not parse");
    if (!plausible(w, error)) return false;
    out = std::move(w);
    return true;
}

std::string savePath(const std::string& slot) { return (fs::path(slot) / "atman.save").string(); }
std::string previousSavePath(const std::string& slot) { return (fs::path(slot) / "atman.save.prev").string(); }

bool writeSave(const std::string& slot, const World& w, std::string* error) {
    std::error_code ec;
    fs::create_directories(slot, ec);
    const std::vector<uint8_t> bytes = encodeSave(w);
    const fs::path tmp = fs::path(slot) / "atman.save.tmp";
    {
        std::ofstream f(tmp, std::ios::binary | std::ios::trunc);
        f.write(reinterpret_cast<const char*>(bytes.data()), static_cast<std::streamsize>(bytes.size()));
        f.flush();
        if (!f) {
            if (error) *error = "could not write " + tmp.string();
            return false;
        }
    }
    // Keep what was there as the previous save, then move the new one in. A
    // damaged save never replaces a good previous one: it is discarded instead.
    const fs::path cur = savePath(slot), prev = previousSavePath(slot);
    if (fs::exists(cur, ec)) {
        std::vector<uint8_t> old;
        World check;
        if (readFile(cur.string(), old) && decodeSave(old, check, nullptr)) {
            fs::remove(prev, ec);
            fs::rename(cur, prev, ec);
            if (ec) {
                if (error) *error = "could not keep the previous save: " + ec.message();
                return false;
            }
        } else {
            fs::remove(cur, ec);
        }
    }
    fs::rename(tmp, cur, ec);
    if (ec) {
        if (error) *error = "could not move the new save into place: " + ec.message();
        return false;
    }
    return true;
}

LoadResult loadSave(const std::string& slot, World& out) {
    LoadResult r;
    std::vector<uint8_t> bytes;
    std::string why;
    const bool haveCurrent = readFile(savePath(slot), bytes);
    if (haveCurrent && decodeSave(bytes, out, &why)) {
        r.ok = true;
        r.message = "Loaded your save.";
        return r;
    }
    std::string whyPrev;
    if (readFile(previousSavePath(slot), bytes) && decodeSave(bytes, out, &whyPrev)) {
        r.ok = true;
        r.fromPrevious = true;
        r.message = haveCurrent ? "Your latest save was refused (" + why + "). Loaded the one before it."
                                : "Loaded your previous save.";
        return r;
    }
    r.message = haveCurrent ? "Your save was refused (" + why + ") and there is no good earlier save."
                            : "No save to load.";
    return r;
}

bool hasSave(const std::string& slot) {
    std::error_code ec;
    return fs::exists(savePath(slot), ec) || fs::exists(previousSavePath(slot), ec);
}

std::string defaultSlot() {
#ifdef _WIN32
    char* appData = nullptr;
    size_t len = 0;
    std::string base = ".";
    if (_dupenv_s(&appData, &len, "APPDATA") == 0 && appData) base = appData;
    std::free(appData);
    return (fs::path(base) / "Atman" / "default").string();
#else
    const char* xdg = std::getenv("XDG_DATA_HOME");
    if (xdg && *xdg) return (fs::path(xdg) / "atman" / "default").string();
    const char* home = std::getenv("HOME");
    return (fs::path(home ? home : ".") / ".local" / "share" / "atman" / "default").string();
#endif
}

}  // namespace atman
