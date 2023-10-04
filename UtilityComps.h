/*
  ==============================================================================

    Utility.h
    Created: 21 Sep 2023 1:59:41pm
    Author:  Aidan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct Placeholder : juce::Component
{
    Placeholder();

    void paint(juce::Graphics& g) override;
    juce::Colour customColor;
};

struct RotarySlider : juce::Slider
{
    RotarySlider();
};