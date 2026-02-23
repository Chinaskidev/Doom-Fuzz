#include "LevelMeterComponent.h"

LevelMeterComponent::LevelMeterComponent() {
    startTimerHz(30);
}

void LevelMeterComponent::setLevel(float newLevel) {
    level_ = newLevel;
}

void LevelMeterComponent::timerCallback() {
    // Smooth falloff
    const float target = level_;
    if (target > displayLevel_)
        displayLevel_ = target;
    else
        displayLevel_ *= 0.85f;

    repaint();
}

void LevelMeterComponent::paint(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();

    // Background
    g.setColour(juce::Colour(0xFF222222));
    g.fillRoundedRectangle(bounds, 2.0f);

    // Level bar
    const float w = bounds.getWidth() * juce::jmin(displayLevel_, 1.0f);
    juce::Colour barColour;
    if (displayLevel_ > 0.8f)
        barColour = juce::Colour(0xFFFF3333);
    else if (displayLevel_ > 0.5f)
        barColour = juce::Colour(0xFFE8A020);
    else
        barColour = juce::Colour(0xFF00FF44);

    g.setColour(barColour);
    g.fillRoundedRectangle(bounds.withWidth(w), 2.0f);
}
