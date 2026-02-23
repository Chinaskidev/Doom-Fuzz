#pragma once

#include "Biquad.h"

namespace doomfuzz {

class ToneStack {
public:
    void prepare(float sampleRate, float tone);
    void reset();
    float process(float x, float tone);

private:
    Biquad lpTone_;
    OnePoleHP hpTone_;
    float sampleRate_ = 44100.0f;
    float lastTone_ = -1.0f;
};

} // namespace doomfuzz
