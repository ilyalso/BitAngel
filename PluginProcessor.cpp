/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/
#include "JuceHeader.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <math.h>
#include <juce_audio_processors/juce_audio_processors.h>





//==============================================================================
Bit_depth3AudioProcessor::Bit_depth3AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()  
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
    ), treeState(*this, nullptr, "PARAMETERS", createParameterLayout())


#else



#endif

{
    treeState.addParameterListener("thresh", this);
    treeState.addParameterListener("gain", this);
    treeState.addParameterListener("bitcrushing", this);
    treeState.addParameterListener("downsampling", this);
    treeState.addParameterListener("cutoff", this);
    treeState.addParameterListener("xtragain", this);

    

}



juce::AudioProcessorValueTreeState::ParameterLayout Bit_depth3AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>("gain", "Gain", 0.0f, 3.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("bitcrushing", "BitCrushing", 1.0, 30, 16));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("downsampling", "DownSampling", 1.0f, 8.0f, 1.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("cutoff", "CutOff", 100.0f, 20000.0f, 20000.0f));
    params.push_back(std::make_unique<juce::AudioParameterBool>("xtragain", "XtraGain", false));







    return { params.begin(), params.end() };
}


Bit_depth3AudioProcessor::~Bit_depth3AudioProcessor()
{
    treeState.removeParameterListener("gain", this);
    treeState.removeParameterListener("bitcrushing", this);
    treeState.removeParameterListener("downsampling", this);
    treeState.removeParameterListener("cutoff", this);
    treeState.removeParameterListener("xtragain", this);
}




//==============================================================================
const juce::String Bit_depth3AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Bit_depth3AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Bit_depth3AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Bit_depth3AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Bit_depth3AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Bit_depth3AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Bit_depth3AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Bit_depth3AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Bit_depth3AudioProcessor::getProgramName (int index)
{
    return {};
}

void Bit_depth3AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void Bit_depth3AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    filter.setSamplingRate(static_cast<float>(sampleRate));
}

void Bit_depth3AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Bit_depth3AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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



void Bit_depth3AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    int downsampledIndex = 0;
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    


    for (int channel = 0; channel < totalNumInputChannels; ++channel)
    {
        auto* channelData = buffer.getWritePointer(channel);
        for (int samples = 0; samples < buffer.getNumSamples(); samples++)
        {
           // float dbValue = -6.0f; // example dB value
           // float gainValue = juce::Decibels::decibelsToGain(dbValue);

            float gainTreeState = *treeState.getRawParameterValue("gain");
            float bitDepth = *treeState.getRawParameterValue("bitcrushing");
            int ds = *treeState.getRawParameterValue("downsampling");
            gainFunction(channelData, samples, gainTreeState);
            
            float gainValue = channelData[samples];
            float dbValue = juce::Decibels::gainToDecibels(gainValue);
            float clippedData = juce::jlimit(-100.0f, 0.0f, dbValue);

            channelData[samples] = clippedData;
            float unClippedData = juce::Decibels::decibelsToGain(clippedData);
            channelData[samples] = unClippedData;

                    //bitcrush effect              
                    roundedSamples = round(channelData[samples]);
                    float sample = channelData[samples];
                    float crushedSample = round(sample * bitDepth) / bitDepth;
                    channelData[samples] = crushedSample;

                   
                    if (ds > 1 || ds > 8.0000001)
                        //downsampling effect
                    {

                        if (samples % ds == 0)

                        {

                            if (downsampledIndex % ds == 0 && downsampledIndex < downsampledData.size())
                            {
                                channelData[samples] = downsampledData[downsampledIndex]; ++downsampledIndex;
                            }
                        }
                        else
                        {
                            channelData[samples] = 0;
                        }
                        
                    }
                    
                    if (treeState.getParameterAsValue("xtragain") == true)
                    {
                        *treeState.getRawParameterValue("gain") = gainTreeState * 100000000000000;
                        
                    }
        }
        //lp filter
        if (*treeState.getRawParameterValue("cutoff") > 0)
        {
            float cutoffFrequency = *treeState.getRawParameterValue("cutoff");
            filter.setCutoffFrequency(cutoffFrequency);
            filter.processBlock(buffer, midiMessages);
        }
    }
}

//==============================================================================
bool Bit_depth3AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Bit_depth3AudioProcessor::createEditor()
{
return new Bit_depth3AudioProcessorEditor (*this);

}

//==============================================================================
void Bit_depth3AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = treeState.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary( *xml, destData );
}

void Bit_depth3AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr <juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(treeState.state.getType()))
            treeState.replaceState(juce::ValueTree::fromXml(*xmlState));
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Bit_depth3AudioProcessor();
}



