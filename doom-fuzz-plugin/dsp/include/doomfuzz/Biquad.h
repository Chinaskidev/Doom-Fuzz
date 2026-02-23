#pragma once

namespace doomfuzz {

// Second-order biquad filter (LP, HP, PeakEQ)
class Biquad {
public:
    void reset();
    void setLowpass(float freq, float sampleRate, float Q = 0.707f);
    void setHighpass(float freq, float sampleRate, float Q = 0.707f);
    void setPeakEQ(float freq, float sampleRate, float gainDb, float bw);
    float process(float x);

private:
    float b0_ = 0.0f, b1_ = 0.0f, b2_ = 0.0f;
    float a1_ = 0.0f, a2_ = 0.0f;
    float x1_ = 0.0f, x2_ = 0.0f;
    float y1_ = 0.0f, y2_ = 0.0f;
};

// First-order highpass filter
class OnePoleHP {
public:
    void reset();
    void setFreq(float freq, float sampleRate);
    float process(float x);

private:
    float alpha_ = 0.0f;
    float x1_ = 0.0f;
    float y1_ = 0.0f;
};

} // namespace doomfuzz
