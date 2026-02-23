cmake_minimum_required(VERSION 3.22)

add_library(DoomFuzzDSP STATIC
    src/Biquad.cpp
    src/EnvelopeFollower.cpp
    src/DelayLine.cpp
    src/NoiseGate.cpp
    src/GainStage.cpp
    src/VoltageSag.cpp
    src/OctaveDown.cpp
    src/ToneStack.cpp
    src/CabinetSim.cpp
    src/StereoWidth.cpp
    src/DoomFuzzEngine.cpp
)

target_include_directories(DoomFuzzDSP PUBLIC include)
target_compile_features(DoomFuzzDSP PUBLIC cxx_std_17)
