#include "KnobComponent.h"

KnobComponent::KnobComponent(juce::AudioProcessorValueTreeState& apvts,
                               const juce::String& paramId,
                               const juce::String& labelText,
                               juce::Colour colour,
                               int size)
{
    slider_.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider_.setTextBoxStyle(juce::Slider::NoTextBox, true, 0, 0);
    slider_.setColour(juce::Slider::thumbColourId, colour);
    slider_.setColour(juce::Slider::rotarySliderFillColourId, colour);
    addAndMakeVisible(slider_);

    label_.setText(labelText, juce::dontSendNotification);
    label_.setJustificationType(juce::Justification::centred);
    label_.setFont(juce::Font("Courier New", 10.0f, juce::Font::bold));
    label_.setColour(juce::Label::textColourId, juce::Colour(0xFFBBBBBB));
    addAndMakeVisible(label_);

    attachment_ = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        apvts, paramId, slider_);

    setSize(size + 10, size + 22);
}

void KnobComponent::resized() {
    auto bounds = getLocalBounds();
    label_.setBounds(bounds.removeFromBottom(16));
    slider_.setBounds(bounds);
}
