#pragma once

#include "Biquad.h"

namespace doomfuzz {

class OctaveDown {
public:
    void prepare(float sampleRate);
    void reset();
    float process(float x, float octaveMix);

private:
    Biquad lp120_;
};

} // namespace doomfuzz
