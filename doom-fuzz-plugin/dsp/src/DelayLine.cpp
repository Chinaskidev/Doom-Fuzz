#include "doomfuzz/DelayLine.h"
#include <cstring>

namespace doomfuzz {

void DelayLine::reset() {
    std::memset(buffer_, 0, sizeof(buffer_));
    writeIdx_ = 0;
}

void DelayLine::write(float x) {
    buffer_[writeIdx_] = x;
    writeIdx_ = (writeIdx_ + 1) % kMaxSize;
}

float DelayLine::read(int delaySamples) const {
    if (delaySamples >= kMaxSize) delaySamples = kMaxSize - 1;
    if (delaySamples < 0) delaySamples = 0;
    const int readIdx = (writeIdx_ - 1 - delaySamples + kMaxSize * 2) % kMaxSize;
    return buffer_[readIdx];
}

} // namespace doomfuzz
