/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>
#include "LowpassHighpassFilter.h"
#include <juce_audio_processors/juce_audio_processors.h>
#define GAIN_ID "gain"
#define GAIN_NAME "Gain"


//==============================================================================
/**
*/

class Bit_depth3AudioProcessor : public juce::AudioProcessor, public juce::AudioProcessorValueTreeState::Listener 



{
public:
    //==============================================================================
    Bit_depth3AudioProcessor();
    ~Bit_depth3AudioProcessor() override;
    std::vector<float> downsampledData;
    std::atomic<float>* cutofffrequencyParameter = nullptr;
    float cutoffFrequency;
    std::vector<float> dnBuffer;
    juce::AudioProcessorValueTreeState treeState;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;


private:
    

    std::atomic<float>* highpassParameter = nullptr;
    LowpassHighpassFilter filter;
    float highpass;
    float bitrate;
    float roundedSamples;
    float xtraGain;
    bool on;
    bool off;


    //params==============


    void parameterChanged(const juce::String& parameterID, float newValue)
    
    {

        DBG(treeState.getRawParameterValue("gain")->load());
        DBG(treeState.getRawParameterValue("bitcrushing")->load());
        DBG(treeState.getRawParameterValue("downsampling")->load());
        DBG(treeState.getRawParameterValue("cutoff")->load());
        DBG(treeState.getRawParameterValue("xtragain")->load());


    }
    void gainFunction(float channelData[], int samples, float gainTreeState)
    {
        channelData[samples] *= gainTreeState;
    }
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Bit_depth3AudioProcessor)
};
