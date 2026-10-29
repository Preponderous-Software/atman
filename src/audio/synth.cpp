#include "audio/synth.h"

#include <cmath>

namespace atman::audio {

namespace {

constexpr float kTwoPi = 6.2831853f;
constexpr float kDt = 1.0f / static_cast<float>(kRate);
constexpr float kGlide = 1.0f / (0.5f * static_cast<float>(kRate));  // the steady sounds' glide, per sample

// A small deterministic noise source, so the one-off sounds render the same
// every time.
struct Rng {
    uint32_t s;
    float next() {  // -1..1
        s ^= s << 13;
        s ^= s >> 17;
        s ^= s << 5;
        return static_cast<float>(s >> 8) / 8388608.0f - 1.0f;
    }
    float between(float a, float b) { return a + (b - a) * (next() * 0.5f + 0.5f); }
};

// One-pole low-pass filter coefficient for a cutoff in Hz.
float pole(float hz) { return 1.0f - std::exp(-kTwoPi * hz * kDt); }

int samples(float seconds) { return static_cast<int>(seconds * static_cast<float>(kRate)); }
float at(int i) { return static_cast<float>(i) * kDt; }

// 0 at the edges of [start, start + length], 1 in the middle.
float window(float t, float start, float length) {
    const float u = (t - start) / length;
    if (u <= 0 || u >= 1) return 0;
    const float s = std::sin(u * 3.14159265f);
    return s * s;
}

float decay(float t, float start, float tau) { return t < start ? 0.0f : std::exp(-(t - start) / tau); }

// A struck bell: inharmonic partials, the higher ones dying first.
std::vector<float> bell(float hz, float seconds) {
    static const float ratio[] = {1.0f, 2.0f, 2.76f, 5.4f, 8.9f};
    static const float amp[] = {1.0f, 0.6f, 0.4f, 0.25f, 0.1f};
    static const float tau[] = {1.6f, 1.1f, 0.8f, 0.4f, 0.2f};
    std::vector<float> out(static_cast<size_t>(samples(seconds)));
    for (size_t i = 0; i < out.size(); ++i) {
        const float t = at(static_cast<int>(i));
        const float attack = std::fmin(t / 0.004f, 1.0f);
        float s = 0;
        for (int k = 0; k < 5; ++k) s += amp[k] * std::exp(-t / tau[k]) * std::sin(kTwoPi * hz * ratio[k] * t);
        out[i] = 0.3f * attack * s;
    }
    return out;
}

// Soft sine tones starting `apart` seconds apart.
std::vector<float> chime(std::initializer_list<float> notes, float apart, float tau, float seconds) {
    std::vector<float> out(static_cast<size_t>(samples(seconds)));
    for (size_t i = 0; i < out.size(); ++i) {
        const float t = at(static_cast<int>(i));
        float s = 0, start = 0;
        for (float hz : notes) {
            if (t >= start) {
                const float u = t - start;
                s += std::fmin(u / 0.005f, 1.0f) * std::exp(-u / tau) *
                     (std::sin(kTwoPi * hz * u) + 0.3f * std::sin(kTwoPi * 2 * hz * u));
            }
            start += apart;
        }
        out[i] = 0.25f * s;
    }
    return out;
}

std::vector<float> fillJar(Rng& r) {
    std::vector<float> out(static_cast<size_t>(samples(0.9f)));
    float lo = 0, hi = 0;
    float blipAt[6], blipHz[6];
    for (int k = 0; k < 6; ++k) {
        blipAt[k] = r.between(0.08f, 0.7f);
        blipHz[k] = r.between(350, 800);
    }
    for (size_t i = 0; i < out.size(); ++i) {
        const float t = at(static_cast<int>(i)), n = r.next();
        lo += pole(500) * (n - lo);
        hi += pole(2800) * (n - hi);
        float s = (hi - lo) * 2.5f * decay(t, 0, 0.16f);
        for (int k = 0; k < 6; ++k) {
            const float u = t - blipAt[k];
            if (u > 0 && u < 0.05f) s += 0.25f * window(u, 0, 0.05f) * std::sin(kTwoPi * blipHz[k] * (1 + 8 * u) * u);
        }
        out[i] = s;
    }
    return out;
}

std::vector<float> digClay(Rng& r) {
    std::vector<float> out(static_cast<size_t>(samples(0.8f)));
    float lo = 0, thud = 0;
    for (size_t i = 0; i < out.size(); ++i) {
        const float t = at(static_cast<int>(i)), n = r.next();
        lo += pole(900) * (n - lo);
        thud += pole(140) * (n - thud);
        float env = 0;
        for (float start : {0.0f, 0.25f, 0.5f}) env += window(t, start, 0.13f);
        out[i] = env * (1.6f * lo + 4.0f * thud);
    }
    return out;
}

std::vector<float> drink() {
    std::vector<float> out(static_cast<size_t>(samples(1.1f)));
    for (size_t i = 0; i < out.size(); ++i) {
        const float t = at(static_cast<int>(i));
        float s = 0;
        for (float start : {0.0f, 0.35f, 0.7f}) {
            const float u = t - start;
            if (u > 0 && u < 0.11f) s += window(u, 0, 0.11f) * std::sin(kTwoPi * (220 - 600 * u) * u);
        }
        out[i] = 0.5f * s;
    }
    return out;
}

std::vector<float> lampOn(Rng& r) {
    std::vector<float> out(static_cast<size_t>(samples(0.6f)));
    float lo = 0, sweep = 0;
    for (size_t i = 0; i < out.size(); ++i) {
        const float t = at(static_cast<int>(i)), n = r.next();
        lo += pole(1500) * (n - lo);
        sweep += pole(300 + 1500 * t) * (n - sweep);
        out[i] = 0.6f * (n - lo) * decay(t, 0, 0.012f) + 1.2f * sweep * window(t, 0.02f, 0.5f);
    }
    return out;
}

std::vector<float> lampOff(Rng& r) {
    std::vector<float> out(static_cast<size_t>(samples(0.3f)));
    float lo = 0;
    for (size_t i = 0; i < out.size(); ++i) {
        const float t = at(static_cast<int>(i));
        lo += pole(500) * (r.next() - lo);
        out[i] = 2.0f * lo * window(t, 0, 0.22f);
    }
    return out;
}

std::vector<float> potBroken(Rng& r) {
    std::vector<float> out(static_cast<size_t>(samples(0.9f)));
    float thud = 0, lo = 0;
    float grainAt[30];
    for (float& g : grainAt) g = r.between(0.0f, 0.35f);
    for (size_t i = 0; i < out.size(); ++i) {
        const float t = at(static_cast<int>(i)), n = r.next();
        thud += pole(250) * (n - thud);
        lo += pole(2500) * (n - lo);
        float grains = 0;
        for (float g : grainAt)
            if (t > g && t < g + 0.008f) grains += (1 - t * 2) * decay(t, g, 0.002f);
        out[i] = 3.0f * thud * decay(t, 0, 0.06f) + 0.5f * (n - lo) * grains;
    }
    return out;
}

// The tanpura's four strings, plucked in turn: Pa, Sa, Sa, and Sa an octave down.
std::vector<float> pluck(float hz) {
    constexpr int kPartials = 10;
    std::vector<float> out(static_cast<size_t>(samples(5.0f)));
    // Each partial (and a copy 0.4 Hz sharp, for the shimmer) turns as a
    // rotating phasor rather than a sine per sample.
    double re[2 * kPartials], im[2 * kPartials], cr[2 * kPartials], ci[2 * kPartials];
    for (int n = 0; n < 2 * kPartials; ++n) {
        const double f = static_cast<double>(hz) * (n % kPartials + 1) + (n >= kPartials ? 0.4 : 0.0);
        const double step = 6.283185307179586 * f / kRate;
        re[n] = 1;
        im[n] = 0;
        cr[n] = std::cos(step);
        ci[n] = std::sin(step);
    }
    float swell[kPartials], fade[kPartials], fadeStep[kPartials];
    for (int n = 0; n < kPartials; ++n) {
        const float k = static_cast<float>(n + 1);
        fade[n] = 1.0f / k;
        fadeStep[n] = std::exp(-kDt * (1.0f + 0.12f * k) / 4.0f);
        swell[n] = 1;
    }
    for (size_t i = 0; i < out.size(); ++i) {
        if (i % 64 == 0) {
            // The jawari bridge keeps the upper partials alive, swelling and fading.
            const float t = at(static_cast<int>(i));
            for (int n = 0; n < kPartials; ++n) {
                const float k = static_cast<float>(n + 1);
                swell[n] = 1.0f + 0.6f * std::sin(kTwoPi * (0.35f + 0.05f * k) * t + k);
            }
        }
        float s = 0;
        for (int n = 0; n < 2 * kPartials; ++n) {
            const double r = re[n] * cr[n] - im[n] * ci[n];
            im[n] = re[n] * ci[n] + im[n] * cr[n];
            re[n] = r;
            const int p = n % kPartials;
            s += swell[p] * fade[p] * static_cast<float>(im[n]) * (n >= kPartials ? 0.5f : 1.0f);
        }
        for (int n = 0; n < kPartials; ++n) fade[n] *= fadeStep[n];
        out[i] = 0.12f * std::fmin(at(static_cast<int>(i)) / 0.01f, 1.0f) * s;
    }
    return out;
}

std::vector<float> chirps(Rng& r) {
    const int count = static_cast<int>(r.between(2, 5.99f));
    std::vector<float> out(static_cast<size_t>(samples(0.9f)));
    float start = 0;
    for (int c = 0; c < count; ++c) {
        const float len = r.between(0.05f, 0.11f), f0 = r.between(2800, 4200), bend = r.between(-0.35f, 0.25f);
        for (int i = samples(start); i < samples(start + len) && i < static_cast<int>(out.size()); ++i) {
            const float u = at(i) - start;
            const float hz = f0 * (1 + bend * u / len);
            out[static_cast<size_t>(i)] += window(u, 0, len) * (std::sin(kTwoPi * hz * u) + 0.2f * std::sin(kTwoPi * 2 * hz * u));
        }
        start += len + r.between(0.04f, 0.09f);
    }
    for (float& s : out) s *= 0.3f;
    return out;
}

// The koel's rising call, ku-oo, ku-oo, each higher than the last, from a tree away.
std::vector<float> koel(Rng& r) {
    std::vector<float> out(static_cast<size_t>(samples(2.4f)));
    float hz = r.between(650, 760), lo = 0;
    for (int note = 0; note < 3; ++note) {
        const float start = 0.75f * static_cast<float>(note);
        for (int i = samples(start); i < samples(start + 0.45f); ++i) {
            const float u = at(i) - start;
            const float f = hz * (1 + 0.45f * std::fmin(u / 0.25f, 1.0f));
            out[static_cast<size_t>(i)] += window(u, 0, 0.45f) * std::sin(kTwoPi * f * u);
        }
        hz *= 1.12f;
    }
    for (float& s : out) {
        lo += pole(2000) * (s - lo);
        s = 0.35f * lo;
    }
    return out;
}

std::vector<float> step(Rng& r, bool stone) {
    std::vector<float> out(static_cast<size_t>(samples(0.14f)));
    float lo = 0;
    const float cutoff = stone ? 2000.0f : 320.0f, tau = stone ? 0.012f : 0.03f, gain = stone ? 0.9f : 3.5f;
    for (size_t i = 0; i < out.size(); ++i) {
        const float t = at(static_cast<int>(i)), n = r.next();
        lo += pole(cutoff) * (n - lo);
        out[i] = gain * lo * std::fmin(t / 0.003f, 1.0f) * std::exp(-t / tau);
    }
    return out;
}

std::vector<float> heartbeat() {
    std::vector<float> out(static_cast<size_t>(samples(0.4f)));
    for (size_t i = 0; i < out.size(); ++i) {
        const float t = at(static_cast<int>(i));
        out[i] = 0.8f * window(t, 0, 0.09f) * std::sin(kTwoPi * 55 * t) +
                 0.55f * window(t, 0.18f, 0.08f) * std::sin(kTwoPi * 70 * t);
    }
    return out;
}

// A pure tone at time `t`, in double: a float t of minutes would smear the
// phase of anything above a few hundred hertz.
float tone(double hz, double t) { return static_cast<float>(std::sin(6.283185307179586 * hz * t)); }

// Gentle saturation, so loud moments round off instead of clipping.
float soften(float x) {
    if (x > 3) return 1;
    if (x < -3) return -1;
    return x * (27 + x * x) / (27 + 9 * x * x);
}

}  // namespace

Synth::Synth() {
    Rng r{0x9E3779B9u};
    auto set = [&](Cue c, std::vector<float> s) { cues_[static_cast<size_t>(c)] = std::move(s); };
    set(Cue::FillJar, fillJar(r));
    set(Cue::DigClay, digClay(r));
    set(Cue::ChoreDone, chime({660, 990}, 0.12f, 0.45f, 1.6f));
    set(Cue::Drink, drink());
    set(Cue::Eat, chime({1240, 1240}, 0.35f, 0.15f, 0.9f));
    set(Cue::Wake, bell(330, 4));
    set(Cue::LampOn, lampOn(r));
    set(Cue::LampOff, lampOff(r));
    set(Cue::PotMade, chime({784, 1176}, 0.1f, 0.5f, 1.5f));
    set(Cue::PotBroken, potBroken(r));
    set(Cue::Lens, chime({523, 587, 659, 784, 880}, 0.1f, 0.35f, 1.8f));
    set(Cue::Page, bell(523, 4));
    set(Cue::Rope, bell(392, 4));
    set(Cue::Sunset, bell(262, 6));  // low and long, from the shrine
    for (int i = 0; i < 8; ++i) chirps_.push_back(chirps(r));
    for (int i = 0; i < 3; ++i) koel_.push_back(koel(r));
    for (int i = 0; i < 4; ++i) {
        soil_.push_back(step(r, false));
        stone_.push_back(step(r, true));
    }
    const float sa = 138.59f;  // C#3
    plucks_ = {pluck(sa * 0.75f), pluck(sa), pluck(sa), pluck(sa / 2)};
    heart_ = {heartbeat()};
}

void Synth::play(Cue c) {
    // The bells (a page, a rope, waking, the evening) are the quietest: they are not rewards.
    const float gain = c == Cue::Page || c == Cue::Rope || c == Cue::Wake || c == Cue::Sunset ? 0.6f : 0.8f;
    playing_.push_back({&cues_[static_cast<size_t>(c)], 0, gain});
}

float Synth::noise() {
    seed_ ^= seed_ << 13;
    seed_ ^= seed_ >> 17;
    seed_ ^= seed_ << 5;
    return static_cast<float>(seed_ >> 8) / 8388608.0f - 1.0f;
}

void Synth::every(float& clock, float perSecond, const std::vector<std::vector<float>>& sounds, float gain) {
    if (perSecond <= 0) return;
    clock += perSecond * kDt;
    if (clock < 1) return;
    clock = -0.6f * (noise() * 0.5f + 0.5f);  // not like clockwork
    const size_t pick = static_cast<size_t>((noise() * 0.5f + 0.5f) * static_cast<float>(sounds.size())) % sounds.size();
    playing_.push_back({&sounds[pick], 0, gain});
}

float Synth::next() {
    auto glide = [](float& v, float to) { v += (to - v) * kGlide; };
    glide(now_.river, target_.river);
    glide(now_.wind, target_.wind);
    glide(now_.birds, target_.birds);
    glide(now_.crickets, target_.crickets);
    glide(now_.wheel, target_.wheel);
    glide(now_.heart, target_.heart);
    now_.drone += (target_.drone - now_.drone) * kGlide * 0.1f;  // the drone comes in over seconds
    time_ += static_cast<double>(kDt);
    const double td = std::fmod(time_, 1000.0);  // for tones
    const float t = static_cast<float>(td);        // for slow swells and rhythms
    const float n = noise();
    float s = 0;

    // The river: a low rush and a brighter hiss, slowly breathing.
    riverLow_ += pole(350) * (n - riverLow_);
    riverHigh_ += pole(3000) * (n - riverHigh_);
    riverHigh2_ += pole(900) * (riverHigh_ - riverHigh2_);
    const float breath = 0.8f + 0.15f * std::sin(kTwoPi * 0.13f * t) + 0.08f * std::sin(kTwoPi * 0.37f * t);
    s += now_.river * breath * (1.0f * riverLow_ + 0.85f * (riverHigh_ - riverHigh2_));

    // The wind: low noise whose colour drifts.
    windLow_ += pole(160 + 120 * std::sin(kTwoPi * 0.05f * t)) * (n - windLow_);
    s += now_.wind * 0.9f * windLow_ * (0.7f + 0.3f * std::sin(kTwoPi * 0.07f * t + 1));

    // Crickets: two, each a train of quick pulses on a high carrier.
    if (now_.crickets > 0.001f) {
        auto cricket = [&](double hz, float period, float offset) {
            const float u = std::fmod(t + offset, period);
            float gate = 0;
            for (int k = 0; k < 3; ++k) gate += window(u, 0.03f * static_cast<float>(k), 0.018f);
            return gate * tone(hz, td);
        };
        s += now_.crickets * 0.15f * (cricket(4700, 0.55f, 0) + 0.7f * cricket(5150, 0.71f, 0.2f));
    }

    // The wheel: a low wobbling hum and the slap of wet clay under the hands.
    if (now_.wheel > 0.001f) {
        wheelSlap_ += pole(1200) * (n - wheelSlap_);
        const float wobble = 1 + 0.3f * std::sin(kTwoPi * 1.3f * t);
        float slap = 0.5f + 0.5f * std::sin(kTwoPi * 2.6f * t);
        slap *= slap * slap * slap;
        s += now_.wheel * (0.08f * wobble * (tone(82, td) + 0.4f * tone(164, td)) + 0.5f * slap * wheelSlap_);
    }

    // Birds by day: chirps often, the koel now and then.
    every(birdClock_, now_.birds * 0.6f, chirps_, 0.7f * now_.birds);
    every(koelClock_, now_.birds * 0.035f, koel_, 0.7f * now_.birds);

    // Footsteps, as fast as the body walks.
    if (target_.steps > 0) {
        stepClock_ += target_.steps * kDt;
        if (stepClock_ >= 1) {
            stepClock_ -= 1;
            const auto& set = target_.stone ? stone_ : soil_;
            playing_.push_back({&set[static_cast<size_t>((noise() * 0.5f + 0.5f) * 3.99f)], 0, 0.6f});
        }
    } else {
        stepClock_ = 0.7f;  // the first step comes soon after setting off
    }

    // Fear: a heartbeat, quicker as it grows.
    if (now_.heart > 0.02f) {
        heartClock_ += (55 + 75 * now_.heart) / 60 * kDt;
        if (heartClock_ >= 1) {
            heartClock_ -= 1;
            playing_.push_back({&heart_[0], 0, 0.6f * now_.heart});
        }
    }

    // The after-mode: a tanpura's drone, one string after another.
    if (now_.drone > 0.01f) {
        pluckClock_ += kDt / 1.2f;
        if (pluckClock_ >= 1) {
            pluckClock_ -= 1;
            playing_.push_back({&plucks_[static_cast<size_t>(pluck_++ % 4)], 0, 0.5f * now_.drone});
        }
    }

    // Everything playing once.
    for (size_t i = 0; i < playing_.size();) {
        Playing& p = playing_[i];
        s += p.gain * (*p.samples)[p.at++];
        if (p.at >= p.samples->size()) {
            playing_[i] = playing_.back();
            playing_.pop_back();
        } else {
            ++i;
        }
    }
    return soften(0.7f * s);
}

void Synth::render(float* out, int frames) {
    for (int i = 0; i < frames; ++i) out[i] = next();
}

}  // namespace atman::audio
