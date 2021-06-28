#include "krit/Utils.h"

#include <cmath>

namespace krit {

std::random_device rd;
std::mt19937 rng(rd());

template <> float smoothStep<0>(float n) {
    return pow(n, 2) * (3 - 2 * n);
}

float maybeLerp(float t, float f(float)) {
    if (f) {
        t = f(t);
    }
    return t;
}

float noChange(float t) {
    return 0;
}

float easeInOut(float t) {
    return pow(2 * t - 1, 2);
}

float easeOutBounce(float t) {
    if (t < (1/2.75)) {
        return 7.5625*t*t;
    } else if (t < (2/2.75)) {
        t -= 1.5 / 2.75;
        return 7.5625*t*t + .75;
    } else if (t < (2.5/2.75)) {
        t -= 2.25 / 2.75;
        return 7.5625*t*t + .9375;
    } else {
        t -= 2.625 / 2.75;
        return 7.5625*t*t + .984375;
    }
}

}
