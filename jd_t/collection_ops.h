//
//  collection_ops.h
//  jd_CMatrix
//
//  Created by Jaiden Muschett on 04/09/2017.
//
//

#ifndef collection_ops_h
#define collection_ops_h

#include <tuple>
#include <typeinfo>
#include <type_traits>
#include <typeinfo>
#include <vector>
#include <map>
//#include <list>
#include <utility>
//#include <array>
//#include <set>

template<class C>
C normalised_list (C& list)
{
    auto normalisedList = C (list);
    using F = typename C::value_type;
    
    auto front = std::distance(list.begin(), std::min_element(list.begin(), list.end()));
    auto back = std::distance(list.begin(), std::max_element(list.begin(), list.end()));
    
    for (auto& value : normalisedList)
        value = linlin(value, list.at(front), list.at(back), F {0.}, F {1.});
    return normalisedList;
}

template<class C, typename F = typename C::value_type>
C   interpolated_list (C& list,
                       F newMin,
                       F newMax)
{
    auto interpolatedList = normalised_list(list);
    for (auto& value : interpolatedList)
        value = value * (newMax - newMin) + newMin;
    return interpolatedList;
}

#endif /* collection_ops_h */
