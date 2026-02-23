#include "doomfuzz/StereoWidth.h"
#include <algorithm>

namespace doomfuzz {

void StereoWidth::prepare(float sampleRate) {
    // Haas delay: 3ms on right channel
    delaySamples_ = std::min(static_cast<int>(sampleRate * 0.003f), 511);
}

void StereoWidth::reset() {
    delay_.reset();
}

void StereoWidth::process(float monoIn, float& outL, float& outR) {
    delay_.write(monoIn);
    outL = monoIn;
    outR = delay_.read(delaySamples_);
}

} // namespace doomfuzz
