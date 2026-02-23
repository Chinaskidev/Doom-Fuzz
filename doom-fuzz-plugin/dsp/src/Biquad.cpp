#include "doomfuzz/Biquad.h"
#include <cmath>

namespace doomfuzz {

static constexpr float kPi = 3.14159265358979323846f;

// ── Biquad (2nd order) ──────────────────────────────────────

void Biquad::reset() {
    x1_ = x2_ = y1_ = y2_ = 0.0f;
}

void Biquad::setLowpass(float freq, float sampleRate, float Q) {
    const float w0    = 2.0f * kPi * freq / sampleRate;
    const float sinw  = std::sin(w0);
    const float cosw  = std::cos(w0);
    const float alpha = sinw / (2.0f * Q);

    const float a0 = 1.0f + alpha;
    b0_ = ((1.0f - cosw) / 2.0f) / a0;
    b1_ = (1.0f - cosw) / a0;
    b2_ = ((1.0f - cosw) / 2.0f) / a0;
    a1_ = (-2.0f * cosw) / a0;
    a2_ = (1.0f - alpha) / a0;
}

void Biquad::setHighpass(float freq, float sampleRate, float Q) {
    const float w0    = 2.0f * kPi * freq / sampleRate;
    const float sinw  = std::sin(w0);
    const float cosw  = std::cos(w0);
    const float alpha = sinw / (2.0f * Q);

    const float a0 = 1.0f + alpha;
    b0_ = ((1.0f + cosw) / 2.0f) / a0;
    b1_ = (-(1.0f + cosw)) / a0;
    b2_ = ((1.0f + cosw) / 2.0f) / a0;
    a1_ = (-2.0f * cosw) / a0;
    a2_ = (1.0f - alpha) / a0;
}

void Biquad::setPeakEQ(float freq, float sampleRate, float gainDb, float bw) {
    const float A     = std::pow(10.0f, gainDb / 40.0f);
    const float w0    = 2.0f * kPi * freq / sampleRate;
    const float sinw  = std::sin(w0);
    const float alpha = sinw * std::sinh((std::log(2.0f) / 2.0f) * bw * (w0 / sinw));

    const float a0 = 1.0f + alpha / A;
    b0_ = (1.0f + alpha * A) / a0;
    b1_ = (-2.0f * std::cos(w0)) / a0;
    b2_ = (1.0f - alpha * A) / a0;
    a1_ = (-2.0f * std::cos(w0)) / a0;
    a2_ = (1.0f - alpha / A) / a0;
}

float Biquad::process(float x) {
    const float y = b0_ * x + b1_ * x1_ + b2_ * x2_ - a1_ * y1_ - a2_ * y2_;
    x2_ = x1_;
    x1_ = x;
    y2_ = y1_;
    y1_ = y;
    return y;
}

// ── OnePoleHP (1st order) ───────────────────────────────────

void OnePoleHP::reset() {
    x1_ = y1_ = 0.0f;
}

void OnePoleHP::setFreq(float freq, float sampleRate) {
    const float rc = 1.0f / (2.0f * kPi * freq);
    const float dt = 1.0f / sampleRate;
    alpha_ = rc / (rc + dt);
}

float OnePoleHP::process(float x) {
    const float y = alpha_ * (y1_ + x - x1_);
    x1_ = x;
    y1_ = y;
    return y;
}

} // namespace doomfuzz
