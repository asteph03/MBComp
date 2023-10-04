/*
  ==============================================================================

    CompressorBand.h
    Created: 21 Sep 2023 2:23:12pm
    Author:  Aidan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "../GUI/Utilities.h"

struct CompressorBand {
    juce::AudioParameterFloat* attack{ nullptr };
    juce::AudioParameterFloat* release{ nullptr };
    juce::AudioParameterFloat* threshold{ nullptr };
    juce::AudioParameterChoice* ratio{ nullptr };
    juce::AudioParameterBool* bypassed{ nullptr };
    juce::AudioParameterBool* mute{ nullptr };
    juce::AudioParameterBool* solo{ nullptr };

    void prepare(const juce::dsp::ProcessSpec& spec);

    void updateCompressorSettings();

    void process(juce::AudioBuffer<float>& buffer);

    float getRMSInputLevel() const { return rmsInputLevel; }
    float getRMSOutputLevel() const { return rmsOutputLevel; }
private:
    juce::dsp::Compressor<float> compressor;

    std::atomic<float> rmsInputLevel{ NEGINF };
    std::atomic<float> rmsOutputLevel{ NEGINF };

    template <typename T>
    float computeRMSLevel(const T& buffer)
    {
        int numChannels = static_cast<int>(buffer.getNumChannels());
        int numSamples = static_cast<int>(buffer.getNumSamples());
        auto rms = 0.f;

        for (int c = 0; c < numChannels; c++)
        {
            rms += buffer.getRMSLevel(c, 0, numSamples);
        }

        rms /= static_cast<float>(numChannels);
        return rms;
    }
};