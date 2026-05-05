#pragma once
#include <JuceHeader.h>

class VUMeterAudioProcessor : public juce::AudioProcessor
{
public:
    static constexpr int kNumChannels = 8;

    // Shared across all instances in the same DAW process
    inline static std::atomic<float> sharedLevels[kNumChannels] {};
    inline static std::atomic<bool>  showUpperBank { false };  // false = 1-4, true = 5-8
    inline static std::atomic<bool>  sharedMutes[kNumChannels] {};
    inline static std::atomic<int>   sharedEditorWidth  { 140 };
    inline static std::atomic<int>   sharedEditorHeight { 300 };

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
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int size)  override;

    // Which of the 8 slots this instance writes to (0–7)
    std::atomic<int> myChannel { 0 };

    // Persisted editor size
    int lastEditorWidth  { 140 };
    int lastEditorHeight { 300 };

    // MIDI-mappable parameters — map via Ctrl+M in Ableton
    juce::AudioParameterBool* bankParam                  { nullptr };
    juce::AudioParameterBool* muteParams[kNumChannels]   {};

private:
    static constexpr float kRmsTimeConstantSec = 0.30f;

    float currentRms_                  { 0.0f };
    float rmsAlpha_                    { 0.0f };
    bool  prevBankParam_               { false };
    bool  prevMuteParams_[kNumChannels] {};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VUMeterAudioProcessor)
};
