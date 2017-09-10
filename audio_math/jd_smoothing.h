#ifndef jd_smoothing_h
#define jd_smoothing_h

#include <math.h>
#include <atomic>
#include "../audio_math/jd_audio_math.h"

template <typename FloatType>
struct OneZero {
    FloatType operator () (FloatType current, FloatType coeff = 0.99)
    {
        FloatType output = (::fabs(current - prev) < 0.0000001)
        ? current
        : prev * coeff + (1. -  coeff) * current;
        prev = output;
        return  output;
    }
    FloatType prev = 0.;
};
//========================================================================
/* Based on timur dumlers lock free audio code talk*/
template <typename FloatType>
class SmoothedValue {
    
    enum SmoothingMode {
        Stepwise,
        Recursive
    };
    
    FloatType m_value{0};
//    std::atomic<FloatType> m_newTarget {0};
    FloatType m_newTarget {0};
    FloatType m_target;
    FloatType m_starting {0};
    int m_remaining;
    int m_amount;
    FloatType m_increment;
    FloatType m_curve {1};
    SmoothingMode mode { Stepwise  };
    
    double m_sampleRate {0.};
    
public:
    
    SmoothedValue() = default;
//    SmoothedValue(SmoothedValue& ) = default;
//    SmoothedValue  (SmoothedValue&& other) {
//        setSampleRate(other.m_sampleRate);
//        setDurationS(0.01, other.m_curve);
//        setValue(other.m_target);
//    }
    
    void setSampleRate(double sampleRate) {
        m_sampleRate = sampleRate;
    }
    
    void setDurationS(double duration, FloatType curve)
    {
        m_curve = curve;
        m_amount = (int)floor(duration * m_sampleRate);
    }
    
    void setTarget(FloatType target)
    {
//        m_newTarget.store(target);
        m_newTarget = target;
    }
    
    void updateTarget() {
        auto newTarget = m_newTarget;
        if (newTarget != m_target) {
            m_starting = m_value;
            m_target = newTarget;
            m_remaining = m_amount;
            if (m_remaining <= 0)
                m_value = m_target;
            else
                m_increment = (m_target - m_value) / (FloatType) m_remaining;
        }
    }
    
    FloatType currentValue()
    {
        return m_value;
    }
    /* call once per audio rate cycle*/
    FloatType nextValue()
    {
        if (m_remaining-- <= 0)
            return m_target;
        
        FloatType increment;
        switch (mode) {
            case Stepwise:
                increment = m_increment;
                break;
            case Recursive:
                auto distance = m_target - m_value;
                increment = distance / 1.05;
                break;
        };
        
        m_value += increment;

        return m_value;
    }
    
};

#endif /* jd_smoothing_h */
