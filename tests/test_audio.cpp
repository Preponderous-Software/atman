// Sound without a sound card: the cues follow the world (a jar filled, a pot
// broken, a page opened), the steady sounds follow the place and the hour,
// and the synth's output is finite, bounded, audible and the same every run.
#include <cmath>

#include "audio/cues.h"
#include "audio/synth.h"
#include "check.h"
#include "sim/valley.h"

using namespace atman;
using namespace atman::audio;
namespace V = atman::valley;

namespace {

bool has(const std::vector<Cue>& cs, Cue c) {
    for (Cue x : cs)
        if (x == c) return true;
    return false;
}

float peak(const std::vector<float>& s) {
    float p = 0;
    for (float x : s) p = std::fmax(p, std::fabs(x));
    return p;
}

bool finite(const std::vector<float>& s) {
    for (float x : s)
        if (!std::isfinite(x)) return false;
    return true;
}

float rms(const std::vector<float>& s) {
    double sum = 0;
    for (float x : s) sum += static_cast<double>(x) * static_cast<double>(x);
    return static_cast<float>(std::sqrt(sum / static_cast<double>(s.size())));
}

void atMinute(World& w, int minute) {
    w.tick = static_cast<int64_t>((minute - kStartMinute) / kGameMinutesPerSecond) * kTickRate;
}

}  // namespace

int main() {
    // Nothing changes, nothing sounds.
    {
        const World w = newWorld(1);
        CHECK(cues(w, w).empty());
    }

    // Each change brings its cue.
    {
        const World before = newWorld(1);
        World after = before;
        after.carry = Carry::Water;
        CHECK(has(cues(before, after), Cue::FillJar));
        after.carry = Carry::Clay;
        CHECK(has(cues(before, after), Cue::DigClay));

        after = before;
        after.questDone[0] = true;
        CHECK(has(cues(before, after), Cue::ChoreDone));

        after = before;
        after.lampLit = true;
        CHECK(has(cues(before, after), Cue::LampOn));
        CHECK(has(cues(after, before), Cue::LampOff));

        after = before;
        after.potsMade = 1;
        after.pots[0].made = true;
        CHECK(has(cues(before, after), Cue::PotMade));
        World broken = after;
        broken.pots[0].broken = true;
        CHECK(has(cues(after, broken), Cue::PotBroken));

        after = before;
        after.codex[0] = true;
        after.coils[0].seenThrough = true;
        const std::vector<Cue> seen = cues(before, after);
        CHECK(has(seen, Cue::Page));
        CHECK(has(seen, Cue::Rope));

        // Waking puts the lamp out, but that is not a lamp being blown out.
        after = before;
        after.lampLit = true;
        World morning = after;
        morning.day = 1;
        morning.lampLit = false;
        const std::vector<Cue> woke = cues(after, morning);
        CHECK(has(woke, Cue::Wake));
        CHECK(!has(woke, Cue::LampOff));

        // The shrine's bell as the sun goes down, awake, and not in the dream.
        World afternoon = before, dusk = before;
        atMinute(afternoon, kDuskMinute - 1);
        atMinute(dusk, kDuskMinute + 1);
        CHECK(has(cues(afternoon, dusk), Cue::Sunset));
        CHECK(!has(cues(dusk, dusk), Cue::Sunset));
        dusk.sleep = afternoon.sleep = Sleep::Dream;
        CHECK(!has(cues(afternoon, dusk), Cue::Sunset));
    }

    // The steady sounds follow the place and the hour.
    {
        World w = newWorld(1);
        const Ambience village = ambience(w, {-8, 1.6f, 15}, 0);
        const Ambience river = ambience(w, {V::kRiverWest - 1, 1.6f, 0}, 0);
        CHECK(river.river > 0.9f);
        CHECK(village.river < 0.1f);
        CHECK(village.birds > 0);
        CHECK(village.crickets == 0);
        CHECK(village.drone == 0);
        atMinute(w, 21 * 60);
        const Ambience night = ambience(w, {-8, 1.6f, 15}, 0);
        CHECK(night.crickets == 1);
        CHECK(night.birds == 0);
        w.after = true;
        CHECK(ambience(w, {-8, 1.6f, 15}, 0).drone == 1);
        w.player.x = 26;
        w.player.z = 0;
        CHECK(ambience(w, {26, 1.6f, 0}, 1.8f).stone);

        // The dream has the evening's birds, though it is night by the game
        // clock; deep sleep has only the river, wherever the body lies.
        w.after = false;
        w.sleep = Sleep::Dream;
        w.sleptTick = w.tick;
        const Ambience dream = ambience(w, {-8, 1.6f, 15}, 1.0f);
        CHECK(dream.birds > 0 && dream.crickets == 0);
        w.sleep = Sleep::Deep;
        const Ambience deep = ambience(w, {-8, 1.6f, 15}, 1.0f);
        CHECK(deep.river > 0.5f);
        CHECK(deep.wind == 0 && deep.birds == 0 && deep.crickets == 0 && deep.steps == 0 && deep.drone == 0);

        // Captions name what the sounds tell: the river in deep sleep, a
        // heartbeat while afraid, the wheel; and nothing for the ordinary day.
        CHECK(captions(w, deep) == "(the river)");
        w.sleep = Sleep::Awake;
        w.sleptTick = -1;
        Ambience afraid;
        afraid.heart = 0.8f;
        afraid.wheel = 1;
        CHECK(captions(w, afraid) == "(your heart pounding, the wheel turning)");
        CHECK(captions(w, village).empty());
        w.after = true;
        CHECK(captions(w, ambience(w, {-8, 1.6f, 15}, 0)).empty());  // the tanpura is not captioned
    }

    // Every one-off sound is there, finite and neither silent nor clipping.
    {
        const Synth synth;
        for (int c = 0; c < static_cast<int>(Cue::Count); ++c) {
            const std::vector<float>& s = synth.sound(static_cast<Cue>(c));
            CHECK(!s.empty());
            CHECK(finite(s));
            CHECK(peak(s) > 0.05f);
            CHECK(peak(s) < 1.5f);
        }
    }

    // A night in the after-mode by the river, walking, afraid, with every cue
    // played: bounded, audible, and the same twice.
    {
        auto run = [] {
            Synth synth;
            Ambience a;
            a.river = a.wind = a.crickets = a.heart = a.drone = 1;
            a.steps = 2;
            synth.setAmbience(a);
            for (int c = 0; c < static_cast<int>(Cue::Count); ++c) synth.play(static_cast<Cue>(c));
            std::vector<float> out(static_cast<size_t>(kRate) * 3);
            synth.render(out.data(), static_cast<int>(out.size()));
            return out;
        };
        const std::vector<float> a = run(), b = run();
        CHECK(finite(a));
        CHECK(peak(a) <= 1.0f);
        CHECK(rms(a) > 0.01f);
        CHECK(a == b);
    }

    // Silence in, silence out: an empty ambience is quiet.
    {
        Synth synth;
        synth.setAmbience(Ambience{});
        std::vector<float> out(static_cast<size_t>(kRate));
        synth.render(out.data(), static_cast<int>(out.size()));
        CHECK(peak(out) < 0.01f);
    }

    TEST_MAIN_END();
}
