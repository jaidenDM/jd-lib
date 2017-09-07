#ifndef gate_tmp_h
#define gate_tmp_h

//#include <iostream>
//#include <vector>
//#include <memory.h>
//#include <type_traits>

template<typename F, class Derived>
class Gate {
public:
    
    friend class Gate<F, Derived>;
    friend Derived;
    
    Derived& impl() { return *static_cast<Derived*>(this); }
    //INITIALISATION
    void init (double sampleRate, int blockSize)
    {
        const int numThresholds = Derived::NUM_THRESHOLDS;
        static_assert(numThresholds > 0,"no_thresholds");
        thresholds.resize(numThresholds);
        isAbove.resize(numThresholds);
        outputs.resize(blockSize);
        for (int i = 0; i < numThresholds; i++) isAbove[i] = false;
    }
    
    //PROCESSING
    void processBlock(const F* input, int numSamples) {
        for (int i = 0; i < numSamples; i++)
        {
            const F inputSample = input[i];
            outputs[i] = checkThreshold(inputSample);
        }
    }
    
    int checkThreshold(float detectorSignal) {
        
        int code = -1;
        
        for (int i = 0; i < Derived::NUM_THRESHOLDS; i++)
        {
            if (detectorSignal > thresholds[i])
            {
                if (!isAbove[i])
                    code = i;
                isAbove[i] = true;
            } else {
                if (isAbove[i])
                    code = i + 1;
                isAbove[i] = false;
            }
        }
        
        return code;
    }

    //PARAMETER MODIFICATION
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
//    const Detector& detector() { return m_detector; }
    //protected:
    std::vector<F> thresholds;
    int m_thresholds_i = 0;
    std::vector<char> isAbove;
    std::vector<int> outputs;
};

//template<typename F>
//class GateSingle : public Gate<F, GateSingle<F>>
//{
//public:
//    enum Thresholds {
//        THRESH,
//        NUM_THRESHOLDS
//    };
//protected:
//    void checkThreshold(float detectorSignal, int sampleIndex) {
//        std::cout << "h" << std::endl;
//        
//        if (detectorSignal > super::thresholds[THRESH])
//        {
//            if (!super::isAbove[THRESH]) {
//                //HERE
//            }
//            super::isAbove[THRESH] = true;
//        } else {
//            if (super::isAbove[THRESH]) {
//                //HERE
//            }
//            super::isAbove[THRESH] = false;
//        }
//    }
//private:
//    using super = Gate<F, GateSingle<F>>;
//    friend Gate<F, GateSingle<F>>;
//};

template<typename F>
class GateDouble : public Gate<F, GateDouble<F>>
{
public:
    virtual ~GateDouble() = default;
    
    enum Thresholds {
        LOWER,
        UPPER,
        NUM_THRESHOLDS
    };

    void setLowerThreshold(F threshold)
    {
        super::thresholds[LOWER] = threshold;
    }
    
    void setUpperThreshold(F threshold)
    {
        super::thresholds[UPPER] = threshold;
    }
    
    bool isWithinRange () const
    {
        return (super::isAbove[LOWER] && !super::isAbove[UPPER]);
    }
    
    bool isWithinRange (int lower, int upper)
    {
        return (super::isAbove[lower] && !super::isAbove[upper]);
    }
    
protected:
//    int checkThreshold(float detectorSignal) {
//
//        int code = -1;
//        
//        for (int i = 0; i < NUM_THRESHOLDS; i++)
//        {
//            if (detectorSignal > super::thresholds[i])
//            {
//                if (!super::isAbove[i])
//                    code = 0 * i;
//                super::isAbove[i] = true;
//            } else {
//                if (super::isAbove[i])
//                    code = 0 * i + 1;
//                super::isAbove[i] = false;
//            }
//        }
//        
//        return code;
//    }
    
//    virtual void onExitFromBelow(int sampleIndex) {
////        std::cout << "onExitFromBelow" << std::endl;
//    }
//    virtual void onEntryToBelow(int sampleIndex) {
////        std::cout << "onEntryToBelow" << std::endl;
//    }
//    virtual void onExitFromAbove(int sampleIndex) {
////        std::cout << "onExitFromAbove" << std::endl;
//    }
//    virtual void onEntryToAbove(int sampleIndex) {
////        std::cout << "onEntryToAbove" << std::endl;
//    }
    
private:
    using super = Gate<F, GateDouble<F>>;
    friend Gate<F, GateDouble<F>>;
};
#endif /* gate_tmp_h */
