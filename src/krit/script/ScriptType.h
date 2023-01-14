#ifndef KRIT_SCRIPT_SCRIPTTYPES
#define KRIT_SCRIPT_SCRIPTTYPES

#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>

namespace krit {

template <typename T> struct prop_type {
    using type = typename std::conditional<
        std::is_pointer<T>::value ||
            std::is_reference<typename std::remove_const<T>::type>::value,
        T, T &>::type;
};

template <typename T> struct remove_all { using type = T; };
template <typename T> struct remove_all<T *> {
    using type = typename remove_all<T>::type;
};
template <typename T> struct remove_all<std::unique_ptr<T>> {
    using type = typename remove_all<T>::type;
};
template <typename T> struct remove_all<std::shared_ptr<T>> {
    using type = typename remove_all<T>::type;
};
template <typename T> struct remove_all<T &> {
    using type = typename remove_all<T>::type;
};

template <typename T> struct remove_all<T &&> {
    using type = typename remove_all<T>::type;
};

template <typename T> struct remove_all<const T> {
    using type = typename remove_all<T>::type;
};

template <typename T, typename U> struct TypeValidator {
    using A = typename remove_all<T>::type;
    using B = typename remove_all<U>::type;
    constexpr static bool valid =
        (std::is_arithmetic<A>::value && std::is_arithmetic<B>::value) ||
        std::is_same<A, B>::value;
};
template <typename T> struct TypeValidator<std::vector<T>, std::vector<T>> {
    constexpr static bool valid = true;
};
template <typename T, typename U>
struct TypeValidator<std::vector<T>, std::vector<U>> {
    using A = typename remove_all<T>::type;
    using B = typename remove_all<U>::type;
    constexpr static bool valid = TypeValidator<A, B>::valid;
};
template <> struct TypeValidator<void, void> {
    constexpr static bool valid = true;
};
template <typename T> struct TypeValidator<T, T> {
    constexpr static bool valid = true;
};
template <typename T> struct TypeValidator<T, void> {
    constexpr static bool valid = true;
};
template <> struct TypeValidator<std::string, const char *> {
    constexpr static bool valid = true;
};
template <> struct TypeValidator<std::string, std::string_view> {
    constexpr static bool valid = true;
};
template <> struct TypeValidator<const char *, std::string> {
    constexpr static bool valid = true;
};
template <> struct TypeValidator<const char *, std::string_view> {
    constexpr static bool valid = true;
};
template <> struct TypeValidator<std::string_view, std::string> {
    constexpr static bool valid = true;
};
template <> struct TypeValidator<std::string_view, const char *> {
    constexpr static bool valid = true;
};

// primary template.
template <class T>
struct FunctionInfo : FunctionInfo<decltype(&T::operator())> {};

// partial specialization for function type
template <class R, class... Args> struct FunctionInfo<R(Args...)> {
    using ReturnType = R;
    using ArgTypes = std::tuple<Args...>;
};

// partial specialization for function pointer
template <class R, class... Args> struct FunctionInfo<R (*)(Args...)> {
    using ReturnType = R;
    using ArgTypes = std::tuple<Args...>;
};

// partial specialization for std::function
template <class R, class... Args>
struct FunctionInfo<std::function<R(Args...)>> {
    using ReturnType = R;
    using ArgTypes = std::tuple<Args...>;
};

// partial specialization for pointer-to-member-function (i.e., operator()'s)
template <class T, class R, class... Args>
struct FunctionInfo<R (T::*)(Args...)> {
    using ReturnType = R;
    using ArgTypes = std::tuple<Args...>;
};

template <class T, class R, class... Args>
struct FunctionInfo<R (T::*)(Args...) const> {
    using ReturnType = R;
    using ArgTypes = std::tuple<Args...>;
};

}

#endif
