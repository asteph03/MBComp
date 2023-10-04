/*
  ==============================================================================

    CustomButtons.h
    Created: 21 Sep 2023 1:52:47pm
    Author:  Aidan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

struct PowerButton : juce::ToggleButton { };

struct AnalyzerButton : juce::ToggleButton
{
    void resized() override;

    juce::Path randomPath;
};