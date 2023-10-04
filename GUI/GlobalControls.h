/*
  ==============================================================================

    GlobalControls.h
    Created: 21 Sep 2023 2:18:48pm
    Author:  Aidan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "RotarySliderWithLabels.h"
#include "Utilities.h"

struct GlobalControls : juce::Component
{
    GlobalControls(juce::AudioProcessorValueTreeState& apvts);

    void paint(juce::Graphics& g) override;

    void resized() override;
private:
    using RSWL = RotarySliderWithLabels;
    std::unique_ptr<RSWL> inGainSlider, lowMidSlider, midHighSlider, outGainSlider;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> lowMidSliderATT, midHighSliderATT, inGainSliderATT, outGainSliderATT;
};