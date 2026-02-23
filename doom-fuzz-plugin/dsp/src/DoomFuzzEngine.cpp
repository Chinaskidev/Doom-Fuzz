#include "doomfuzz/DoomFuzzEngine.h"

namespace doomfuzz {

void DoomFuzzEngine::prepare(float sampleRate) {
    sampleRate_ = sampleRate;

    gate_.reset();
    gainStage_.prepare(sampleRate);
    voltageSag_.prepare(sampleRate);
    octaveDown_.prepare(sampleRate);
    toneStack_.prepare(sampleRate, params_.tone);
    cabinetSim_.prepare(sampleRate);
    stereoWidth_.prepare(sampleRate);
}

void DoomFuzzEngine::setParameters(const Parameters& params) {
    params_ = params;
}

void DoomFuzzEngine::processBlock(const float* inputL, const float* inputR,
                                   float* outputL, float* outputR, int numSamples) {
    for (int i = 0; i < numSamples; ++i) {
        // Sum to mono
        float mono = inputL[i];
        if (inputR != nullptr)
            mono += inputR[i];

        float L, R;
        processSample(mono, L, R);
        outputL[i] = L;
        outputR[i] = R;
    }
}

void DoomFuzzEngine::processSample(float x, float& outL, float& outR) {
    // Bypass: clean pass-through
    if (params_.bypass) {
        outL = x;
        outR = x;
        return;
    }

    // Noise gate
    x = gate_.process(x, params_.gateTh);

    // Gain stage (includes bass boost + 3-stage clipping)
    x = gainStage_.process(x, params_.gain, params_.bass);

    // Voltage sag compressor
    x = voltageSag_.process(x, params_.sag);

    // Octave down
    x = octaveDown_.process(x, params_.octave);

    // Tone stack
    x = toneStack_.process(x, params_.tone);

    // Cabinet simulation
    x = cabinetSim_.process(x);

    // Output volume
    x *= params_.volume;

    // Stereo width (Haas effect)
    stereoWidth_.process(x, outL, outR);
}

} // namespace doomfuzz
