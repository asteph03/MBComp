/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DSP/Params.h"

//==============================================================================
MBCompAudioProcessor::MBCompAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    using namespace Params;
    const auto& params = GetParams();

    auto floatHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
    {
        param = dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    floatHelper(lowBandComp.attack, Names::LowAttack);
    floatHelper(lowBandComp.release, Names::LowRelease);
    floatHelper(lowBandComp.threshold, Names::LowThreshold);

    floatHelper(midBandComp.attack, Names::MidAttack);
    floatHelper(midBandComp.release, Names::MidRelease);
    floatHelper(midBandComp.threshold, Names::MidThreshold);

    floatHelper(highBandComp.attack, Names::HighAttack);
    floatHelper(highBandComp.release, Names::HighRelease);
    floatHelper(highBandComp.threshold, Names::HighThreshold);

    auto choiceHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
    {
        param = dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    choiceHelper(lowBandComp.ratio, Names::LowRatio);
    choiceHelper(midBandComp.ratio, Names::MidRatio);
    choiceHelper(highBandComp.ratio, Names::HighRatio);

    auto boolHelper = [&apvts = this->apvts, &params](auto& param, const auto& paramName)
    {
        param = dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(params.at(paramName)));
        jassert(param != nullptr);
    };

    boolHelper(lowBandComp.bypassed, Names::LowBypassed);
    boolHelper(midBandComp.bypassed, Names::MidBypassed);
    boolHelper(highBandComp.bypassed, Names::HighBypassed);

    boolHelper(lowBandComp.mute, Names::LowMute);
    boolHelper(midBandComp.mute, Names::MidMute);
    boolHelper(highBandComp.mute, Names::HighMute);

    boolHelper(lowBandComp.solo, Names::LowSolo);
    boolHelper(midBandComp.solo, Names::MidSolo);
    boolHelper(highBandComp.solo, Names::HighSolo);

    floatHelper(lowMidCrossover, Names::LowMidCrossoverFreq);
    floatHelper(midHighCrossover, Names::MidHighCrossoverFreq);

    floatHelper(inGainParam, Names::GainIn);
    floatHelper(outGainParam, Names::GainOut);

    LP1.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    LP2.setType(juce::dsp::LinkwitzRileyFilterType::lowpass);
    HP1.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    HP2.setType(juce::dsp::LinkwitzRileyFilterType::highpass);
    AP2.setType(juce::dsp::LinkwitzRileyFilterType::allpass);
}

MBCompAudioProcessor::~MBCompAudioProcessor()
{
}

//==============================================================================
const juce::String MBCompAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool MBCompAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool MBCompAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool MBCompAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double MBCompAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int MBCompAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int MBCompAudioProcessor::getCurrentProgram()
{
    return 0;
}

void MBCompAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String MBCompAudioProcessor::getProgramName (int index)
{
    return {};
}

void MBCompAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void MBCompAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;

    for (auto & comp : compressors)
    {
        comp.prepare(spec);
    }

    LP1.prepare(spec);
    HP1.prepare(spec);
    LP2.prepare(spec);
    HP2.prepare(spec);
    AP2.prepare(spec);

    inputGain.prepare(spec);
    outputGain.prepare(spec);

    inputGain.setRampDurationSeconds(0.05);
    outputGain.setRampDurationSeconds(0.05);

    for (auto& buffer : filterBuffers)
    {
        buffer.setSize(spec.numChannels, samplesPerBlock);
    }

    leftChannelFifo.prepare(samplesPerBlock);
    rightChannelFifo.prepare(samplesPerBlock);

    osc.initialise([](float x) {return std::sin(x); });
    osc.prepare(spec);
    osc.setFrequency(getSampleRate() / ((2 << FFTOrder::order2048) - 1) * 50);

    gain.prepare(spec);
    gain.setGainDecibels(-12.f);
}

void MBCompAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool MBCompAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void MBCompAudioProcessor::updateState()
{
    for (auto& comp : compressors)
    {
        comp.updateCompressorSettings();
    }

    auto lowMidCutoff = lowMidCrossover->get();
    LP1.setCutoffFrequency(lowMidCutoff);
    HP1.setCutoffFrequency(lowMidCutoff);

    auto midHighCutoff = midHighCrossover->get();
    AP2.setCutoffFrequency(midHighCutoff);
    LP2.setCutoffFrequency(midHighCutoff);
    HP2.setCutoffFrequency(midHighCutoff);

    inputGain.setGainDecibels(inGainParam->get());
    outputGain.setGainDecibels(outGainParam->get());
}

void MBCompAudioProcessor::splitBands(const juce::AudioBuffer<float> &inputBuffer)
{
    for (auto& filterBuffer : filterBuffers)
    {
        filterBuffer = inputBuffer;
    }

    auto fb0Block = juce::dsp::AudioBlock<float>(filterBuffers[0]);
    auto fb1Block = juce::dsp::AudioBlock<float>(filterBuffers[1]);
    auto fb2Block = juce::dsp::AudioBlock<float>(filterBuffers[2]);

    auto fb0Ctx = juce::dsp::ProcessContextReplacing<float>(fb0Block);
    auto fb1Ctx = juce::dsp::ProcessContextReplacing<float>(fb1Block);
    auto fb2Ctx = juce::dsp::ProcessContextReplacing<float>(fb2Block);

    LP1.process(fb0Ctx);
    AP2.process(fb0Ctx);

    HP1.process(fb1Ctx);
    filterBuffers[2] = filterBuffers[1];
    LP2.process(fb1Ctx);

    HP2.process(fb2Ctx);
}

void MBCompAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    updateState();

    if (false)
    {
        buffer.clear();
        auto block = juce::dsp::AudioBlock<float>(buffer);
        auto ctx = juce::dsp::ProcessContextReplacing<float>(block);
        osc.process(ctx);
        gain.process(ctx);
    }

    leftChannelFifo.update(buffer);
    rightChannelFifo.update(buffer);

    applyGain(buffer, inputGain);

    splitBands(buffer);

    for (size_t i = 0; i < filterBuffers.size(); ++i)
    {
        compressors[i].process(filterBuffers[i]);
    }

    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    buffer.clear();

    auto addFilterBand = [nc = numChannels, ns = numSamples](auto& inputBuffer, const auto& source)
    {
        for (auto i = 0; i < nc; i++)
        {
            inputBuffer.addFrom(i, 0, source, i, 0, ns);
        }
    };

    auto solo = false;
    for (auto& comp : compressors)
    {
        if (comp.solo->get())
        {
            solo = true;
            break;
        }
    }

    if (solo)
    {
        for (size_t i = 0; i < compressors.size(); ++i)
        {
            auto& comp = compressors[i];
            if (comp.solo->get())
            {
                addFilterBand(buffer, filterBuffers[i]);
            }
        }
    }
    else
    {
        for (size_t i = 0; i < compressors.size(); ++i)
        {
            auto& comp = compressors[i];
            if (!comp.mute->get())
            {
                addFilterBand(buffer, filterBuffers[i]);
            }
        }
    }

    applyGain(buffer, outputGain);

    /*addFilterBand(buffer, filterBuffers[0]);
    addFilterBand(buffer, filterBuffers[1]);
    addFilterBand(buffer, filterBuffers[2]);*/
}

//==============================================================================
bool MBCompAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* MBCompAudioProcessor::createEditor()
{
    return new MBCompAudioProcessorEditor (*this);
    //return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void MBCompAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::MemoryOutputStream m(destData, true);
    apvts.state.writeToStream(m);
}

void MBCompAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    auto tree = juce::ValueTree::readFromData(data, sizeInBytes);
    if (tree.isValid()) {
        apvts.replaceState(tree);
    }
}

juce::AudioProcessorValueTreeState::ParameterLayout MBCompAudioProcessor::createParameterLayout() 
{
    APVTS::ParameterLayout layout;

    using namespace juce;
    using namespace Params;
    const auto& params = GetParams();

    auto gainRange = NormalisableRange<float>(-24.f, 24.f, 0.5f, 1.f);
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::GainIn), params.at(Names::GainIn), gainRange, 0));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::GainOut), params.at(Names::GainOut), gainRange, 0));

    auto thresholdRange = NormalisableRange<float>(MINTHRESH, MAXDB, 1, 1);
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::LowThreshold), params.at(Names::LowThreshold), thresholdRange, 0));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::MidThreshold), params.at(Names::MidThreshold), thresholdRange, 0));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::HighThreshold), params.at(Names::HighThreshold), thresholdRange, 0));

    auto arRange = NormalisableRange<float>(5, 500, 1, 1);
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::LowAttack), params.at(Names::LowAttack), arRange, 50));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::MidAttack), params.at(Names::MidAttack), arRange, 50));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::HighAttack), params.at(Names::HighAttack), arRange, 50));
    
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::LowRelease), params.at(Names::LowRelease), arRange, 250));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::MidRelease), params.at(Names::MidRelease), arRange, 250));
    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::HighRelease), params.at(Names::HighRelease), arRange, 250));

    auto choices = std::vector<double>{ 1, 1.5, 2, 3, 4, 5, 6, 7, 8, 10, 15, 20, 50, 100 };
    juce::StringArray sa;
    for (auto choice : choices) {
        sa.add(juce::String(choice, 1));
    }

    layout.add(std::make_unique<AudioParameterChoice>(params.at(Names::LowRatio), params.at(Names::LowRatio), sa, 3));
    layout.add(std::make_unique<AudioParameterChoice>(params.at(Names::MidRatio), params.at(Names::MidRatio), sa, 3));
    layout.add(std::make_unique<AudioParameterChoice>(params.at(Names::HighRatio), params.at(Names::HighRatio), sa, 3));

    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::LowBypassed), params.at(Names::LowBypassed), false));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::MidBypassed), params.at(Names::MidBypassed), false));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::HighBypassed), params.at(Names::HighBypassed), false));

    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::LowMute), params.at(Names::LowMute), false));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::MidMute), params.at(Names::MidMute), false));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::HighMute), params.at(Names::HighMute), false));

    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::LowSolo), params.at(Names::LowSolo), false));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::MidSolo), params.at(Names::MidSolo), false));
    layout.add(std::make_unique<AudioParameterBool>(params.at(Names::HighSolo), params.at(Names::HighSolo), false));

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::LowMidCrossoverFreq),params.at(Names::LowMidCrossoverFreq), NormalisableRange<float>(MINFREQ, 999, 1, 1), 400));

    layout.add(std::make_unique<AudioParameterFloat>(params.at(Names::MidHighCrossoverFreq), params.at(Names::MidHighCrossoverFreq), NormalisableRange<float>(1000, MAXFREQ, 1, 1), 2000));

    return layout;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new MBCompAudioProcessor();
}
