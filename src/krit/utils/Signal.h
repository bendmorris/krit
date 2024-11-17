#ifndef KRIT_UTILS_SIGNAL
#define KRIT_UTILS_SIGNAL

#include <functional>

namespace krit {

template <typename... Args> using VoidSignal = std::function<void(Args...)>;
template <typename T, typename... Args>
using ReturnSignal = std::function<T(Args...)>;

using Signal = VoidSignal<>;

template <typename... Args> void invoke(VoidSignal<Args...> s, Args... args) {
    if (s) {
        s(args...);
    }
}
template <typename T, typename... Args>
T invoke(ReturnSignal<T, Args...> s, T defaultReturn, Args... args) {
    if (s) {
        return s(args...);
    }
    return defaultReturn;
}

}

#endif
