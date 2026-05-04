#include "PluginEditor.h"

static constexpr float kDbMin = -60.0f;
static constexpr float kDbMax =   6.0f;

static constexpr const char* kFallLabels[VUMeterAudioProcessor::kNumFallPresets] = {
    "Fast - 60 dB/s",
    "Medium - 20 dB/s",
    "Slow - 10 dB/s",
    "Very Slow - 4 dB/s"
};

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

void VUMeterAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    if (! e.mods.isRightButtonDown())
        return;

    const int  currentFall = audioProcessor.fallTimeIndex.load();
    const bool isPeakMode  = audioProcessor.peakMode.load();

    juce::PopupMenu fallMenu;
    for (int i = 0; i < VUMeterAudioProcessor::kNumFallPresets; ++i)
        fallMenu.addItem (i + 1, kFallLabels[i], true, currentFall == i);

    juce::PopupMenu menu;
    menu.addSubMenu ("Fall Time", fallMenu);
    menu.addSeparator();
    menu.addItem (10, "Peak Mode", true, isPeakMode);

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
        [this] (int result)
        {
            if (result >= 1 && result <= VUMeterAudioProcessor::kNumFallPresets)
                audioProcessor.fallTimeIndex.store (result - 1);
            else if (result == 10)
                audioProcessor.peakMode.store (! audioProcessor.peakMode.load());
        });
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

    // ── Level readout (linear → dB) ─────────────────────────────────────────
    auto toDb = [] (float lin) noexcept {
        return lin > 0.f ? 20.f * std::log10 (lin) : kDbMin;
    };

    const bool  isPeak = audioProcessor.peakMode.load();
    const float barDb  = toDb (isPeak ? audioProcessor.peakLevel.load()
                                      : audioProcessor.rmsLevel.load());
    const float holdDb = toDb (audioProcessor.peakHold.load());

    const float yL = dbToY (barDb, meterTop, meterBot);

    // ── Bar fill ─────────────────────────────────────────────────────────────
    if (yL < meterBot)
    {
        g.setColour (juce::Colour (0xff27ae60));
        g.fillRect (meterX, yL, meterW, meterBot - yL);
    }

    // ── Peak hold marker ─────────────────────────────────────────────────────
    if (audioProcessor.peakHold.load() > 0.001f)
    {
        float yH = dbToY (holdDb, meterTop, meterBot);
        g.setColour (juce::Colours::white.withAlpha (0.9f));
        g.fillRect (meterX, yH - 1.f, meterW, 2.f);
    }

    // ── Meter border ─────────────────────────────────────────────────────────
    g.setColour (juce::Colour (0xff3a3a3c));
    g.drawRect (meterX, meterTop, meterW, meterBot - meterTop, 1.f);
}
