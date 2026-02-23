#include "doomfuzz/VoltageSag.h"
#include <cmath>

namespace doomfuzz {

void VoltageSag::prepare(float sampleRate) {
    sampleRate_ = sampleRate;
}

void VoltageSag::reset() {
    compEnv_ = 0.0f;
}

float VoltageSag::process(float x, float sag) {
    const float level = std::fabs(x);
    const float attCoeff = 0.001f * sampleRate_;
    const float relCoeff = (0.08f + sag * 0.3f) * sampleRate_;

    if (level > compEnv_) {
        compEnv_ += (level - compEnv_) / (attCoeff + 1.0f);
    } else {
        compEnv_ += (level - compEnv_) / (relCoeff + 1.0f);
    }

    const float compThresh = 0.316f;  // ~-10dB
    if (compEnv_ > compThresh) {
        const float ratio = 2.0f + sag * 6.0f;
        const float dB = 20.0f * std::log10(compEnv_ / compThresh);
        const float reduction = dB * (1.0f - 1.0f / ratio);
        x *= std::pow(10.0f, -reduction / 20.0f);
    }

    return x;
}

} // namespace doomfuzz
