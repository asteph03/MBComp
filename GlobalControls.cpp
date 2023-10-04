/*
  ==============================================================================

    GlobalControls.cpp
    Created: 21 Sep 2023 2:18:48pm
    Author:  Aidan

  ==============================================================================
*/

#include "GlobalControls.h"
#include "../DSP/Params.h"

GlobalControls::GlobalControls(juce::AudioProcessorValueTreeState& apvts)
{
    using namespace Params;
    const auto& params = GetParams();

    auto getParamHelper = [&params, &apvts](const auto& name) -> auto&
    {
        return getParam(apvts, params, name);
    };

    auto& inGainParam = getParamHelper(Names::GainIn);
    auto& lowMidParam = getParamHelper(Names::LowMidCrossoverFreq);
    auto& midHighParam = getParamHelper(Names::MidHighCrossoverFreq);
    auto& outGainParam = getParamHelper(Names::GainOut);

    inGainSlider = std::make_unique<RSWL>(&inGainParam, "dB", "INPUT GAIN");
    lowMidSlider = std::make_unique<RSWL>(&lowMidParam, "Hz", "LOW-MID X-OVER");
    midHighSlider = std::make_unique<RSWL>(&midHighParam, "Hz", "MID-HI X-OVER");
    outGainSlider = std::make_unique<RSWL>(&outGainParam, "dB", "OUTPUT GAIN");

    auto makeAttachmentHelper = [&params, &apvts](auto& attachment, const auto& name, auto& slider)
    {
        makeAttachment(attachment, apvts, params, name, slider);
    };

    makeAttachmentHelper(inGainSliderATT, Names::GainIn, *inGainSlider);
    makeAttachmentHelper(lowMidSliderATT, Names::LowMidCrossoverFreq, *lowMidSlider);
    makeAttachmentHelper(midHighSliderATT, Names::MidHighCrossoverFreq, *midHighSlider);
    makeAttachmentHelper(outGainSliderATT, Names::GainOut, *outGainSlider);

    addLabelPairs(inGainSlider->labels, inGainParam, "dB");
    addLabelPairs(lowMidSlider->labels, lowMidParam, "Hz");
    addLabelPairs(midHighSlider->labels, midHighParam, "Hz");
    addLabelPairs(outGainSlider->labels, outGainParam, "dB");

    addAndMakeVisible(*inGainSlider);
    addAndMakeVisible(*lowMidSlider);
    addAndMakeVisible(*midHighSlider);
    addAndMakeVisible(*outGainSlider);
}

void GlobalControls::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds();
    drawModuleBackground(g, bounds);
}

void GlobalControls::resized()
{
    auto bounds = getLocalBounds().reduced(5);
    using namespace juce;

    FlexBox flexBox;
    flexBox.flexDirection = FlexBox::Direction::row;
    flexBox.flexWrap = FlexBox::Wrap::noWrap;

    auto spacer = FlexItem().withWidth(4);
    auto endCap = FlexItem().withWidth(6);
    flexBox.items.add(endCap);
    flexBox.items.add(FlexItem(*inGainSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*lowMidSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*midHighSlider).withFlex(1.f));
    flexBox.items.add(spacer);
    flexBox.items.add(FlexItem(*outGainSlider).withFlex(1.f));
    flexBox.items.add(endCap);

    flexBox.performLayout(bounds);
}
