#include "krit/Utils.h"

namespace krit {

std::random_device rd;
std::mt19937 rng(rd());

template <> double smoothStep<0>(double n) {
    return pow(n, 2) * (3 - 2 * n);
}

double maybeLerp(double t, double f(double)) {
    if (f) {
        t = f(t);
    }
    return t;
}

double noChange(double t) {
    return 0;
}

double easeInOut(double t) {
    return pow(2 * t - 1, 2);
}

double easeOutBounce(double t) {
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
