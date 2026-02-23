#include "doomfuzz/OctaveDown.h"
#include <cmath>

namespace doomfuzz {

void OctaveDown::prepare(float sampleRate) {
    lp120_.setLowpass(120.0f, sampleRate);
}

void OctaveDown::reset() {
    lp120_.reset();
}

float OctaveDown::process(float x, float octaveMix) {
    // Full-wave rectification → LP 120Hz → tanh → mix
    const float sub = std::tanh(lp120_.process(std::fabs(x)) * 2.0f) * octaveMix;
    return x + sub;
}

} // namespace doomfuzz
