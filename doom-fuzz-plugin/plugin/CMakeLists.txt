juce_add_plugin(DoomFuzzPlugin
    PRODUCT_NAME "PATADOOM"
    COMPANY_NAME "Yultic"
    BUNDLE_ID "com.yultic.patadoom"
    PLUGIN_MANUFACTURER_CODE Yult
    PLUGIN_CODE DmFz
    FORMATS VST3 Standalone
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

target_sources(DoomFuzzPlugin PRIVATE
    PluginProcessor.cpp
    PluginEditor.cpp
    ui/DoomFuzzLookAndFeel.cpp
    ui/KnobComponent.cpp
    ui/LevelMeterComponent.cpp
)

target_compile_definitions(DoomFuzzPlugin PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(DoomFuzzPlugin
    PRIVATE
        DoomFuzzDSP
        juce::juce_audio_utils
        juce::juce_audio_processors
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags
)
