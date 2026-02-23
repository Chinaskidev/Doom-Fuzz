#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class DoomFuzzLookAndFeel : public juce::LookAndFeel_V4 {
public:
    DoomFuzzLookAndFeel();

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    // Colors
    static constexpr uint32_t kBackground  = 0xFF0A0A0A;
    static constexpr uint32_t kPedalBody   = 0xFF1A1A1A;
    static constexpr uint32_t kBorder      = 0xFF2A2A2A;
    static constexpr uint32_t kGold        = 0xFFE8A020;
    static constexpr uint32_t kLabelText   = 0xFFBBBBBB;
    static constexpr uint32_t kSubText     = 0xFF666666;
    static constexpr uint32_t kLedOn       = 0xFF00FF44;
    static constexpr uint32_t kLedOff      = 0xFF331A00;
    static constexpr uint32_t kRed         = 0xFFFF4444;
    static constexpr uint32_t kBlue        = 0xFF44AAFF;
    static constexpr uint32_t kPurple      = 0xFFAA44FF;
    static constexpr uint32_t kGreen       = 0xFF44FF88;
    static constexpr uint32_t kGray        = 0xFF888888;
};
