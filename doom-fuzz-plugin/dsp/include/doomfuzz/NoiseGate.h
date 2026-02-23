#pragma once

#include "EnvelopeFollower.h"

namespace doomfuzz {

class NoiseGate {
public:
    void reset();
    float process(float x, float thresholdDb);

private:
    EnvelopeFollower env_;
};

} // namespace doomfuzz
