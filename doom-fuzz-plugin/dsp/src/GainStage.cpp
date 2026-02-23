#include "doomfuzz/GainStage.h"
#include <cmath>
#include <algorithm>

namespace doomfuzz {

void GainStage::prepare(float sampleRate) {
    lp250_.setLowpass(250.0f, sampleRate);
}

void GainStage::reset() {
    lp250_.reset();
}

float GainStage::process(float x, float gain, float bass) {
    // Bass boost: parallel LP mix
    const float lowPart = lp250_.process(x) * bass * 3.0f;
    x = x + lowPart;

    // Stage 1: soft saturation
    x *= 1.0f + gain * 40.0f;
    x = std::tanh(x);

    // Stage 2: push harder
    x *= 3.0f;
    x = std::tanh(x);

    // Stage 3: asymmetric hard clip (broken speaker character)
    x *= 2.5f;
    x = std::min(0.8f, std::max(-1.0f, x));

    return x;
}

} // namespace doomfuzz
