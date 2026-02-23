#pragma once

namespace doomfuzz {

struct Parameters {
    float gain    = 0.7f;
    float volume  = 0.5f;
    float tone    = 0.35f;
    float bass    = 0.6f;
    float octave  = 0.3f;
    float gateTh  = -60.0f;  // dB
    float sag     = 0.4f;
    bool  bypass  = false;
};

} // namespace doomfuzz
