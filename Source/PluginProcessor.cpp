/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
Track24AudioProcessor::Track24AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), parameters(*this, nullptr, "PARAMETER", createParameterLayout())
#endif
{
}

Track24AudioProcessor::~Track24AudioProcessor()
{
}

AudioProcessorValueTreeState::ParameterLayout Track24AudioProcessor::createParameterLayout()
{
    std::vector <std::unique_ptr <RangedAudioParameter>> params;

    auto gainParam = std::make_unique <AudioParameterFloat>(GAIN_ID, GAIN_NAME, -12.0f, 24.0f, 0.f);
    auto overParam = std::make_unique <AudioParameterBool>(OS_ID, OS_NAME, true);
    auto toneParam = std::make_unique <AudioParameterInt>(TONE_ID, TONE_NAME, 1, 4, 3);
    auto HPFParam = std::make_unique <AudioParameterInt>(HPF_ID, HPF_NAME, 20, 400, 20);
    auto LPFParam = std::make_unique <AudioParameterInt>(LPF_ID, LPF_NAME, 5000, 20000, 20000);
    auto orderParam = std::make_unique <AudioParameterInt>(ORDER_ID, ORDER_NAME, 0, 100, 50);
    auto outputParam = std::make_unique <AudioParameterFloat>(OUT_ID, OUT_NAME, -12.f, 12.f, 0.f);
    auto drywetParam = std::make_unique <AudioParameterInt>(DW_ID, DW_NAME, 0, 100, 100);
   
    params.push_back(std::move(gainParam));
    params.push_back(std::move(overParam));
    params.push_back(std::move(toneParam));
    params.push_back(std::move(HPFParam));
    params.push_back(std::move(LPFParam));
    params.push_back(std::move(orderParam));
    params.push_back(std::move(outputParam));
    params.push_back(std::move(drywetParam));

    return { params.begin(), params.end() };
}


//==============================================================================
const juce::String Track24AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool Track24AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool Track24AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool Track24AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double Track24AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int Track24AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int Track24AudioProcessor::getCurrentProgram()
{
    return 0;
}

void Track24AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String Track24AudioProcessor::getProgramName (int index)
{
    return {};
}

void Track24AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

void Track24AudioProcessor::setParams()
{
    mGain = (*parameters.getRawParameterValue(GAIN_ID));
    mOs = (*parameters.getRawParameterValue(OS_ID));
    mTone = *parameters.getRawParameterValue(TONE_ID);
    mHPF = *parameters.getRawParameterValue(HPF_ID);
    mLPF = *parameters.getRawParameterValue(LPF_ID);
    mOrder = *parameters.getRawParameterValue(ORDER_ID) / 100;
    mDryWet = *parameters.getRawParameterValue(DW_ID) / 100;
    mOutput = *parameters.getRawParameterValue(OUT_ID);

    HPFFilter.state->type = dsp::StateVariableFilter::Parameters<float>::Type::highPass;
    HPFFilter.state->setCutOffFrequency(mSamplerate, mHPF, 0.707f);

    LPFFilter.state->type = dsp::StateVariableFilter::Parameters<float>::Type::lowPass;
    LPFFilter.state->setCutOffFrequency(mSamplerate, mLPF, 0.707f);

    switch (mTone)
    {
    case 2:
        *hiMidBoost.state = *dsp::IIR::Coefficients<float>::makePeakFilter(mSamplerate, 2000.f, 0.35f, Decibels::decibelsToGain(0.2f));
        *loMidCut.state = *dsp::IIR::Coefficients<float>::makePeakFilter(mSamplerate, 120.f, 1.1f, Decibels::decibelsToGain(-0.2f));
        *airBoost.state = *dsp::IIR::Coefficients<float>::makePeakFilter(mSamplerate, 15000.f, 0.8f, Decibels::decibelsToGain(0.5f));
        *highShelf.state = *dsp::IIR::Coefficients<float>::makeHighShelf(mSamplerate, 10000.f, 0.707f, Decibels::decibelsToGain(-1.f));
        *highPass.state = *dsp::IIR::Coefficients<float>::makeHighPass(mSamplerate, 35.f, 0.85f);
        break;
    case 3:

        linkwitzHPF.setCutoffFrequency(25.f);
        *highShelf.state = *dsp::IIR::Coefficients<float>::makeHighShelf(mSamplerate, 8000.f, 0.4f, Decibels::decibelsToGain(-2.f));
        break;
    case 4:
        linkwitzHPF.setCutoffFrequency(30.f);
        *highShelf.state = *dsp::IIR::Coefficients<float>::makeHighShelf(mSamplerate, 8000.f, 0.4f, Decibels::decibelsToGain(-4.f));
        break;
    };

}
//==============================================================================
void Track24AudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    mSamplerate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    spec.sampleRate = sampleRate;

    hiMidBoost.reset();
    hiMidBoost.prepare(spec);
    loMidCut.reset();
    loMidCut.prepare(spec);
    airBoost.reset();
    airBoost.prepare(spec);
    highShelf.reset();
    highShelf.prepare(spec);
    highPass.reset();
    highPass.prepare(spec);
    HPFFilter.reset();
    HPFFilter.prepare(spec);
    LPFFilter.reset();
    LPFFilter.prepare(spec);
    linkwitzHPF.reset();
    linkwitzHPF.prepare(spec);
    linkwitzHPF.setType(dsp::LinkwitzRileyFilterType::highpass);
    

    meterEnvelopeL.prepareForPlayback(sampleRate);
    meterEnvelopeR.prepareForPlayback(sampleRate);

    meterEnvelopeL.setAttack(30);
    meterEnvelopeL.setHold(10);
    meterEnvelopeL.setRelease(300);
    meterEnvelopeR.setAttack(30);
    meterEnvelopeR.setHold(10);
    meterEnvelopeR.setRelease(300);

    mOversample.reset(new juce::dsp::Oversampling<float>(getTotalNumOutputChannels(), 2, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple, true, true));
    mOversample->initProcessing(static_cast<size_t>(samplesPerBlock));

    delay.reset();
    delay.prepare(spec);

    setParams();
}

void Track24AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool Track24AudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
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

void Track24AudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    setParams();

    mOversample->setUsingIntegerLatency(true);
    mLatency = static_cast <int>(mOversample->getLatencyInSamples());

    if (mOs == true)
    {
        setLatencySamples(mLatency);
    }
    else
    {
        setLatencySamples(0);
    }

    buffer.applyGain(Decibels::decibelsToGain(mGain));

    dsp::AudioBlock<float> mBlock(buffer);
    //HPF & LPF Filters
    if (mHPF > 20) HPFFilter.process(dsp::ProcessContextReplacing<float>(mBlock));
    if (mLPF < 20000) LPFFilter.process(dsp::ProcessContextReplacing<float>(mBlock));

    //prepare dry buffer
    delay.setDelay(mLatency);
    DryBuffer.makeCopyOf(buffer);
    //Buffer to meter
    if (buffer.getNumChannels() == 1)
    {
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float sL = abs(buffer.getSample(0, i));
            meterEnvelopeL.processAudioSample(sL);
            mMeterSampleL = jlimit<float>(0, 1, sL);
            mMeterSampleR = mMeterSampleL;
        }
    }
    else if (buffer.getNumChannels() > 1)
    {
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float sL = abs(buffer.getSample(0, i));
            meterEnvelopeL.processAudioSample(sL);

            float sR = abs(buffer.getSample(1, i));
            meterEnvelopeR.processAudioSample(sR);

            mMeterSampleL = jlimit<float>(0, 1, sL * 4);
            mMeterSampleR = jlimit<float>(0, 1, sR * 4);
        }
    }

    

    if (mOs == true)
    {
        //prepare for oversampling
        dsp::AudioBlock<float> processBlock(buffer);
        dsp::AudioBlock<float> highSampleRateBlock = mOversample->processSamplesUp(processBlock);

        //Clipper
        for (int ch = 0; ch < highSampleRateBlock.getNumChannels(); ch++)
        {
            float* w = highSampleRateBlock.getChannelPointer(ch);
            for (int i = 0; i < highSampleRateBlock.getNumSamples(); i++)
            {
                w[i] = Clipper(highSampleRateBlock.getSample(ch, i), mOrder);
            }
        }

        mOversample->processSamplesDown(processBlock);

        //delay dry buffer
        for (int channel = 0; channel < totalNumInputChannels; ++channel)
        {
            auto* data = DryBuffer.getWritePointer(channel);

            for (int i = 0; i < buffer.getNumSamples(); i++)
            {
                delay.pushSample(channel, data[i]);
                data[i] = delay.popSample(channel, mLatency);
            }
        }
    }
    else
    {
        //Clipper
        for (int ch = 0; ch < buffer.getNumChannels(); ch++)
        {
            float* w = buffer.getWritePointer(ch);
            for (int i = 0; i < buffer.getNumSamples(); i++)
            {
                w[i] = Clipper(buffer.getSample(ch, i), mOrder);
            }
        }
    }
    
    //TONE
    
    switch (mTone)
    {
    case 1:
        break;
    case 2:
        hiMidBoost.process(dsp::ProcessContextReplacing<float>(mBlock));
        loMidCut.process(dsp::ProcessContextReplacing<float>(mBlock));
        airBoost.process(dsp::ProcessContextReplacing<float>(mBlock));
        highPass.process(dsp::ProcessContextReplacing<float>(mBlock));
        highShelf.process(dsp::ProcessContextReplacing<float>(mBlock));
        break;
    case 3:
        linkwitzHPF.process(dsp::ProcessContextReplacing<float>(mBlock));
        highShelf.process(dsp::ProcessContextReplacing<float>(mBlock));
        break;
    case 4:
        linkwitzHPF.process(dsp::ProcessContextReplacing<float>(mBlock));
        highShelf.process(dsp::ProcessContextReplacing<float>(mBlock));
        break;
    };


    //Gain output
    buffer.applyGain(Decibels::decibelsToGain(mOutput));

    //Dry Wet
    for (int ch = 0; ch < buffer.getNumChannels(); ch++)
    {
        float* w = buffer.getWritePointer(ch);
        for (int i = 0; i < buffer.getNumSamples(); i++)
        {
            float dryS = DryBuffer.getSample(ch, i);
            float wetS = buffer.getSample(ch, i);

            w[i] = (wetS * mDryWet) + (dryS * ((mDryWet * -1) + 1));
        }
    }

    buffer.applyGain(Decibels::decibelsToGain(-mGain));
  
}

float Track24AudioProcessor::Clipper(float sample, float order)
{
    float s = sample;
    s *= Decibels::decibelsToGain(-4.f);

    s = jlimit<float>(-1, 1, s);
    float s2 = s, s3 = s;
    double pi = MathConstants<double>::pi;
    double hist = pow(fabs(s), 0.25);

    //2nd order
    {
        if (s <= 0)
        {
            if (hist > 0.0) s2 = (sin(s * hist * pi * 0.5) / hist);
        }
        else
        {
            if (hist > 0.0) s2 = (s * hist * pi * 0.5) / hist;
        }
    }

    //3rd oder
    {
        if (hist > 0.0) s3 = (sin(s * hist * pi * 0.5) / hist);
    }

    s = (s2 * order) + (s3 * ((order * -1) + 1));

    return s;
}
      
//==============================================================================
bool Track24AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* Track24AudioProcessor::createEditor()
{
    return new Track24AudioProcessorEditor(*this);
}

//==============================================================================
void Track24AudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    MemoryOutputStream stream(destData, false);
    parameters.state.writeToStream(stream);
}

void Track24AudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    ValueTree tree = ValueTree::readFromData(data, size_t(sizeInBytes));
    if (tree.isValid())
        parameters.state = tree;
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new Track24AudioProcessor();
}