#pragma once

namespace doomfuzz {

class DelayLine {
public:
    void reset();
    void write(float x);
    float read(int delaySamples) const;

private:
    static constexpr int kMaxSize = 512;
    float buffer_[kMaxSize] = {};
    int writeIdx_ = 0;
};

} // namespace doomfuzz
