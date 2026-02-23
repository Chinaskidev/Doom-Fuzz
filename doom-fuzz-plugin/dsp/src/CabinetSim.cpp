#include "doomfuzz/CabinetSim.h"

namespace doomfuzz {

void CabinetSim::prepare(float sampleRate) {
    lpCab_.setLowpass(4500.0f, sampleRate);
    peakEq_.setPeakEQ(1500.0f, sampleRate, 3.0f, 1.5f);
    hpCab_.setFreq(60.0f, sampleRate);
}

void CabinetSim::reset() {
    lpCab_.reset();
    peakEq_.reset();
    hpCab_.reset();
}

float CabinetSim::process(float x) {
    x = lpCab_.process(x);
    x = peakEq_.process(x);
    x = hpCab_.process(x);
    return x;
}

} // namespace doomfuzz
