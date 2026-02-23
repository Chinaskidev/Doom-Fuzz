#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>

class KnobComponent : public juce::Component {
public:
    KnobComponent(juce::AudioProcessorValueTreeState& apvts,
                  const juce::String& paramId,
                  const juce::String& label,
                  juce::Colour colour,
                  int size = 56);
    ~KnobComponent() override = default;

    void resized() override;

private:
    juce::Slider slider_;
    juce::Label label_;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment_;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KnobComponent)
};
