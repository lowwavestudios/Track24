#pragma once


#include <cmath>

class EnvelopeShaper
{
public:
    // callback for initializing with samplerate
    void prepareForPlayback(float samplerate)
    {
        m_Samplerate = samplerate;

        update();
    }

    // callback for audio processing
    void processAudioSample(float& sample)
    {
        if (sample > m_Envelope)
        {
            holdcount = 0.0;
            m_Envelope += m_Attack * (sample - m_Envelope);
        }
        else if (sample < m_Envelope)
        {

            if (holdcount < m_Hold)
            {
                m_Envelope;
                holdcount++;

            }
            else if (holdcount >= m_Hold)
            {
                m_Envelope += m_Release * (sample - m_Envelope);
            }

        }
        sample = m_Envelope;
    }

    // setters for compression parameters
    void setAttack(float attack)
    {
        m_AttackInMilliseconds = attack;
        update();
    }
    void setHold(float hold)
    {
        m_HoldInMilliseconds = hold;
        update();
    }

    void setRelease(float release)
    {
        m_ReleaseInMilliseconds = release;
        update();
    }

private:
    // envelope shaper private variables and functions
    float m_Envelope{ 0.0f };
    float m_Samplerate{ 44100 };
    float m_AttackInMilliseconds{ 0.0f };
    float m_HoldInMilliseconds{ 0.0f };
    float m_ReleaseInMilliseconds{ 0.0f };
    float m_Attack{ 0.0f };
    float m_Release{ 0.0f };
    double m_Hold{ 0.0 };
    double holdcount{ 0.0 };

    // update attack and release scaling factors
    void update()
    {
        m_Attack = calculate(m_AttackInMilliseconds);
        m_Release = calculate(m_ReleaseInMilliseconds);
        m_Hold = (m_HoldInMilliseconds * 0.001) * m_Samplerate;
    }

    // calculate scaling factor from a value in milliseconds
    float calculate(float time)
    {
        if (time <= 0.f || m_Samplerate <= 0.f)
        {
            return 1.f;
        }
        return 1.f - exp(-1.f / (time * 0.001f * m_Samplerate));
    }
};
