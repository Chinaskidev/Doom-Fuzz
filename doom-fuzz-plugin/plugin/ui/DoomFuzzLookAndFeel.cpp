#include "DoomFuzzLookAndFeel.h"

DoomFuzzLookAndFeel::DoomFuzzLookAndFeel() {
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(kBackground));
}

void DoomFuzzLookAndFeel::drawRotarySlider(juce::Graphics& g,
    int x, int y, int width, int height,
    float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
    juce::Slider& slider)
{
    const float radius = static_cast<float>(juce::jmin(width, height)) / 2.0f - 4.0f;
    const float centreX = static_cast<float>(x) + static_cast<float>(width) * 0.5f;
    const float centreY = static_cast<float>(y) + static_cast<float>(height) * 0.5f;

    // Knob body: metallic gradient
    juce::ColourGradient bodyGrad(
        juce::Colour(0xFF555555), centreX - radius * 0.3f, centreY - radius * 0.3f,
        juce::Colour(0xFF111111), centreX + radius * 0.5f, centreY + radius * 0.5f,
        true);
    g.setGradientFill(bodyGrad);
    g.fillEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f);

    // Border
    g.setColour(juce::Colour(0xFF333333));
    g.drawEllipse(centreX - radius, centreY - radius, radius * 2.0f, radius * 2.0f, 2.0f);

    // Indicator line
    const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const float lineLength = radius * 0.65f;
    const float lineX = centreX + std::sin(angle) * lineLength;
    const float lineY = centreY - std::cos(angle) * lineLength;

    // Get indicator color from slider property
    auto colour = slider.findColour(juce::Slider::thumbColourId);
    g.setColour(colour);
    g.drawLine(centreX, centreY, lineX, lineY, 3.0f);

    // Glow dot at tip
    g.setColour(colour.withAlpha(0.5f));
    g.fillEllipse(lineX - 3.0f, lineY - 3.0f, 6.0f, 6.0f);
}

void DoomFuzzLookAndFeel::drawToggleButton(juce::Graphics& g,
    juce::ToggleButton& button,
    bool shouldDrawButtonAsHighlighted,
    bool /*shouldDrawButtonAsDown*/)
{
    const auto bounds = button.getLocalBounds().toFloat().reduced(4.0f);
    const float size = juce::jmin(bounds.getWidth(), bounds.getHeight());
    const float centreX = bounds.getCentreX();
    const float centreY = bounds.getCentreY();
    const float radius = size * 0.5f;
    const bool isOn = button.getToggleState();

    // Footswitch circle
    juce::ColourGradient grad(
        isOn ? juce::Colour(0xFF444444) : juce::Colour(0xFF333333),
        centreX - radius * 0.3f, centreY - radius * 0.3f,
        isOn ? juce::Colour(0xFF222222) : juce::Colour(0xFF1A1A1A),
        centreX + radius, centreY + radius, true);
    g.setGradientFill(grad);
    g.fillEllipse(centreX - radius, centreY - radius, size, size);

    // Border
    g.setColour(isOn ? juce::Colour(kGold) : juce::Colour(0xFF333333));
    g.drawEllipse(centreX - radius, centreY - radius, size, size, 3.0f);

    // Label
    g.setColour(isOn ? juce::Colour(kGold) : juce::Colour(0xFF555555));
    g.setFont(juce::Font("Courier New", 12.0f, juce::Font::bold));
    g.drawText(isOn ? "ON" : "OFF",
               button.getLocalBounds(), juce::Justification::centred);

    // Highlight
    if (shouldDrawButtonAsHighlighted) {
        g.setColour(juce::Colours::white.withAlpha(0.05f));
        g.fillEllipse(centreX - radius, centreY - radius, size, size);
    }
}
