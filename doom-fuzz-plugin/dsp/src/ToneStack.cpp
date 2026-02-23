#include "doomfuzz/ToneStack.h"
#include <cmath>

namespace doomfuzz {

void ToneStack::prepare(float sampleRate, float tone) {
    sampleRate_ = sampleRate;
    lastTone_ = tone;
    const float lpFreq = 400.0f + tone * 3000.0f;
    const float hpFreq = 80.0f + (1.0f - tone) * 200.0f;
    lpTone_.setLowpass(lpFreq, sampleRate);
    hpTone_.setFreq(hpFreq, sampleRate);
}

void ToneStack::reset() {
    lpTone_.reset();
    hpTone_.reset();
    lastTone_ = -1.0f;
}

float ToneStack::process(float x, float tone) {
    // Recalculate coefficients only when tone changes
    if (tone != lastTone_) {
        lastTone_ = tone;
        const float lpFreq = 400.0f + tone * 3000.0f;
        const float hpFreq = 80.0f + (1.0f - tone) * 200.0f;
        lpTone_.setLowpass(lpFreq, sampleRate_);
        hpTone_.setFreq(hpFreq, sampleRate_);
    }

    const float lpSig = lpTone_.process(x);
    const float hpSig = hpTone_.process(x);
    return lpSig * (1.0f - tone * 0.6f) + hpSig * (tone * 0.6f);
}

} // namespace doomfuzz
