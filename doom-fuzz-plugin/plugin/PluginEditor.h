#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"
#include "ui/DoomFuzzLookAndFeel.h"
#include "ui/KnobComponent.h"
#include "ui/LevelMeterComponent.h"

class DoomFuzzEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
    explicit DoomFuzzEditor(DoomFuzzProcessor&);
    ~DoomFuzzEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    DoomFuzzProcessor& processor_;
    DoomFuzzLookAndFeel lookAndFeel_;

    // LED indicator
    juce::Component ledIndicator_;

    // Level meter
    LevelMeterComponent levelMeter_;

    // Main knobs: Gain, Tone, Volume
    KnobComponent gainKnob_;
    KnobComponent toneKnob_;
    KnobComponent volumeKnob_;

    // Secondary knobs: Bass, Octave, Sag
    KnobComponent bassKnob_;
    KnobComponent octaveKnob_;
    KnobComponent sagKnob_;

    // Gate knob (smaller)
    KnobComponent gateKnob_;

    // Footswitch (bypass toggle)
    juce::ToggleButton footswitch_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAttachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DoomFuzzEditor)
};
