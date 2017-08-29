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

inline const double pi () { return acos(-1.); }
    

#endif /* jdAudioUtils_h */
