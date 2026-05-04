#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class VUMeterAudioProcessorEditor : public juce::AudioProcessorEditor,
                                     private juce::Timer
{
public:
    explicit VUMeterAudioProcessorEditor (VUMeterAudioProcessor&);
    ~VUMeterAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override {}
    void mouseDown (const juce::MouseEvent&) override;

private:
    void timerCallback() override { repaint(); }

    static float dbToY (float db, float top, float bottom) noexcept;

    VUMeterAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VUMeterAudioProcessorEditor)
};
