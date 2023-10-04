/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DSP/Params.h"

ControlBar::ControlBar()
{
    analyzerButton.setToggleState(true, juce::NotificationType::dontSendNotification);
    addAndMakeVisible(analyzerButton);
    addAndMakeVisible(globalBypass);
}

void ControlBar::resized()
{
    auto bounds = getLocalBounds();
    analyzerButton.setBounds(bounds.removeFromLeft(50).withTrimmedTop(4).withTrimmedBottom(4));

    globalBypass.setBounds(bounds.removeFromRight(60).withTrimmedTop(2).withTrimmedBottom(2));
}

//==============================================================================
MBCompAudioProcessorEditor::MBCompAudioProcessorEditor (MBCompAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setLookAndFeel(&lnf);

    controlBar.analyzerButton.onClick = [this]()
    {
        auto toggleState = controlBar.analyzerButton.getToggleState();
        analyzer.toggleAnalysisEnablement(toggleState);
    };

    controlBar.globalBypass.onClick = [this]()
    {
        toggleGlobalBypass();
    };

    addAndMakeVisible(controlBar);
    addAndMakeVisible(analyzer);
    addAndMakeVisible(globalControls);
    addAndMakeVisible(bandControls);

    setSize (720, 600);

    startTimerHz(60);
}

MBCompAudioProcessorEditor::~MBCompAudioProcessorEditor()
{
    setLookAndFeel(nullptr);
}

//==============================================================================
void MBCompAudioProcessorEditor::paint (juce::Graphics& g)
{
    /*
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Hello World!", getLocalBounds(), juce::Justification::centred, 1);
    */
    g.fillAll(juce::Colours::black);
}

void MBCompAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();
    controlBar.setBounds(bounds.removeFromTop(38));

    bandControls.setBounds(bounds.removeFromBottom(162));

    analyzer.setBounds(bounds.removeFromTop(270));

    globalControls.setBounds(bounds);
}

void MBCompAudioProcessorEditor::timerCallback()
{
    std::vector<float> values
    {
        audioProcessor.lowBandComp.getRMSInputLevel(),
        audioProcessor.lowBandComp.getRMSOutputLevel(),
        audioProcessor.midBandComp.getRMSInputLevel(),
        audioProcessor.midBandComp.getRMSOutputLevel(),
        audioProcessor.highBandComp.getRMSInputLevel(),
        audioProcessor.highBandComp.getRMSOutputLevel()
    };

    analyzer.update(values);

    updateGlobalBypass();
}

void MBCompAudioProcessorEditor::updateGlobalBypass()
{
    auto params = getBypassParams();

    bool bypassed = std::all_of(params.begin(), params.end(), [](const auto& param) { return param->get(); });

    controlBar.globalBypass.setToggleState(bypassed, juce::NotificationType::dontSendNotification);
}

void MBCompAudioProcessorEditor::toggleGlobalBypass()
{
    auto toggleState = !(controlBar.globalBypass.getToggleState());

    auto params = getBypassParams();

    auto bypassParamHelper = [](auto* param, bool bypassed)
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost(bypassed ? 1.f : 0.f);
        param->endChangeGesture();
    };

    for (auto* param : params)
    {
        bypassParamHelper(param, !toggleState);
    }

    bandControls.toggleAllBands(!toggleState);
}

std::array<juce::AudioParameterBool*, 3> MBCompAudioProcessorEditor::getBypassParams()
{
    using namespace juce;
    using namespace Params;
    const auto& params = Params::GetParams();
    auto& apvts = audioProcessor.apvts;

    auto boolHelper = [&apvts, &params](const auto& paramName)
    {
        auto param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);

        return param;
    };

    auto* lowBypassParam = boolHelper(Names::LowBypassed);
    auto* midBypassParam = boolHelper(Names::MidBypassed);
    auto* highBypassParam = boolHelper(Names::HighBypassed);

    return
    {
        lowBypassParam,
        midBypassParam,
        highBypassParam
    };
}