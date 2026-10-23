// The valley's sounds, made from nothing: every sound is synthesized (no
// recordings ship with the game). The steady sounds are generated sample by
// sample from the Ambience; the one-off sounds are rendered once, when the
// synth is made, and mixed in when they are played. Mono, at kRate.
//
// Not thread-safe on its own: the device (device.h) locks around it.
#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "audio/cues.h"

namespace atman::audio {

constexpr int kRate = 48000;

class Synth {
public:
    Synth();

    // Where the steady sounds are heading; they glide there over about half a second.
    void setAmbience(const Ambience& a) { target_ = a; }
    void play(Cue c);
    // Writes `frames` samples, each in -1..1.
    void render(float* out, int frames);

    // The rendered one-off sound for a cue (for the tests).
    const std::vector<float>& sound(Cue c) const { return cues_[static_cast<size_t>(c)]; }

private:
    struct Playing {
        const std::vector<float>* samples;
        size_t at;
        float gain;
    };
    float noise();
    float next();
    void every(float& clock, float perSecond, const std::vector<std::vector<float>>& sounds, float gain);

    uint32_t seed_ = 0x2545F491u;
    Ambience target_, now_;
    std::array<std::vector<float>, static_cast<size_t>(Cue::Count)> cues_;
    std::vector<std::vector<float>> chirps_, koel_, soil_, stone_, plucks_, heart_;
    std::vector<Playing> playing_;
    // The steady sounds' state.
    float riverLow_ = 0, riverHigh_ = 0, riverHigh2_ = 0, windLow_ = 0, wheelSlap_ = 0;
    double time_ = 0;
    float birdClock_ = 0, koelClock_ = 0, stepClock_ = 0, pluckClock_ = 0, heartClock_ = 0;
    int pluck_ = 0;
};

}  // namespace atman::audio
