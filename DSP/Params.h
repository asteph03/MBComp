/*
  ==============================================================================

    Params.h
    Created: 21 Sep 2023 2:20:55pm
    Author:  Aidan

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

namespace Params {
    enum Names {
        LowMidCrossoverFreq,
        MidHighCrossoverFreq,

        LowThreshold,
        MidThreshold,
        HighThreshold,

        LowAttack,
        MidAttack,
        HighAttack,

        LowRelease,
        MidRelease,
        HighRelease,

        LowRatio,
        MidRatio,
        HighRatio,

        LowBypassed,
        MidBypassed,
        HighBypassed,

        LowMute,
        MidMute,
        HighMute,

        LowSolo,
        MidSolo,
        HighSolo,

        GainIn,
        GainOut,
    };

    inline const std::map<Names, juce::String>& GetParams() {
        static std::map<Names, juce::String> params =
        {
            {LowMidCrossoverFreq, "Low-Mid Crossover Frequency"},
            {MidHighCrossoverFreq, "Mid-High Crossover Frequency"},
            {LowThreshold, "Threshold Low Band"},
            {MidThreshold, "Threshold Mid Band"},
            {HighThreshold, "Threshold High Band"},
            {LowAttack, "Attack Low Band"},
            {MidAttack, "Attack Mid Band"},
            {HighAttack, "Attack High Band"},
            {LowRelease, "Release Low Band"},
            {MidRelease, "Release Mid Band"},
            {HighRelease, "Release High Band"},
            {LowRatio, "Ratio Low Band"},
            {MidRatio, "Ratio Mid Band"},
            {HighRatio, "Ratio High Band"},
            {LowBypassed, "Bypassed Low Band"},
            {MidBypassed, "Bypassed Mid Band"},
            {HighBypassed, "Bypassed High Band"},
            {LowMute, "Mute Low Band"},
            {MidMute, "Mute Mid Band"},
            {HighMute, "Mute High Band"},
            {LowSolo, "Solo Low Band"},
            {MidSolo, "Solo Mid Band"},
            {HighSolo, "Solo High Band"},
            {GainIn, "Gain In"},
            {GainOut, "Gain Out"},
        };

        return params;
    }
}