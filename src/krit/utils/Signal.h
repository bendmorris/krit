#ifndef KRIT_UTILS_SIGNAL
#define KRIT_UTILS_SIGNAL

#include <cstdio>
#include <functional>

namespace krit {
    class RenderContext;
    class UpdateContext;

    typedef std::function<void(void)> Signal;
    template <typename A> using Signal1 = std::function<void(A)>;
    template <typename A, typename B> using Signal2 = std::function<void(A, B)>;

    template <typename T> using ReturnSignal = std::function<T(void)>;
    template <typename T, typename A> using ReturnSignal1 = std::function<T(A)>;
    template <typename T, typename A, typename B> using ReturnSignal2 = std::function<T(A, B)>;

    typedef Signal1<UpdateContext*> UpdateSignal;
    typedef Signal1<RenderContext*> RenderSignal;
    typedef ReturnSignal2<bool, UpdateContext*, void*> CustomSignal;
    typedef Signal2<UpdateContext*, void*> SelfUpdateSignal;

    void invoke(Signal s);

    template <typename A> void invoke(Signal1<A> s, A a) {
        if (s) {
            s(a);
        }
    }

    template <typename A, typename B> void invoke(Signal2<A, B> s, A a, B b) {
        if (s) {
            s(a, b);
        }
    }

    template <typename T> T invoke(ReturnSignal<T> s, T defaultReturn) {
        if (s) {
            return s();
        } else {
            return defaultReturn;
        }
    }

    template <typename T, typename A> T invoke(ReturnSignal1<T, A> s, T defaultReturn, A a) {
        if (s) {
            return s(a);
        } else {
            return defaultReturn;
        }
    }

    template <typename T, typename A, typename B> T invoke(ReturnSignal2<T, A, B> s, T defaultReturn, A a, B b) {
        if (s) {
            return s(a, b);
        } else {
            return defaultReturn;
        }
    }
}

#endif
