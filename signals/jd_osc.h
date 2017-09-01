//
//  jdOsc.h
//  jdGating_tmp
//
//  Created by Jaiden Muschett on 29/08/2017.
//
//

#ifndef jdOsc_h
#define jdOsc_h
#include "jd_audio_math.h"

template <typename F, class Derived>
class Osc {
public:
    Derived& impl() { return *static_cast<Derived*>(this); };
    //INIT
    void init (F sampleRate)
    {
        m_sampleRate = sampleRate;
        calculateIncrement();
    }
    //UPDATE
    void processBlock (F* output, int numSamples)
    {
        for (int i = 0; i < numSamples; i++) {
//            F offsetPhase = m_phase + m_phaseOffset;
            F phase = m_phase;
            if ((m_phase + m_phaseIncrement) >= 1.)
            {
                phase = 1.;
                m_phase = 0.;
            }
            output[i] = impl().processedSample(phase) * m_amplitude;
            m_phase += m_phaseIncrement;
        }
    }
    
    void setFrequency(const F frequency) {
        m_frequency = frequency;
        calculateIncrement();
    }
    void setAmplitude(const F amplitude) {
        m_amplitude = amplitude;
    }
    void setPhaseOffset(const F phaseOffset) { m_phaseOffset = phaseOffset; }
    
private:
    void calculateIncrement() { m_phaseIncrement = (m_frequency / m_sampleRate);}
    
    F m_sampleRate;
    F m_phase = 0;
    F m_phaseIncrement;
    F m_phaseOffset = 0.;
    F m_frequency = 100.;
    F m_amplitude = 0.5 ;
};

template <typename F>
class Phasor : public Osc<F, Phasor<F>>
{
public:
    F processedSample (const F phase) {
        return phase;
    }
};

template <typename F>
class SinOsc : public Osc<F, SinOsc<F>>
{
public:
    F processedSample (const F phase)
    {
        return sin(2. * jd::pi() * phase);
    }
};

template <typename F>
class TriOsc : public Osc<F, TriOsc<F>>
{
public:
    F processedSample (const F phase) {
        return ((phase < 0.5) ? (2. * phase) : (1. - 2. * ( phase - 0.5))) * 2. - 1.;
    }
};


template <typename F>
class Impulse : public Osc<F, Impulse<F>>
{
public:
    F processedSample (const F phase) {
        return (phase > 0.99999) ? 1. : 0.;
    }
};

#endif /* jdOsc_h */
