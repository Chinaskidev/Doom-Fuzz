cmake_minimum_required(VERSION 3.22)
project(DoomFuzz VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use local JUCE installation
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../JUCE JUCE)

# DSP core library (pure C++, no JUCE dependency)
add_subdirectory(dsp)

# JUCE plugin
add_subdirectory(plugin)
