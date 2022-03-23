#ifndef KRIT_MATH_RANGE
#define KRIT_MATH_RANGE

#include "krit/math/Random.h"

namespace krit {

template <typename T> struct Range {
    T start;
    T end;

    Range() : start(0), end(0) {}
    Range(T val) : start(val), end(val) {}
    Range(T start, T end) : start(start), end(end) {}

    T random() {
        std::uniform_real_distribution<float> r(0, 1);
        return start + (end - start) * r(rng);
    }

    void setTo(T val) { this->start = this->end = val; }
    void setTo(T start, T end) {
        this->start = start;
        this->end = end;
    }
};

}

#endif
