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
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int size)  override;

    // ── Metering state — written on audio thread, read on UI thread ────────────
    // All linear amplitude values (not dB).
    std::atomic<float> peakLevel { 0.0f };   // decaying peak
    std::atomic<float> rmsLevel  { 0.0f };   // smoothed RMS
    std::atomic<float> peakHold  { 0.0f };   // held peak marker

    // ── Metering config ────────────────────────────────────────────────────────
    static constexpr float kFallTimePresetsDb[4] = { 60.f, 20.f, 10.f, 4.f }; // dB/sec options
    static constexpr int   kNumFallPresets        = 4;
    static constexpr float kRmsTimeConstantSec    = 0.30f;
    static constexpr float kHoldTimeSec           = 1.50f;
    static constexpr float kHoldDecayDbPerSec     = 40.0f;

    // ── UI-controlled state (written on UI thread, read on audio/UI thread) ──
    std::atomic<int>  fallTimeIndex { 0 };     // index into kFallTimePresetsDb (default: Fast)
    std::atomic<bool> peakMode      { true };  // false = RMS bar, true = Peak bar

private:
    double sampleRate_       { 44100.0 };
    float  currentPeak_      { 0.0f };
    float  currentRms_       { 0.0f };
    float  currentHold_      { 0.0f };
    int    holdSampsLeft_    { 0 };

    float  peakDecayPresets_[4] {};   // one precomputed multiplier per fall-time preset
    float  rmsAlpha_            { 0.0f };
    float  holdDecayPerBlock_   { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VUMeterAudioProcessor)
};
