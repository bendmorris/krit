#ifndef KRIT_ECS_UTILS
#define KRIT_ECS_UTILS

#include <tuple>
#include <type_traits>
#include <utility>

namespace krit {

using namespace std;
using namespace krit;

template <class Tuple, class T, bool Fail = true, int Index = 0>
struct find_first;

template <int Index, bool Fail, bool Valid>
struct find_first_final_test
    : public integral_constant<int, Index>
{
};

template <int Index>
struct find_first_final_test<Index, true, false>
{
    static_assert(Index == -1, "This component type is not part of the World");
};

template <int Index>
struct find_first_final_test<Index, false, false>
    : public integral_constant<int, -1>
{
};

template <class Head, class T, bool Fail, int Index>
struct find_first<tuple<Head>, T, Fail, Index>
    : public find_first_final_test<Index, Fail, is_same<Head, T>::value>
{
};

template <class Head, class ...Rest, class T, bool Fail, int Index>
struct find_first<tuple<Head, Rest...>, T, Fail, Index>
    : public conditional<is_same<Head, T>::value,
        integral_constant<int, Index>,
        find_first<tuple<Rest...>, T, Fail, Index+1>>::type
{
};

}

#endif
