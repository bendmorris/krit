#pragma once

#include <functional>
#include <type_traits>
#include <variant>

namespace krit {

template <typename T> struct FunctionType;
template <typename Result, typename... Args>
struct FunctionType<Result (*)(Args...)> {
    using return_type = Result;
    using args_tuple = std::tuple<Args...>;
    template <size_t i> struct arg {
        typedef typename std::tuple_element<i, args_tuple>::type type;
    };
};
template <typename ClassType, typename Result, typename... Args>
struct FunctionType<Result (ClassType::*)(Args...)> {
    using return_type = Result;
    using args_tuple = std::tuple<Args...>;
    template <size_t i> struct arg {
        typedef typename std::tuple_element<i, args_tuple>::type type;
    };
};

template <typename T> decltype(auto) access_property(T &obj) {
    return std::invoke(
        [](auto &o) -> decltype(auto) {
            return std::invoke(
                &std::remove_reference_t<decltype(o)>::propertyName, o);
        },
        obj);
}

}
