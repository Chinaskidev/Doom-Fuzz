#include "doomfuzz/NoiseGate.h"
#include <cmath>
#include <algorithm>

namespace doomfuzz {

void NoiseGate::reset() {
    env_.reset();
    env_.setCoeffs(0.01f, 0.0003f);  // attack ~2ms, release ~700ms
}

float NoiseGate::process(float x, float thresholdDb) {
    const float env = env_.process(x);
    const float thresh = std::pow(10.0f, thresholdDb / 20.0f);
    const float gateMul = (env > thresh)
        ? 1.0f
        : env / std::max(thresh, 1e-10f);
    return x * gateMul;
}

} // namespace doomfuzz
