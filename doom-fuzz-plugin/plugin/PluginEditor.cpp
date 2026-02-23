#include "PluginEditor.h"

DoomFuzzEditor::DoomFuzzEditor(DoomFuzzProcessor& p)
    : AudioProcessorEditor(p),
      processor_(p),
      gainKnob_(p.getAPVTS(), "gain", "GAIN", juce::Colour(DoomFuzzLookAndFeel::kRed), 56),
      toneKnob_(p.getAPVTS(), "tone", "TONE", juce::Colour(DoomFuzzLookAndFeel::kGold), 56),
      volumeKnob_(p.getAPVTS(), "volume", "VOLUME", juce::Colour(DoomFuzzLookAndFeel::kGold), 56),
      bassKnob_(p.getAPVTS(), "bass", "BASS", juce::Colour(DoomFuzzLookAndFeel::kBlue), 56),
      octaveKnob_(p.getAPVTS(), "octave", "OCTAVE", juce::Colour(DoomFuzzLookAndFeel::kPurple), 56),
      sagKnob_(p.getAPVTS(), "sag", "SAG", juce::Colour(DoomFuzzLookAndFeel::kGreen), 56),
      gateKnob_(p.getAPVTS(), "gateTh", "GATE", juce::Colour(DoomFuzzLookAndFeel::kGray), 46)
{
    setLookAndFeel(&lookAndFeel_);
    setSize(340, 580);

    addAndMakeVisible(levelMeter_);
    addAndMakeVisible(gainKnob_);
    addAndMakeVisible(toneKnob_);
    addAndMakeVisible(volumeKnob_);
    addAndMakeVisible(bassKnob_);
    addAndMakeVisible(octaveKnob_);
    addAndMakeVisible(sagKnob_);
    addAndMakeVisible(gateKnob_);

    footswitch_.setClickingTogglesState(true);
    addAndMakeVisible(footswitch_);

    // The bypass parameter is inverted: bypass=true means effect OFF
    // We use the toggle as "active" so we invert via a lambda is not possible
    // with ButtonAttachment. We'll attach directly and note that toggled ON = bypass OFF.
    bypassAttachment_ = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(
        p.getAPVTS(), "bypass", footswitch_);

    startTimerHz(30);
}

DoomFuzzEditor::~DoomFuzzEditor() {
    setLookAndFeel(nullptr);
}

void DoomFuzzEditor::timerCallback() {
    levelMeter_.setLevel(processor_.getOutputLevel());
    repaint(0, 86, getWidth(), 20); // LED area only
}

void DoomFuzzEditor::paint(juce::Graphics& g) {
    // Background gradient
    juce::ColourGradient bgGrad(
        juce::Colour(0xFF1A1510), getWidth() * 0.5f, getHeight() * 0.3f,
        juce::Colour(0xFF0A0A0A), getWidth() * 0.5f, static_cast<float>(getHeight()),
        true);
    g.setGradientFill(bgGrad);
    g.fillAll();

    // Pedal body
    auto pedalBounds = getLocalBounds().reduced(0).toFloat();
    juce::ColourGradient bodyGrad(
        juce::Colour(0xFF1E1E1E), 0.0f, 0.0f,
        juce::Colour(0xFF111111), 0.0f, static_cast<float>(getHeight()),
        false);
    g.setGradientFill(bodyGrad);
    g.fillRoundedRectangle(pedalBounds, 16.0f);

    // Border
    g.setColour(juce::Colour(DoomFuzzLookAndFeel::kBorder));
    g.drawRoundedRectangle(pedalBounds, 16.0f, 2.0f);

    // Metallic texture overlay (subtle noise pattern)
    g.setColour(juce::Colours::white.withAlpha(0.015f));
    for (int py = 0; py < getHeight(); py += 4)
        for (int px = 0; px < getWidth(); px += 4)
            g.fillRect(px, py, 1, 1);

    // Header: "YULTIC DSP"
    g.setColour(juce::Colour(DoomFuzzLookAndFeel::kSubText));
    g.setFont(juce::Font("Courier New", 11.0f, juce::Font::plain));
    g.drawText("YULTIC DSP", getLocalBounds().removeFromTop(45).withTrimmedTop(20),
               juce::Justification::centred);

    // Title: "DOOM FUZZ"
    g.setColour(juce::Colour(DoomFuzzLookAndFeel::kGold));
    g.setFont(juce::Font("Georgia", 28.0f, juce::Font::bold));
    g.drawText("DOOM FUZZ", getLocalBounds().removeFromTop(75).withTrimmedTop(42),
               juce::Justification::centred);

    // Subtitle
    g.setColour(juce::Colour(0xFF555555));
    g.setFont(juce::Font("Courier New", 9.0f, juce::Font::plain));
    g.drawText("STONER / DOOM DISTORTION",
               getLocalBounds().removeFromTop(90).withTrimmedTop(73),
               juce::Justification::centred);

    // LED indicator
    const bool active = !footswitch_.getToggleState(); // bypass inverted
    const float ledX = getWidth() * 0.5f - 5.0f;
    const float ledY = 98.0f;
    if (active) {
        g.setColour(juce::Colour(DoomFuzzLookAndFeel::kLedOn).withAlpha(0.3f));
        g.fillEllipse(ledX - 6.0f, ledY - 6.0f, 22.0f, 22.0f);
    }
    g.setColour(active ? juce::Colour(DoomFuzzLookAndFeel::kLedOn)
                       : juce::Colour(DoomFuzzLookAndFeel::kLedOff));
    g.fillEllipse(ledX, ledY, 10.0f, 10.0f);
    g.setColour(juce::Colour(0xFF333333));
    g.drawEllipse(ledX, ledY, 10.0f, 10.0f, 1.0f);

    // Footer: "HANDCRAFTED IN EL SALVADOR"
    g.setColour(juce::Colour(0xFF333333));
    g.setFont(juce::Font("Courier New", 8.0f, juce::Font::plain));
    g.drawText("HANDCRAFTED IN EL SALVADOR",
               getLocalBounds().removeFromBottom(30),
               juce::Justification::centred);
}

void DoomFuzzEditor::resized() {
    // Level meter
    levelMeter_.setBounds(24, 115, getWidth() - 48, 4);

    // Main knobs row: Gain, Tone, Volume
    const int knobSize = 66;
    const int mainY = 135;
    const int spacing = (getWidth() - knobSize * 3) / 4;
    gainKnob_.setBounds(spacing, mainY, knobSize, knobSize + 16);
    toneKnob_.setBounds(spacing * 2 + knobSize, mainY, knobSize, knobSize + 16);
    volumeKnob_.setBounds(spacing * 3 + knobSize * 2, mainY, knobSize, knobSize + 16);

    // Secondary knobs row: Bass, Octave, Sag
    const int secY = mainY + knobSize + 30;
    bassKnob_.setBounds(spacing, secY, knobSize, knobSize + 16);
    octaveKnob_.setBounds(spacing * 2 + knobSize, secY, knobSize, knobSize + 16);
    sagKnob_.setBounds(spacing * 3 + knobSize * 2, secY, knobSize, knobSize + 16);

    // Gate knob (centered, smaller)
    const int gateSize = 56;
    const int gateY = secY + knobSize + 30;
    gateKnob_.setBounds((getWidth() - gateSize) / 2, gateY, gateSize, gateSize + 16);

    // Footswitch
    const int switchSize = 80;
    const int switchY = gateY + gateSize + 35;
    footswitch_.setBounds((getWidth() - switchSize) / 2, switchY, switchSize, switchSize);
}
