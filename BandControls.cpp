/*
  ==============================================================================

    BandControls.cpp
    Created: 21 Sep 2023 2:16:24pm
    Author:  Aidan

  ==============================================================================
*/

#include "BandControls.h"
#include "Utilities.h"
#include "../DSP/Params.h"

BandControls::BandControls(juce::AudioProcessorValueTreeState& apv) : apvts(apv),
attackSlider(nullptr, "ms", "ATTACK"),
releaseSlider(nullptr, "ms", "RELEASE"),
thresholdSlider(nullptr, "dB", "THRESHOLD"),
ratioSlider(nullptr, "")
{
    addAndMakeVisible(attackSlider);
    addAndMakeVisible(releaseSlider);
    addAndMakeVisible(thresholdSlider);
    addAndMakeVisible(ratioSlider);

    bypassButton.addListener(this);
    soloButton.addListener(this);
    muteButton.addListener(this);

    bypassButton.setName("X");
    bypassButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::yellow);
    bypassButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    soloButton.setName("S");
    soloButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::limegreen);
    soloButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    muteButton.setName("M");
    muteButton.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::red);
    muteButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    addAndMakeVisible(bypassButton);
    addAndMakeVisible(soloButton);
    addAndMakeVisible(muteButton);

    lowBand.setName("Low");
    lowBand.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
    lowBand.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    midBand.setName("Mid");
    midBand.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
    midBand.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    highBand.setName("High");
    highBand.setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
    highBand.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);

    lowBand.setRadioGroupId(1);
    midBand.setRadioGroupId(1);
    highBand.setRadioGroupId(1);

    auto buttonSwitcher = [safePtr = this->safePtr]()
    {
        if (auto* c = safePtr.getComponent())
        {
            c->updateAttachments();
        }
    };

    lowBand.onClick = buttonSwitcher;
    midBand.onClick = buttonSwitcher;
    highBand.onClick = buttonSwitcher;

    lowBand.setToggleState(true, juce::NotificationType::dontSendNotification);

    updateAttachments();
    updateSliderEnablements();
    updateBandSelectButtonStates();

    addAndMakeVisible(lowBand);
    addAndMakeVisible(midBand);
    addAndMakeVisible(highBand);
}

BandControls::~BandControls()
{
    bypassButton.removeListener(this);
    soloButton.removeListener(this);
    muteButton.removeListener(this);
}

void BandControls::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    using namespace juce;

    auto createButtonBox = [](std::vector<Component*> comps)
    {
        FlexBox flexBox;
        flexBox.flexDirection = FlexBox::Direction::column;
        flexBox.flexWrap = FlexBox::Wrap::noWrap;

        auto spacer = FlexItem().withHeight(2);

        for (auto* comp : comps)
        {
            flexBox.items.add(spacer);
            flexBox.items.add(FlexItem(*comp).withFlex(1.f));
        }
        flexBox.items.add(spacer);

        return flexBox;
    };

    auto buttonBox = createButtonBox({ &bypassButton, &soloButton, &muteButton });
    auto selectBox = createButtonBox({ &lowBand, &midBand, &highBand });

    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;

    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(selectBox).withWidth(50));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(attackSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(releaseSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(thresholdSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(ratioSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(buttonBox).withWidth(30));

    flexBox.performLayout(bounds);
}

void BandControls::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    drawModuleBackground(g, bounds);
}

void BandControls::buttonClicked(juce::Button* button)
{
    updateSliderEnablements();
    updateButtonStates(*button);
    updateActiveBandColors(*button);
}

void BandControls::toggleAllBands(bool bypassed)
{
    std::vector<Component*> bands{ &lowBand, &midBand, &highBand };

    for (auto* band : bands)
    {
        band->setColour(juce::TextButton::ColourIds::buttonOnColourId, 
                        bypassed ? bypassButton.findColour(juce::TextButton::ColourIds::buttonOnColourId) : 
                        juce::Colours::grey);

        band->setColour(juce::TextButton::ColourIds::buttonColourId,
                        bypassed ? bypassButton.findColour(juce::TextButton::ColourIds::buttonOnColourId) :
                        juce::Colours::black);

        band->repaint();
    }
}

void BandControls::updateActiveBandColors(juce::Button& button)
{
    jassert(activeBand != nullptr);

    if (!button.getToggleState())
    {
        resetActiveBandColors();
    }
    else
    {
        refreshButtonColors(*activeBand, button);
    }
}

void BandControls::resetActiveBandColors()
{
    activeBand->setColour(juce::TextButton::ColourIds::buttonOnColourId, juce::Colours::grey);
    activeBand->setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colours::black);
    activeBand->repaint();
}

void BandControls::refreshButtonColors(juce::Button& band, juce::Button& colorSource)
{
    band.setColour(juce::TextButton::ColourIds::buttonOnColourId, colorSource.findColour(juce::TextButton::ColourIds::buttonOnColourId));
    band.setColour(juce::TextButton::ColourIds::buttonColourId, colorSource.findColour(juce::TextButton::ColourIds::buttonOnColourId));
    band.repaint();
}

void BandControls::updateBandSelectButtonStates()
{
    using namespace Params;
    std::vector<std::array<Names, 3>> checkParams
    {
        {Names::LowSolo, Names::LowMute, Names::LowBypassed},
        {Names::MidSolo, Names::MidMute, Names::MidBypassed},
        {Names::HighSolo, Names::HighMute, Names::HighBypassed}
    };

    const auto& params = GetParams();

    auto paramHelper = [&params, this](const auto& name)
    {
        return dynamic_cast<juce::AudioParameterBool*>(&getParam(apvts, params, name));
    };

    for (size_t i = 0; i < checkParams.size(); ++i)
    {
        auto& list = checkParams[i];

        auto* bandButton = (i == 0) ? &lowBand : (i == 1) ? &midBand : &highBand;

        if (auto* solo = paramHelper(list[0]); solo->get())
        {
            refreshButtonColors(*bandButton, soloButton);
        }
        else if (auto* mute = paramHelper(list[1]); mute->get())
        {
            refreshButtonColors(*bandButton, muteButton);
        }
        else if (auto* bypass = paramHelper(list[2]); bypass->get())
        {
            refreshButtonColors(*bandButton, bypassButton);
        }
    }
}

void BandControls::updateSliderEnablements()
{
    auto disabled = muteButton.getToggleState() || bypassButton.getToggleState();
    attackSlider.setEnabled(!disabled);
    releaseSlider.setEnabled(!disabled);
    thresholdSlider.setEnabled(!disabled);
    ratioSlider.setEnabled(!disabled);
}

void BandControls::updateButtonStates(juce::Button& button)
{
    if (&button == &soloButton && soloButton.getToggleState())
    {
        bypassButton.setToggleState(false, juce::NotificationType::sendNotification);
        muteButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
    else if (&button == &muteButton && muteButton.getToggleState())
    {
        bypassButton.setToggleState(false, juce::NotificationType::sendNotification);
        soloButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
    else if (&button == &bypassButton && bypassButton.getToggleState())
    {
        soloButton.setToggleState(false, juce::NotificationType::sendNotification);
        muteButton.setToggleState(false, juce::NotificationType::sendNotification);
    }
}

void BandControls::updateAttachments()
{
    enum BandType
    {
        Low,
        Mid,
        High
    };

    BandType bandType = [this]()
    {
        if (lowBand.getToggleState())
        {
            return BandType::Low;
        }
        if (midBand.getToggleState())
        {
            return BandType::Mid;
        }

        return BandType::High;
    }();

    using namespace Params;
    std::vector<Names> names;

    switch (bandType)
    {
    case Low:
    {
        names = std::vector<Names>
        {
            Names::LowAttack,
            Names::LowRelease,
            Names::LowThreshold,
            Names::LowRatio,
            Names::LowMute,
            Names::LowSolo,
            Names::LowBypassed
        };
        activeBand = &lowBand;
        break;
    }
    case Mid:
    {
        names = std::vector<Names>
        {
            Names::MidAttack,
            Names::MidRelease,
            Names::MidThreshold,
            Names::MidRatio,
            Names::MidMute,
            Names::MidSolo,
            Names::MidBypassed
        };
        activeBand = &midBand;
        break;
    }
    case High:
    {
        names = std::vector<Names>
        {
            Names::HighAttack,
            Names::HighRelease,
            Names::HighThreshold,
            Names::HighRatio,
            Names::HighMute,
            Names::HighSolo,
            Names::HighBypassed
        };
        activeBand = &highBand;
        break;
    }

    }

    enum Pos
    {
        Attack,
        Release,
        Threshold,
        Ratio,
        Mute,
        Solo,
        Bypass
    };

    const auto& params = GetParams();

    auto getParamHelper = [&params, &apvts = this->apvts, &names](const auto& pos) -> auto&
    {
        return getParam(apvts, params, names.at(pos));
    };

    attackSliderATT.reset();
    releaseSliderATT.reset();
    thresholdSliderATT.reset();
    ratioSliderATT.reset();
    bypassButtonATT.reset();
    soloButtonATT.reset();
    muteButtonATT.reset();

    auto& attackParam = getParamHelper(Pos::Attack);
    addLabelPairs(attackSlider.labels, attackParam, "ms");
    attackSlider.changeParam(&attackParam);

    auto& releaseParam = getParamHelper(Pos::Release);
    addLabelPairs(releaseSlider.labels, releaseParam, "ms");
    releaseSlider.changeParam(&releaseParam);

    auto& thresholdParam = getParamHelper(Pos::Threshold);
    addLabelPairs(thresholdSlider.labels, thresholdParam, "dB");
    thresholdSlider.changeParam(&thresholdParam);

    auto& ratioParamRap = getParamHelper(Pos::Ratio);
    ratioSlider.labels.clear();
    ratioSlider.labels.add({ 0.f, "1:1" });
    auto ratioParam = dynamic_cast<juce::AudioParameterChoice*>(&ratioParamRap);
    ratioSlider.labels.add({ 1.0f, juce::String(ratioParam->choices.getReference(ratioParam->choices.size() - 1).getIntValue()) + ":1" });
    ratioSlider.changeParam(ratioParam);

    auto makeAttachmentHelper = [&params, &apvts = this->apvts](auto& attachment, const auto& name, auto& slider)
    {
        makeAttachment(attachment, apvts, params, name, slider);
    };

    makeAttachmentHelper(attackSliderATT, names[Pos::Attack], attackSlider);
    makeAttachmentHelper(releaseSliderATT, names[Pos::Release], releaseSlider);
    makeAttachmentHelper(thresholdSliderATT, names[Pos::Threshold], thresholdSlider);
    makeAttachmentHelper(ratioSliderATT, names[Pos::Ratio], ratioSlider);
    makeAttachmentHelper(bypassButtonATT, names[Pos::Bypass], bypassButton);
    makeAttachmentHelper(soloButtonATT, names[Pos::Solo], soloButton);
    makeAttachmentHelper(muteButtonATT, names[Pos::Mute], muteButton);
}
