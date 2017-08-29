#ifndef jd_smoothing_h
#define jd_smoothing_h

#include <math.h>

template <typename FloatType>
class SmoothedValue {
public:
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


#endif /* jd_smoothing_h */
