#ifndef jdAudioUtils_h
#define jdAudioUtils_h

//#include <stdio.h>
//#include <math.h>
#include "jd_smoothing.h"
#include <type_traits>
#include <typeinfo>


template<typename T>
using UnaryOp = T(*) (T);

template<typename F>
using FloatConversionFunc = UnaryOp<
typename std::enable_if<std::is_floating_point<F>::value, F>::type>;

template<typename F>
struct ReversibleConversion
{
    FloatConversionFunc<F> forward;
    FloatConversionFunc<F> backward;
};

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

template<typename FloatType>
FloatType hzmidi (FloatType hz)
{
    return 69. + 12 * std::log2(hz/440.);
}

template<typename FloatType>
FloatType midihz (FloatType midi)
{
    return 2 * (midi - 69.) / 12 * 440.;
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
