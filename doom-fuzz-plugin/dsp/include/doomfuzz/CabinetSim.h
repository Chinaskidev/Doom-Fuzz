#pragma once

#include "Biquad.h"

namespace doomfuzz {

class CabinetSim {
public:
    void prepare(float sampleRate);
    void reset();
    float process(float x);

private:
    Biquad lpCab_;
    Biquad peakEq_;
    OnePoleHP hpCab_;
};

} // namespace doomfuzz
