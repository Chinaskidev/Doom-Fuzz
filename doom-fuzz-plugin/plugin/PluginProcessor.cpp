#include "PluginProcessor.h"
#include "PluginEditor.h"

DoomFuzzProcessor::DoomFuzzProcessor()
    : AudioProcessor(BusesProperties()
          .withInput("Input", juce::AudioChannelSet::stereo(), true)
          .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      apvts_(*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout
DoomFuzzProcessor::createParameterLayout() {
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"gain", 1}, "Gain",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.7f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"volume", 1}, "Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.5f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"tone", 1}, "Tone",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.35f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"bass", 1}, "Bass",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.6f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"octave", 1}, "Octave",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.3f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"gateTh", 1}, "Gate",
        juce::NormalisableRange<float>(-80.0f, -20.0f, 0.1f), -60.0f));

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID{"sag", 1}, "Sag",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), 0.4f));

    params.push_back(std::make_unique<juce::AudioParameterBool>(
        juce::ParameterID{"bypass", 1}, "Bypass", false));

    return { params.begin(), params.end() };
}

void DoomFuzzProcessor::prepareToPlay(double sampleRate, int /*samplesPerBlock*/) {
    engine_.prepare(static_cast<float>(sampleRate));
}

void DoomFuzzProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                      juce::MidiBuffer&) {
    juce::ScopedNoDenormals noDenormals;

    // Bridge APVTS parameters to DSP engine
    doomfuzz::Parameters p;
    p.gain   = apvts_.getRawParameterValue("gain")->load();
    p.volume = apvts_.getRawParameterValue("volume")->load();
    p.tone   = apvts_.getRawParameterValue("tone")->load();
    p.bass   = apvts_.getRawParameterValue("bass")->load();
    p.octave = apvts_.getRawParameterValue("octave")->load();
    p.gateTh = apvts_.getRawParameterValue("gateTh")->load();
    p.sag    = apvts_.getRawParameterValue("sag")->load();
    p.bypass = apvts_.getRawParameterValue("bypass")->load() > 0.5f;
    engine_.setParameters(p);

    const int numSamples = buffer.getNumSamples();

    // Ensure stereo output
    if (buffer.getNumChannels() < 2)
        buffer.setSize(2, numSamples, true, false, true);

    float* chL = buffer.getWritePointer(0);
    float* chR = buffer.getWritePointer(1);
    const float* inL = buffer.getReadPointer(0);
    const float* inR = buffer.getNumChannels() > 1 ? buffer.getReadPointer(1) : nullptr;

    engine_.processBlock(inL, inR, chL, chR, numSamples);

    // Calculate output level for metering
    float peak = 0.0f;
    for (int i = 0; i < numSamples; ++i) {
        float absL = std::fabs(chL[i]);
        float absR = std::fabs(chR[i]);
        float m = absL > absR ? absL : absR;
        if (m > peak) peak = m;
    }
    outputLevel_.store(peak);
}

void DoomFuzzProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto state = apvts_.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void DoomFuzzProcessor::setStateInformation(const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xml(getXmlFromBinary(data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName(apvts_.state.getType()))
        apvts_.replaceState(juce::ValueTree::fromXml(*xml));
}

juce::AudioProcessorEditor* DoomFuzzProcessor::createEditor() {
    return new DoomFuzzEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() {
    return new DoomFuzzProcessor();
}
