//
//  env_follower_tmp.h
//  template_meta
//
//  Created by Jaiden Muschett on 28/08/2017.
//  Copyright © 2017 Jaiden Muschett. All rights reserved.
//

#ifndef env_follower_tmp_h
#define env_follower_tmp_h

#include <math.h>

template<typename F, class Derived>
class EnvelopeFollower {
public:
    //INIT
    void init(double sampleRate, int blockSize)
    {
        setSampleRate(sampleRate);
        m_envelopeOutput.resize(blockSize);
        calculateCoefficients();
    }
    
    void setSampleRate(double sampleRate)
    {
        m_sampleRate = sampleRate;
        calculateCoefficients();
    }
    //UPDATE
    void processBlock(const F* input, int numSamples)
    {
        for (int i = 0; i < numSamples; i++)
        {
            m_envelopeOutput[i] = processedSample(input[i]);
        }
    }
    void processBlock(const F* input, F* output, int numSamples)
    {
        for (int i = 0; i < numSamples; i++)
        {
            output[i] = processedSample(input[i]);
        }
    }
    
    F processedSample(const F input) {
        
        const F envInput = impl().calculate(input);
        
        if (m_envelope < envInput)
            m_envelope = (m_envelope * m_attackGainCoeff) + ((1 - m_attackGainCoeff) * envInput);
        else
            m_envelope = (m_envelope * m_releaseGainCoeff) + ((1 - m_releaseGainCoeff) * envInput);
        
        return m_envelope;
    }
    
    //INPUTS
    void setAttackTimeMS(F attackTimeMS) {
        m_attackTimeMS = attackTimeMS;
        calculateCoefficients();
    }
    void setReleaseTimeMS(F releaseTimeMS) {
        m_releaseTimeMS = releaseTimeMS;
        calculateCoefficients();
    }
    
    //OUTPUTS
    float envelope() { return m_envelope; }
    const float* envelopeData() { return m_envelopeOutput.data(); }
protected:
    F m_sampleRate;
private:
    //MAINTENANCE
    void calculateCoefficients()
    {
        m_attackGainCoeff = exp((-1.) / (m_attackTimeMS * m_sampleRate));
        m_releaseGainCoeff = exp((-1.) / (m_releaseTimeMS * m_sampleRate));
    }
    
    F m_attackTimeMS        {0.001};
    F m_releaseTimeMS       {0.001};
    F m_attackGainCoeff;
    F m_releaseGainCoeff;
    F m_envelope            {0.};
    std::vector<F> m_envelopeOutput;
    
    friend class EnvelopeFollower<F, Derived>;
    Derived& impl() { return *static_cast<Derived*>(this); };
};
//================================================================
template<typename F>
class RMSEnvelopeFollower : public EnvelopeFollower<F,RMSEnvelopeFollower<F>>
{
public:
    void init(double sampleRate, int blockSize)
    {
        super::init(sampleRate, blockSize);
        setMaxBufferSizeMS(1000.);
        setBufferSizeMS(100.);
        reset();
    }
    //UPDATE
    const F calculate (F input) {
        F currentSample = input;
        size_t oldestSampleIndex = m_circularBufferIndex + 1;
        
        //    if (oldestSampleIndex >= windowSize) oldestSampleIndex -= windowSize;
        oldestSampleIndex = oldestSampleIndex % m_windowSize;
        
        F oldestSample = m_circularBuffer[oldestSampleIndex];
        if (m_circularBufferIndex >= m_windowSize) m_circularBufferIndex = 0;
        
        F squaresIncrement = ((currentSample * currentSample) - (oldestSample * oldestSample));
        
        m_squaresSum += squaresIncrement;
        
        m_circularBuffer[m_circularBufferIndex] = currentSample;
        
        m_circularBufferIndex++;
        
        return (m_squaresSum > 0) ? sqrt(m_squaresSum / m_windowSize) : 0.0;
    }
    //INPUT
    void setMaxBufferSizeMS (F timeMS) {
        m_maxCircularBufferSize = (timeMS / 1000.) * super::m_sampleRate;
        m_circularBuffer.resize(m_maxCircularBufferSize);
    }
    void setBufferSizeMS (F timeMS)
    {
        size_t circBufferLength = (timeMS / 1000.) * super::m_sampleRate;
        setBufferSizeSamples(circBufferLength);
    }
    void setBufferSizeSamples (size_t lengthSamples)
    {
        m_windowSize = std::max(std::min(lengthSamples, m_maxCircularBufferSize), (size_t)1);
    }
protected:
    //MAINTENANCE
    void reset () {
        m_squaresSum = 0.;
        m_circularBufferIndex = 0;
        zeroCircularBuffer();
    }
    void zeroCircularBuffer (){
        for (auto &i : m_circularBuffer) i = 0.;
    }
private:
    F m_squaresSum {0.};
    std::vector<F> m_circularBuffer;
    size_t m_circularBufferIndex { 0 };
    size_t m_windowSize;
    size_t m_maxCircularBufferSize;/* do later*/
    
    using super = EnvelopeFollower<F, RMSEnvelopeFollower<F>>;
};
//================================================================
template<typename F>
class PeakEnvelopeFollower : public EnvelopeFollower<F,PeakEnvelopeFollower<F>> {
public:
    const F calculate (const F input) {
        return fabs(input);
    }
};


#endif /* env_follower_tmp_h */
