#include "PluginEditor.h"

static constexpr float kDbMin  = -60.0f;
static constexpr float kDbMax  =   6.0f;
static constexpr float kPadL   =   8.f;
static constexpr float kPadR   =   8.f;
static constexpr float kPadT   =   8.f;
static constexpr float kPadB   =   8.f;
static constexpr float kBarGap  =   4.f;
static constexpr float kLabelH  =  18.f;

static const juce::Colour kGreen  { 0xff27ae60 };
static const juce::Colour kPurple { 0xff8e44ad };

VUMeterAudioProcessorEditor::VUMeterAudioProcessorEditor (VUMeterAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setResizable (true, true);
    setResizeLimits (80, 150, 800, 1200);
    setSize (VUMeterAudioProcessor::sharedEditorWidth .load(),
             VUMeterAudioProcessor::sharedEditorHeight.load());
    startTimerHz (60);
}

float VUMeterAudioProcessorEditor::dbToY (float db, float top, float bottom) noexcept
{
    db = juce::jlimit (kDbMin, kDbMax, db);
    float t = (db - kDbMin) / (kDbMax - kDbMin);
    return bottom - t * (bottom - top);
}

void VUMeterAudioProcessorEditor::resized()
{
    audioProcessor.lastEditorWidth  = getWidth();
    audioProcessor.lastEditorHeight = getHeight();
    VUMeterAudioProcessor::sharedEditorWidth .store (getWidth());
    VUMeterAudioProcessor::sharedEditorHeight.store (getHeight());
}

void VUMeterAudioProcessorEditor::mouseDown (const juce::MouseEvent& e)
{
    if (! e.mods.isRightButtonDown())
        return;

    const int  myChannel    = audioProcessor.myChannel.load();
    const bool upperBank    = VUMeterAudioProcessor::showUpperBank.load();
    const int  kHalf        = VUMeterAudioProcessor::kNumChannels / 2;   // 4

    juce::PopupMenu chMenu;
    for (int i = 0; i < VUMeterAudioProcessor::kNumChannels; ++i)
        chMenu.addItem (i + 1, "Channel " + juce::String (i + 1), true, myChannel == i);

    juce::PopupMenu muteMenu;
    for (int i = 0; i < VUMeterAudioProcessor::kNumChannels; ++i)
        muteMenu.addItem (20 + i, "Channel " + juce::String (i + 1), true,
                          ! audioProcessor.muteParams[i]->get());

    juce::PopupMenu menu;
    menu.addSubMenu ("Assign Channel", chMenu);
    menu.addSeparator();
    menu.addItem (10, "Bank 1-" + juce::String (kHalf),       true, ! upperBank);
    menu.addItem (11, "Bank "   + juce::String (kHalf + 1) + "-" + juce::String (kHalf * 2), true, upperBank);
    menu.addSeparator();
    menu.addSubMenu ("Mute Channel", muteMenu);

    menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (this),
        [this] (int result)
        {
            const int kHalf = VUMeterAudioProcessor::kNumChannels / 2;
            if (result >= 1 && result <= VUMeterAudioProcessor::kNumChannels)
            {
                audioProcessor.myChannel.store (result - 1);
            }
            else if (result == 10 || result == 11)
            {
                const bool upper = (result == 11);
                audioProcessor.bankParam->beginChangeGesture();
                audioProcessor.bankParam->setValueNotifyingHost (upper ? 1.0f : 0.0f);
                audioProcessor.bankParam->endChangeGesture();
                VUMeterAudioProcessor::showUpperBank.store (upper);
            }
            else if (result >= 20 && result < 20 + VUMeterAudioProcessor::kNumChannels)
            {
                const int ch = result - 20;
                const bool newMute = ! audioProcessor.muteParams[ch]->get();
                audioProcessor.muteParams[ch]->beginChangeGesture();
                audioProcessor.muteParams[ch]->setValueNotifyingHost (newMute ? 1.0f : 0.0f);
                audioProcessor.muteParams[ch]->endChangeGesture();
                VUMeterAudioProcessor::sharedMutes[ch].store (newMute);
            }
        });
}

void VUMeterAudioProcessorEditor::paint (juce::Graphics& g)
{
    const float W        = (float) getWidth();
    const float H        = (float) getHeight();
    const int   kHalf    = VUMeterAudioProcessor::kNumChannels / 2;   // 4
    const bool  upper    = VUMeterAudioProcessor::showUpperBank.load();
    const int   bankBase = upper ? kHalf : 0;

    const float contentW = W - kPadL - kPadR;
    const float barW     = (contentW - (kHalf - 1) * kBarGap) / (float) kHalf;
    const float stride   = barW + kBarGap;
    const float meterTop = kPadT + kLabelH;
    const float meterBot = H - kPadB;

    g.fillAll (juce::Colour (0xff1c1c1e));

    const juce::Colour barColour = upper ? kPurple : kGreen;

    for (int i = 0; i < kHalf; ++i)
    {
        const float bx      = kPadL + (float) i * stride;
        const int   slotIdx = bankBase + i;

        g.setColour (upper ? kPurple : juce::Colours::white);
        g.setFont (juce::Font (kLabelH, juce::Font::bold));
        g.drawText (juce::String (slotIdx + 1),
                    (int) bx, (int) kPadT, (int) barW, (int) kLabelH,
                    juce::Justification::centred, false);

        const bool muted = ! VUMeterAudioProcessor::sharedMutes[slotIdx].load();

        const float lin = VUMeterAudioProcessor::sharedLevels[slotIdx].load();
        const float db  = lin > 0.f ? 20.f * std::log10 (lin) : kDbMin;
        const float yL  = dbToY (db, meterTop, meterBot);

        if (yL < meterBot)
        {
            g.setColour (muted ? juce::Colour (0xff555555) : barColour);
            g.fillRect (bx, yL, barW, meterBot - yL);
        }

        g.setColour (juce::Colour (0xff3a3a3c));
        g.drawRect (bx, meterTop, barW, meterBot - meterTop, 1.f);
    }
}
