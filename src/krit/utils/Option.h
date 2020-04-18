#ifndef KRIT_UTILS_OPTIONAL
#define KRIT_UTILS_OPTIONAL

template<typename T> struct Option {
    bool present;
    T value;

    Option<T>(): present(false) {}
    Option<T>(T value): present(true), value(value) {}
};

#endif
