/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
//#include "PluginEditor.h"
#include "Compressor.h"
#define GAIN_ID "gain"
#define GAIN_NAME "Gain"
#define OS_ID "oversample"
#define OS_NAME "Oversample"
#define TONE_ID "tone"
#define TONE_NAME "Tone"
#define HPF_ID "hpf"
#define HPF_NAME "HPF"
#define LPF_ID "lpf"
#define LPF_NAME "LPF"
#define ORDER_ID "texture"
#define ORDER_NAME "Texture"
#define DW_ID "drywet"
#define DW_NAME "DryWet"
#define OUT_ID "output"
#define OUT_NAME "Output"

//==============================================================================
/**
*/
class Track24AudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    Track24AudioProcessor();
    ~Track24AudioProcessor() override;

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

    AudioProcessorValueTreeState parameters;
    AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    float mGain{ 1.f }, mSamplerate{ 44100.f }, mNumSamples{ 512.f }, mMeterSampleL{ 0.f }, mMeterSampleR{ 0.f }, mHPF{ 20.f }, mLPF{ 20000.f }, mOrder{ 0.5f }, mDryWet{ 1.f }, mOutput{ 0.f };
    bool mOs{ true };
    int mTone{ 1 }, writePosition{ 0 }, mLatency{ 0 }, delayedBufferLength{ 512 };
   
    EnvelopeShaper meterEnvelopeL, meterEnvelopeR;
    

private:

    float Clipper(float sample, float order);
    void setParams();
    std::unique_ptr<juce::dsp::Oversampling<float>> mOversample;
    AudioSampleBuffer DryBuffer;
    
    dsp::ProcessSpec spec;
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> hiMidBoost;
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> loMidCut; 
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> airBoost;
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> highShelf;
    dsp::ProcessorDuplicator<dsp::IIR::Filter<float>, dsp::IIR::Coefficients<float>> highPass;
    dsp::DelayLine<float, dsp::DelayLineInterpolationTypes::None> delay{ 44100 };
    dsp::LinkwitzRileyFilter<float> linkwitzHPF;
    dsp::ProcessorDuplicator<dsp::StateVariableFilter::Filter<float>, dsp::StateVariableFilter::Parameters<float>> HPFFilter;
    dsp::ProcessorDuplicator<dsp::StateVariableFilter::Filter<float>, dsp::StateVariableFilter::Parameters<float>> LPFFilter;


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Track24AudioProcessor)
};
