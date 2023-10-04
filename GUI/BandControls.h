/*
  ==============================================================================

    BandControls.h
    Created: 21 Sep 2023 2:16:24pm
    Author:  Aidan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "RotarySliderWithLabels.h"

struct BandControls : juce::Component, juce::Button::Listener
{
    BandControls(juce::AudioProcessorValueTreeState& apvts);
    ~BandControls() override;
    void resized() override;
    void paint(juce::Graphics& g) override;
    void buttonClicked(juce::Button* button) override;

    void toggleAllBands(bool bypassed);
private:
    juce::AudioProcessorValueTreeState& apvts;
    RotarySliderWithLabels attackSlider, releaseSlider, thresholdSlider;
    RatioSlider ratioSlider;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> attackSliderATT, releaseSliderATT, thresholdSliderATT, ratioSliderATT;

    juce::ToggleButton bypassButton, soloButton, muteButton, lowBand, midBand, highBand;

    using BtnAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<BtnAttachment> bypassButtonATT, soloButtonATT, muteButtonATT;

    juce::Component::SafePointer<BandControls> safePtr{ this };

    juce::ToggleButton* activeBand = &lowBand;

    void updateAttachments();
    void updateSliderEnablements();
    void updateButtonStates(juce::Button& button);

    void updateActiveBandColors(juce::Button& button);
    void resetActiveBandColors();
    static void refreshButtonColors(juce::Button& band, juce::Button& colorSource);

    void updateBandSelectButtonStates();
};
