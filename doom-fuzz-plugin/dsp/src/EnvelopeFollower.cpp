#include "doomfuzz/EnvelopeFollower.h"
#include <cmath>

namespace doomfuzz {

void EnvelopeFollower::reset() {
    env_ = 0.0f;
}

void EnvelopeFollower::setCoeffs(float attackCoeff, float releaseCoeff) {
    attackCoeff_  = attackCoeff;
    releaseCoeff_ = releaseCoeff;
}

float EnvelopeFollower::process(float x) {
    const float absX = std::fabs(x);
    const float coeff = (absX > env_) ? attackCoeff_ : releaseCoeff_;
    env_ += (absX - env_) * coeff;
    return env_;
}

} // namespace doomfuzz
