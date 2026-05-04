#include "PluginProcessor.h"
#include "PluginEditor.h"

VUMeterAudioProcessor::VUMeterAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
}

bool VUMeterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    // Accept any layout where input matches output and isn't empty
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    return in == out && !in.isDisabled();
}

void VUMeterAudioProcessor::prepareToPlay (double sr, int samplesPerBlock)
{
    sampleRate_ = sr;

    // Convert time-domain decay rates to per-block linear multipliers.
    // peak:     -kPeakDecayDbPerSec dB every second
    // hold:     -kHoldDecayDbPerSec dB every second (after hold expires)
    // rms:      exponential smoothing with kRmsTimeConstantSec time constant
    auto blockDecay = [&] (float dbPerSec)
    {
        double linPerSec = std::pow (10.0, (double)-dbPerSec / 20.0);
        return (float) std::pow (linPerSec, (double)samplesPerBlock / sr);
    };

    for (int i = 0; i < kNumFallPresets; ++i)
        peakDecayPresets_[i] = blockDecay (kFallTimePresetsDb[i]);

    holdDecayPerBlock_ = blockDecay (kHoldDecayDbPerSec);

    double blocksPerSec = sr / samplesPerBlock;
    rmsAlpha_ = (float) std::exp (-1.0 / (kRmsTimeConstantSec * blocksPerSec));

    currentPeak_   = 0.0f;
    currentRms_    = 0.0f;
    currentHold_   = 0.0f;
    holdSampsLeft_ = 0;

    peakLevel.store (0.0f);
    rmsLevel .store (0.0f);
    peakHold .store (0.0f);
}

void VUMeterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels == 0 || numSamples == 0)
        return;

    // Measure channel 0 (left / mono); all channels pass through unchanged.
    const float* data = buffer.getReadPointer (0);

    float blockPeak  = 0.0f;
    float blockRmsSq = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        const float s = std::abs (data[i]);
        if (s > blockPeak) blockPeak = s;
        blockRmsSq += s * s;
    }

    const float blockRms = std::sqrt (blockRmsSq / (float) numSamples);

    // Decaying peak (fall rate selected at runtime)
    currentPeak_ = std::max (blockPeak, currentPeak_ * peakDecayPresets_[fallTimeIndex.load()]);

    // Smoothed RMS
    currentRms_ = rmsAlpha_ * currentRms_ + (1.0f - rmsAlpha_) * blockRms;

    // Peak hold
    if (blockPeak >= currentHold_)
    {
        currentHold_   = blockPeak;
        holdSampsLeft_ = (int) (kHoldTimeSec * (float) sampleRate_);
    }
    else if (holdSampsLeft_ > 0)
    {
        holdSampsLeft_ -= numSamples;
    }
    else
    {
        currentHold_ *= holdDecayPerBlock_;
    }

    peakLevel.store (currentPeak_);
    rmsLevel .store (currentRms_);
    peakHold .store (currentHold_);
}

void VUMeterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream (destData, false);
    stream.writeInt  (fallTimeIndex.load());
    stream.writeBool (peakMode.load());
}

void VUMeterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream (data, (size_t) sizeInBytes, false);
    if (stream.getNumBytesRemaining() >= 4)
        fallTimeIndex.store (juce::jlimit (0, kNumFallPresets - 1, stream.readInt()));
    if (stream.getNumBytesRemaining() >= 1)
        peakMode.store (stream.readBool());
}

juce::AudioProcessorEditor* VUMeterAudioProcessor::createEditor()
{
    return new VUMeterAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VUMeterAudioProcessor();
}
