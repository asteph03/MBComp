/*
  ==============================================================================

    CompressorBand.cpp
    Created: 21 Sep 2023 2:23:12pm
    Author:  Aidan

  ==============================================================================
*/

#include "CompressorBand.h"

void CompressorBand::prepare(const juce::dsp::ProcessSpec& spec)
{
    compressor.prepare(spec);
}

void CompressorBand::updateCompressorSettings()
{
    compressor.setAttack(attack->get());
    compressor.setRelease(release->get());
    compressor.setThreshold(threshold->get());
    compressor.setRatio(ratio->getCurrentChoiceName().getFloatValue());
}

void CompressorBand::process(juce::AudioBuffer<float>& buffer)
{
    auto preRMS = computeRMSLevel(buffer);
    
    auto block = juce::dsp::AudioBlock<float>(buffer);
    auto context = juce::dsp::ProcessContextReplacing<float>(block);

    context.isBypassed = bypassed->get();

    compressor.process(context);

    auto postRMS = computeRMSLevel(buffer);

    rmsInputLevel.store(juce::Decibels::gainToDecibels(preRMS));
    rmsOutputLevel.store(juce::Decibels::gainToDecibels(postRMS));
}