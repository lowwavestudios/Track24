#pragma once

#include "EnvelopeShaper.h"
#include <JuceHeader.h> 

#include <cmath>
#include <algorithm>

class Compressor
{
public:
    // callback for initializing with samplerate
    void prepareForPlayback(float samplerate, int numSamp, int numChannels)
    {
        m_EnvelopeShaper.prepareForPlayback(samplerate);

        numCh = numChannels;
        numS = numSamp;
    }
    
    void processBuffer(AudioSampleBuffer &inputBuffer)
    {
        sideChain.setSize(inputBuffer.getNumChannels(), inputBuffer.getNumSamples());
        float* w = sideChain.getWritePointer(0);

        int numsamples = inputBuffer.getNumSamples();

        if (numCh > 1)
        {
            for (int i = 0; i < numsamples; i++)
            {
                float L, R;
                
                int ch = 0;

                L = inputBuffer.getSample(ch, i);
                R = inputBuffer.getSample(ch + 1, i);
                

                float detectionSignal;
                if (fabs(R) == fmaxf(fabs(L), fabs(R)))
                { 
                    detectionSignal = R;
                }
                else { detectionSignal = L; }
         

                w[i] = detectionSignal;


            }

           

            for (int i = 0; i < numsamples; i++)
            {
                // apply envelope shaping to detection signal
                float detectionSignal = sideChain.getSample(0, i);

                m_EnvelopeShaper.processAudioSample(detectionSignal);

                // check if the detection signal exceeds the threshold
                detectionSignal = amplitudeToDecibel(detectionSignal);

                w[i] = detectionSignal;
            }

        }
        else 
        {
            for (int i = 0; i < numsamples; i++)
            {
                float detectionSignal;
                
                int ch = 0;
                    
                detectionSignal = inputBuffer.getSample(ch, i);

                w[i] = detectionSignal;

            }

            

            for (int i = 0; i < numsamples; i++)
            {
                // apply envelope shaping to detection signal
                float detectionSignal = sideChain.getSample(0, i);

                m_EnvelopeShaper.processAudioSample(detectionSignal);

                // check if the detection signal exceeds the threshold
                detectionSignal = amplitudeToDecibel(detectionSignal);

                w[i] = detectionSignal;
            }
        }
        

        for (int ch = 0; ch < numCh && ch < 2; ch++)
        {
            float* input = inputBuffer.getWritePointer(ch);
            
            for (int i = 0; i < numsamples; i++)
            {
                float sidechain = sideChain.getSample(0, i);
                
                //sc = amplitudeToDecibel(sc);

                if (sidechain > m_Threshold)
                {
                    // compute required gain reduction
                    float scale = 1.f - (1.f / m_Ratio);
                    float gain = scale * (m_Threshold - sidechain);

                    
                    gain = decibelToAmplitude(gain);
                    tick = gain;

                    input[i] *= gain;
                    
           
                }
            }
        }


        

        
    }
    
    float getTick()
    {
        
        return tick;
    }

    
    // setters for compression parameters
    void setThreshold(float threshold)
    {
        m_Threshold = threshold;
    }
    
    void setRatio(float ratio)
    {
        m_Ratio = ratio;
    }
    
    void setAttack(float attack)
    {
        m_EnvelopeShaper.setAttack(attack);
    }
    
    void setRelease(float release)
    {
        m_EnvelopeShaper.setRelease(release);
    }
    void setHold(float hold)
    {
        m_EnvelopeShaper.setHold(hold);
    }

    

private:
    // compressor private variables
    float m_Threshold;
    float m_Ratio;
    float tick = 1.f;

    EnvelopeShaper m_EnvelopeShaper;

    AudioSampleBuffer sideChain;
    int numS, numCh;
    
    const float BOUND_LOG = -96.f;
    const float BOUND_LIN = decibelToAmplitude(BOUND_LOG);
    
    // transform amplitude to decibel
    float amplitudeToDecibel(float amplitude)
    {
        amplitude = std::max(amplitude, BOUND_LIN);
        return 20.f * log10(amplitude);
    }
    
    // transform decibel to amplitude
    float decibelToAmplitude(float decibel)
    {
        return pow(10.f, decibel / 20.f);
    }
}; 