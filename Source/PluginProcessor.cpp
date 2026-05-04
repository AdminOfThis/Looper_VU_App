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
    const auto& in  = layouts.getMainInputChannelSet();
    const auto& out = layouts.getMainOutputChannelSet();
    return in == out && !in.isDisabled();
}

void VUMeterAudioProcessor::prepareToPlay (double sr, int samplesPerBlock)
{
    double blocksPerSec = sr / samplesPerBlock;
    rmsAlpha_ = (float) std::exp (-1.0 / (kRmsTimeConstantSec * blocksPerSec));
    currentRms_ = 0.0f;
    rmsLevel.store (0.0f);
}

void VUMeterAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const int numSamples  = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();

    if (numChannels == 0 || numSamples == 0)
        return;

    const float* data = buffer.getReadPointer (0);

    float rmsSq = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        rmsSq += data[i] * data[i];

    const float blockRms = std::sqrt (rmsSq / (float) numSamples);
    currentRms_ = rmsAlpha_ * currentRms_ + (1.0f - rmsAlpha_) * blockRms;
    rmsLevel.store (currentRms_);
}

juce::AudioProcessorEditor* VUMeterAudioProcessor::createEditor()
{
    return new VUMeterAudioProcessorEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VUMeterAudioProcessor();
}
