#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class LevelMeterComponent : public juce::Component, private juce::Timer {
public:
    LevelMeterComponent();
    void setLevel(float newLevel);
    void paint(juce::Graphics&) override;

private:
    void timerCallback() override;
    float level_ = 0.0f;
    float displayLevel_ = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeterComponent)
};
