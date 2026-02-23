#pragma once

namespace doomfuzz {

class VoltageSag {
public:
    void prepare(float sampleRate);
    void reset();
    float process(float x, float sag);

private:
    float sampleRate_ = 44100.0f;
    float compEnv_ = 0.0f;
};

} // namespace doomfuzz
