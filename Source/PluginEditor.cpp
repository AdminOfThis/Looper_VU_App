#include "PluginEditor.h"

static constexpr float kDbMin = -60.0f;
static constexpr float kDbMax =   6.0f;

VUMeterAudioProcessorEditor::VUMeterAudioProcessorEditor (VUMeterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (60, 300);
    startTimerHz (60);
}

float VUMeterAudioProcessorEditor::dbToY (float db, float top, float bottom) noexcept
{
    db = juce::jlimit (kDbMin, kDbMax, db);
    float t = (db - kDbMin) / (kDbMax - kDbMin);
    return bottom - t * (bottom - top);
}

void VUMeterAudioProcessorEditor::paint (juce::Graphics& g)
{
    const float W = (float) getWidth();
    const float H = (float) getHeight();

    g.fillAll (juce::Colour (0xff1c1c1e));

    const float padL     = 8.f;
    const float padT     = 10.f;
    const float padB     = 10.f;
    const float meterW   = W - padL * 2.f;
    const float meterX   = padL;
    const float meterTop = padT;
    const float meterBot = H - padB;

    const float lin = audioProcessor.rmsLevel.load();
    const float db  = lin > 0.f ? 20.f * std::log10 (lin) : kDbMin;
    const float yL  = dbToY (db, meterTop, meterBot);

    if (yL < meterBot)
    {
        g.setColour (juce::Colour (0xff27ae60));
        g.fillRect (meterX, yL, meterW, meterBot - yL);
    }

    g.setColour (juce::Colour (0xff3a3a3c));
    g.drawRect (meterX, meterTop, meterW, meterBot - meterTop, 1.f);
}
