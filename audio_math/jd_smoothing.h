#ifndef jd_smoothing_h
#define jd_smoothing_h

#include <math.h>
#include <atomic>
#include "jd_audio_math.h"

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
    
    FloatType m_value{0};
    std::atomic<FloatType> m_target {0};
    FloatType m_starting {0};
    int m_remaining;
    int m_amount;
    FloatType m_increment;
    
    FloatType m_curve;
    
    double m_sampleRate {0.};
    
public:
    
    void setSampleRate(double sampleRate) {
        m_sampleRate = sampleRate;
    }
    
    void setDurationS(double duration, FloatType curve)
    {
        m_curve = curve;
        m_amount = (int)floor(duration * m_sampleRate);
    }
    
    void setValue(FloatType target)
    {
        m_target.store(target);
        
    }
    
    void updateTarget() {
        auto newTarget = m_target.load();
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
    
    FloatType nextValue()
    {
        updateTarget();
        if (m_remaining-- <= 0)
            return m_target;
        m_value += m_increment;

        auto norm = linlin(m_value,
                               m_starting,
                               m_target,
                               FloatType {0},
                               FloatType {1});
        return m_value * ::powf(norm, m_curve);
    }
    
};

#endif /* jd_smoothing_h */
