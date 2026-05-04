#pragma once
#include <JuceHeader.h>

class VUMeterAudioProcessor : public juce::AudioProcessor
{
public:
    VUMeterAudioProcessor();
    ~VUMeterAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor()                    const override { return true; }
    const juce::String getName()        const override { return JucePlugin_Name; }
    bool acceptsMidi()                  const override { return false; }
    bool producesMidi()                 const override { return false; }
    bool isMidiEffect()                 const override { return false; }
    double getTailLengthSeconds()       const override { return 0.0; }
    int getNumPrograms()                      override { return 1; }
    int getCurrentProgram()                   override { return 0; }
    void setCurrentProgram (int)              override {}
    const juce::String getProgramName (int)   override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&)      override {}
    void setStateInformation (const void*, int)        override {}

    // Written on audio thread, read on UI thread (linear amplitude)
    std::atomic<float> rmsLevel { 0.0f };

private:
    static constexpr float kRmsTimeConstantSec = 0.30f;

    float currentRms_ { 0.0f };
    float rmsAlpha_   { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VUMeterAudioProcessor)
};
