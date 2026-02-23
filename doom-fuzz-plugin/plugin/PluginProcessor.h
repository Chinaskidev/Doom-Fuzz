#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "doomfuzz/DoomFuzzEngine.h"

class DoomFuzzProcessor : public juce::AudioProcessor {
public:
    DoomFuzzProcessor();
    ~DoomFuzzProcessor() override = default;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "Doom Fuzz"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override { return {}; }
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts_; }
    float getOutputLevel() const { return outputLevel_.load(); }

private:
    juce::AudioProcessorValueTreeState apvts_;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    doomfuzz::DoomFuzzEngine engine_;
    std::atomic<float> outputLevel_ { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoomFuzzProcessor)
};
