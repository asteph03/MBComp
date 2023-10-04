/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/LookAndFeel.h"
#include "GUI/GlobalControls.h"
#include "GUI/BandControls.h"
#include "GUI/UtilityComps.h"
#include "GUI/SpectrumAnalyzer.h"
#include "GUI/CustomButtons.h"

struct ControlBar : juce::Component
{
    ControlBar();
    void resized() override;

    AnalyzerButton analyzerButton;
    PowerButton globalBypass;
};

class MBCompAudioProcessorEditor  : public juce::AudioProcessorEditor, juce::Timer
{
public:
    MBCompAudioProcessorEditor (MBCompAudioProcessor&);
    ~MBCompAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    void timerCallback() override;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    LookAndFeel lnf;

    MBCompAudioProcessor& audioProcessor;

    ControlBar controlBar;
    GlobalControls globalControls{ audioProcessor.apvts };
    BandControls bandControls{ audioProcessor.apvts };
    SpectrumAnalyzer analyzer{ audioProcessor };

    void toggleGlobalBypass();

    std::array<juce::AudioParameterBool*, 3> getBypassParams();

    void updateGlobalBypass();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MBCompAudioProcessorEditor)
};
