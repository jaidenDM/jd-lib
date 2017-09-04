//
//  gate_tmp.h
//  template_meta
//
//  Created by Jaiden Muschett on 28/08/2017.
//  Copyright © 2017 Jaiden Muschett. All rights reserved.
//

#ifndef gate_tmp_h
#define gate_tmp_h


//#include <iostream>
//#include <vector>
//#include <memory.h>
//#include <type_traits>

template<typename F, class Derived, class Detector>
class Gate {
public:
    
    friend class Gate<F, Derived, Detector>;
    friend Derived;
    
    Derived& impl() { return *static_cast<Derived*>(this); }
    //INITIALISATION
    void init (double sampleRate, int blockSize)
    {
        m_detector.init(sampleRate, blockSize);
        const int numThresholds = Derived::NUM_THRESHOLDS;
        thresholds.resize(numThresholds);
        isAbove.resize(numThresholds);
//        shouldTriggerOnToAbove.resize(numThresholds);
//        shouldTriggerOnToBelow.resize(numThresholds);
//        std::fill(isAbove.begin(), isAbove.end(), false);
        for (int i = 0; i < numThresholds; i++) isAbove[i] = false;
    }
    
    //PROCESSING
    void processBlock(const F* input, F* output, int numSamples) {
        for (int i = 0; i < numSamples; i++)
        {
            const F inputSample = input[i];
            const F detectorSample = m_detector.calculate(inputSample);
            impl().checkThreshold(detectorSample, i);
            output[i] = impl().processedSample(inputSample, detectorSample);
        }
    }
    void processBlock(const F* input, int numSamples) {
        for (int i = 0; i < numSamples; i++)
        {
            const F inputSample = input[i];
            const F detectorSample = m_detector.calculate(inputSample);
            impl().checkThreshold(detectorSample, i);
        }
    }
    
    F processSample(F input)
    {
        const F inputSample = input;
        const F detectorSample = m_detector.calculate(inputSample);
        impl().checkThreshold(detectorSample, 0);
        return impl().processedSample(inputSample, detectorSample);
    }

    //PARAMETER MODIFICATION
    void setRMSWindowSizeMS(F windowSizeMS)
    {
        m_detector.setBufferSizeMS(windowSizeMS);
    }
    
    template<typename T>
    void setThresholds(T t)
    {
        thresholds[m_thresholds_i] = t;
        m_thresholds_i = 0;
    }
    template<typename T,  typename ... ThresholdValues>
    void setThresholds(T t, ThresholdValues... thresholdValues)
    {
        static_assert(Derived::NUM_THRESHOLDS > (sizeof...(ThresholdValues)), "Too many Args");
        thresholds[m_thresholds_i++] = t;
        setThresholds(thresholdValues...);
    }
    //""
    const Detector& detector() { return m_detector; }
    //protected:
    std::vector<F> thresholds;
    int m_thresholds_i = 0;
    std::vector<bool> isAbove;
//    std::vector<char> shouldTriggerOnToAbove;
//    std::vector<char> shouldTriggerOnToBelow;
    
    Detector m_detector;
};

template<typename F, class Detector>
class GateSingle : public Gate<F, GateSingle<F,Detector>, Detector>
{
public:
    enum Thresholds {
        THRESH,
        NUM_THRESHOLDS
    };
protected:
    void checkThreshold(float detectorSignal, int sampleIndex) {
        std::cout << "h" << std::endl;
        
        if (detectorSignal > super::thresholds[THRESH])
        {
            if (!super::isAbove[THRESH]) {
                //HERE
            }
            super::isAbove[THRESH] = true;
        } else {
            if (super::isAbove[THRESH]) {
                //HERE
            }
            super::isAbove[THRESH] = false;
        }
    }
private:
    using super = Gate<F, GateSingle<F, Detector>, Detector>;
    friend Gate<F, GateSingle<F, Detector>, Detector>;
};

template<typename F, class Detector>
class GateDouble : public Gate<F, GateDouble<F, Detector>, Detector>
{
public:
    virtual ~GateDouble() = default;
    enum Thresholds {
        LOWER,
        UPPER,
        NUM_THRESHOLDS
    };
    
    F processedSample (F inputSample, F detectorSample)
    {
        return detectorSample;
    }
    
    void setLowerThreshold(F threshold){
        super::thresholds[LOWER] = threshold;
    };
    void setUpperThreshold(F threshold){
        super::thresholds[UPPER] = threshold;
    };
    
    bool isWithinRange ()
    {
        return (super::isAbove[LOWER] && !super::isAbove[UPPER]);
    }
    
protected:
    void checkThreshold(float detectorSignal, int sampleIndex) {

        for (int i = 0; i < NUM_THRESHOLDS; i++)
        {
            if (detectorSignal > super::thresholds[i])
            {
                if (!super::isAbove[i]) {
                    if (i == LOWER) {
                        onExitFromBelow(i);
                    }
                    if (i == UPPER) {
                        onEntryToAbove(i);
                    }
                }
                super::isAbove[i] = true;
            } else
            {
                if (super::isAbove[i]) {
                    if (i == LOWER) {
                        onEntryToBelow(i);
                    }
                    if (i == UPPER) {
                        onExitFromAbove(i);
                    }
                }
                super::isAbove[i] = false;
            }
        }
    }
    
    virtual void onExitFromBelow(int sampleIndex) {
//        std::cout << "onExitFromBelow" << std::endl;
    }
    virtual void onEntryToBelow(int sampleIndex) {
//        std::cout << "onEntryToBelow" << std::endl;
    }
    virtual void onExitFromAbove(int sampleIndex) {
//        std::cout << "onExitFromAbove" << std::endl;
    }
    virtual void onEntryToAbove(int sampleIndex) {
//        std::cout << "onEntryToAbove" << std::endl;
    }
    
private:
    using super = Gate<F, GateDouble<F, Detector>, Detector>;
    friend Gate<F, GateDouble<F, Detector>, Detector>;
};
#endif /* gate_tmp_h */
