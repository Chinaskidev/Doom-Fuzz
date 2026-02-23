#pragma once

#include "DelayLine.h"

namespace doomfuzz {

class StereoWidth {
public:
    void prepare(float sampleRate);
    void reset();
    void process(float monoIn, float& outL, float& outR);

private:
    DelayLine delay_;
    int delaySamples_ = 0;
};

} // namespace doomfuzz
