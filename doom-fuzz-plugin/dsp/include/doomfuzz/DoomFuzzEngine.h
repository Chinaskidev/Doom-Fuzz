#pragma once

#include "Parameters.h"
#include "NoiseGate.h"
#include "GainStage.h"
#include "VoltageSag.h"
#include "OctaveDown.h"
#include "ToneStack.h"
#include "CabinetSim.h"
#include "StereoWidth.h"

namespace doomfuzz {

class DoomFuzzEngine {
public:
    void prepare(float sampleRate);
    void processBlock(const float* inputL, const float* inputR,
                      float* outputL, float* outputR, int numSamples);
    void setParameters(const Parameters& params);

private:
    void processSample(float x, float& outL, float& outR);

    Parameters params_;
    float sampleRate_ = 44100.0f;

    NoiseGate   gate_;
    GainStage   gainStage_;
    VoltageSag  voltageSag_;
    OctaveDown  octaveDown_;
    ToneStack   toneStack_;
    CabinetSim  cabinetSim_;
    StereoWidth stereoWidth_;
};

} // namespace doomfuzz
