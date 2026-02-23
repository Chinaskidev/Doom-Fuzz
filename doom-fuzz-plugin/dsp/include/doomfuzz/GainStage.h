#pragma once

#include "Biquad.h"

namespace doomfuzz {

class GainStage {
public:
    void prepare(float sampleRate);
    void reset();
    float process(float x, float gain, float bass);

private:
    Biquad lp250_;
};

} // namespace doomfuzz
