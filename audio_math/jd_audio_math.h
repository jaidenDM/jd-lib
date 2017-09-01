#ifndef jdAudioUtils_h
#define jdAudioUtils_h

//#include <stdio.h>
//#include <math.h>
#include "jd_smoothing.h"

template<typename F>
static inline F ampdb (F amp)
{
    return 20. * log10(amp);
}

template<typename F>
static inline F dbamp (F db)
{
    return powf(10., (db/20.));
}

template<typename F>
static inline F linlin(F val, F oldMin, F oldMax, F newMin, F newMax)
{
    return (val - oldMin) * (newMax - newMin)/(oldMax - oldMin) + newMin;
}
template<typename F>
static inline F linlin(F val, F newMin, F newMax)
{
    return linlin(val, 0., 1., newMin, newMax);
}

inline const double pi () { return acos(-1.); }
    

#endif /* jdAudioUtils_h */
