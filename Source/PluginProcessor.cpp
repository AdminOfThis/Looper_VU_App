#include "PluginProcessor.h"
#include "PluginEditor.h"

VUMeterAudioProcessor::VUMeterAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true))
{
    addParameter (bankParam = new juce::AudioParameterBool ("bank58", "Bank 5-8", false));

    for (int i = 0; i < kNumChannels; ++i)
        addParameter (muteParams[i] = new juce::AudioParameterBool (
            "ch" + juce::String (i + 1) + "mute",
            "Ch " + juce::String (i + 1) + " Mute",
            false));
}

bool VUMeterAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    return in == out && !in.isDisabled();
}

void VUMeterAudioProcessor::prepareToPlay (double sr, int samplesPerBlock)
{
    double blocksPerSec = sr / samplesPerBlock;
    rmsAlpha_ = (float) std::exp (-1.0 / (kRmsTimeConstantSec * blocksPerSec));
    currentRms_ = 0.0f;
    sharedLevels[myChannel.load()].store (0.0f);
}

void VUMeterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    // Only write shared state when THIS instance's parameters change,
    // so other instances with stable values don't overwrite.
    const bool paramVal = bankParam->get();
    if (paramVal != prevBankParam_)
    {
        showUpperBank.store (paramVal);
        prevBankParam_ = paramVal;
    }

    for (int i = 0; i < kNumChannels; ++i)
    {
        const bool m = muteParams[i]->get();
        if (m != prevMuteParams_[i])
        {
            sharedMutes[i].store (m);
            prevMuteParams_[i] = m;
        }
    }

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels == 0 || numSamples == 0)
        return;

    const float* data = buffer.getReadPointer (0);

    float rmsSq = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        rmsSq += data[i] * data[i];

    currentRms_ = rmsAlpha_ * currentRms_ + (1.0f - rmsAlpha_) * std::sqrt (rmsSq / (float) numSamples);
    sharedLevels[myChannel.load()].store (currentRms_);
}

void VUMeterAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::MemoryOutputStream stream (destData, false);
    stream.writeInt  (myChannel.load());
    stream.writeBool (bankParam->get());
    for (int i = 0; i < kNumChannels; ++i)
        stream.writeBool (muteParams[i]->get());
    stream.writeInt (lastEditorWidth);
    stream.writeInt (lastEditorHeight);
}

void VUMeterAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::MemoryInputStream stream (data, (size_t) sizeInBytes, false);
    if (stream.getNumBytesRemaining() >= 4)
        myChannel.store (juce::jlimit (0, kNumChannels - 1, stream.readInt()));
    if (stream.getNumBytesRemaining() >= 1)
    {
        const bool bank = stream.readBool();
        *bankParam = bank;
        showUpperBank.store (bank);
        prevBankParam_ = bank;
    }
    for (int i = 0; i < kNumChannels; ++i)
    {
        if (stream.getNumBytesRemaining() < 1) break;
        const bool m = stream.readBool();
        *muteParams[i] = m;
        sharedMutes[i].store (m);
        prevMuteParams_[i] = m;
    }
    if (stream.getNumBytesRemaining() >= 4) { lastEditorWidth  = stream.readInt();  sharedEditorWidth .store (lastEditorWidth); }
    if (stream.getNumBytesRemaining() >= 4) { lastEditorHeight = stream.readInt();  sharedEditorHeight.store (lastEditorHeight); }
}

juce::AudioProcessorEditor* VUMeterAudioProcessor::createEditor()
{
    return new VUMeterAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VUMeterAudioProcessor();
}
