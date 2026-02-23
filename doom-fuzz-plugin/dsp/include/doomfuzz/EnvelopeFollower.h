#pragma once

namespace doomfuzz {

class EnvelopeFollower {
public:
    void reset();
    void setCoeffs(float attackCoeff, float releaseCoeff);
    float process(float x);
    float getEnvelope() const { return env_; }

private:
    float attackCoeff_  = 0.01f;
    float releaseCoeff_ = 0.0003f;
    float env_ = 0.0f;
};

} // namespace doomfuzz
