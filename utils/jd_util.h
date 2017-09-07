//

#ifndef jd_util_h
#define jd_util_h

#include "../jdHeader.h"

template<typename F>
struct Range
{
//    Range() = default;
//    template< class Array>
//    explicit Range (Array arr)
//    {
//        auto pair = std::minmax_element(std::begin(arr), std::end(arr));
//        lower = *pair.first();
//        upper = *pair.second();
//    }
    
    template<class Array>
    static Range<F> fromArray (Array arr)
    {
        Range<F> newRange {};
        auto pair = std::minmax_element(std::begin(arr), std::end(arr));
        newRange.lower = arr[*pair.first];
        newRange.upper = arr[*pair.second];
        return newRange;
    }


    void setLimits(F newLower, F newUpper)
    {
        lower = newLower;
        upper = newUpper;
    }
    
    F normalise (F input) {
        return jd::linlin(input, lower, upper, (F)0., (F)1.);
    }
    
    F relative (F input, F otherLower, F otherUpper)
    {
        return jd::linlin(input, otherLower, otherUpper, lower, upper);
    }
    
    F relative (F input, Range otherRange)
    {
        return relativeTo (input, otherRange.lower, otherRange.upper);
    }

    F lower = 0.;
    F upper = 1.;
    //make moveable/adaptive
    //time since last moved

};


template<typename FloatType>
inline FloatType clip (FloatType value, FloatType lower, FloatType upper)
{
    return std::max(std::min(value, upper), lower);
}
template<typename FloatType>
inline FloatType clip (FloatType value, Range<FloatType> range)
{
    return clip(value, range.upper, range.lower);
}

#endif /* jd_util_h */
