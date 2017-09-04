#ifndef arg_info_h
#define arg_info_h

#include <type_traits>
#include <typeinfo>

template<std::size_t I = 0, typename T, typename... Tp>
typename std::enable_if<(sizeof...(Tp)==0), std::size_t>::type
num_floats_args()
{
    return std::is_floating_point<T>() ? I + 1 : I;
}

template<std::size_t I = 0, typename T, typename... Tp>
typename std::enable_if<(sizeof...(Tp) > 0), std::size_t>::type
num_floats_args()
{
    return std::is_floating_point<T>() ?
    num_floats_args<I + 1, Tp...>() :
    num_floats_args<I, Tp...>();
}

//----------------------------------------------------
template<std::size_t I = 0, class Func, typename... Tp>
typename std::enable_if<(sizeof...(Tp)==I), void>::type
if_float_index(Func&& f)
{ }

template<std::size_t I = 0, class Func, typename... Tp>
typename std::enable_if<(I < sizeof...(Tp)), void>::type
if_float_index(Func&& func)
{
    auto t = std::tuple<Tp...> ();
    auto g = std::get<I>(t);
    auto b = std::is_floating_point<decltype(g)>::value;
    func(I,b);
    if_float_index<I + 1, Func, Tp...>(std::forward<Func>(func));
}

struct index_collector {
    void operator() (std::size_t index, bool b)
    {
        if (b) indices.push_back(index);
    };
    std::vector<size_t> indices {};
};
//-----------------------------------------------------------
template<class ContentsType, class ContentsIndexType = size_t>
struct tuple_element_collector {
    tuple_element_collector (size_t numValues = 0) {
        if (numValues > 0)
        {
            values.reserve(numValues);
            indices.reserve(numValues);
            relative_indices.reserve(numValues);
        }
    }
    template<typename ValueType, typename IndexType>
    typename std::enable_if<std::is_floating_point<ValueType>::value>::type
    operator() (ValueType& value, IndexType i, IndexType j)
    {
        values.push_back(value);
        indices.push_back(i);
        relative_indices.push_back(j);
    };
    
    template<typename ValueType, typename IndexType>
    typename std::enable_if<!std::is_floating_point<ValueType>::value>::type
    operator() (ValueType& value, IndexType i, IndexType j)
    {
        
    };
    std::vector<ContentsType> values;
    std::vector<ContentsIndexType> indices;
    std::vector<ContentsIndexType> relative_indices;
};

template<std::size_t I = 0, class FuncT, typename... Tp>
inline typename std::enable_if<I == sizeof...(Tp), void>::type
for_each_tuple(std::tuple<Tp...> &, FuncT)
{ }

template<std::size_t I = 0, class FuncT, typename... Tp>
inline typename std::enable_if<I < sizeof...(Tp), void>::type
for_each_tuple(std::tuple<Tp...>& t, FuncT f)
{
    f(std::get<I>(t), I);
    for_each_tuple<I + 1, FuncT, Tp...>(t, f);
}

template<typename Type, std::size_t I = 0, std::size_t J = 0, class FuncT,  typename... Tp>
inline typename std::enable_if< I == sizeof...(Tp), void>::type
for_each_of_type_in_tuple(std::tuple<Tp...>&& , FuncT&&)
{ }

template<typename Type, std::size_t I = 0, std::size_t J = 0, class FuncT, typename... Tp>
inline typename std::enable_if< I < sizeof...(Tp), void>::type
for_each_of_type_in_tuple(std::tuple<Tp...>&& tup, FuncT&& func)
{
    using tupType = decltype(tup);
    auto val = std::get<I>(tup);
    if (typeid(val).hash_code() == typeid(Type).hash_code()) {
        func(val, I, J);
        for_each_of_type_in_tuple<Type, I + 1, J + 1, FuncT, Tp...>(std::forward<tupType>(tup), func);
    } else {
        for_each_of_type_in_tuple<Type, I + 1, J, FuncT, Tp...>(std::forward<tupType>(tup), func);
    }
}

#endif /* arg_info_h */
