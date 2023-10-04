/*
  ==============================================================================

    Utility.cpp
    Created: 21 Sep 2023 1:59:41pm
    Author:  Aidan

  ==============================================================================
*/

#include "UtilityComps.h"

Placeholder::Placeholder()
{
    juce::Random r;
    customColor = juce::Colour(r.nextInt(255), r.nextInt(255), r.nextInt(255));
}

void Placeholder::paint(juce::Graphics& g)
{
    g.fillAll(customColor);
}

RotarySlider::RotarySlider() :
    juce::Slider(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag, juce::Slider::TextEntryBoxPosition::NoTextBox)
{ }