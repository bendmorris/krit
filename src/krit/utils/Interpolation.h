#ifndef KRIT_UTILS_INTERPOLATION
#define KRIT_UTILS_INTERPOLATION

#include <cassert>
#include <iterator>
#include <random>
#include <stddef.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace krit {

extern std::unordered_map<std::string, float (*)(float)> interpolationFunctions;

template <typename T> T easeOutSine(T t) { return sin((t * M_PI) / 2); }
template <typename T> T easeInSine(T t) { return 1 - cos((t * M_PI) / 2); }
// accelerating from zero velocity
template <typename T> T easeInQuad(T t) { return t * t; }
// decelerating to zero velocity
template <typename T> T easeOutQuad(T t) { return t * (2 - t); }
// acceleration until halfway, then deceleration
template <typename T> T easeInOutQuad(T t) {
    return (t < 0.5 ? 2 * t * t : -1 + (4 - 2 * t) * t);
}
// accelerating from zero velocity
template <typename T> T easeInCubic(T t) { return t * t * t; }
// decelerating to zero velocity
template <typename T> T easeOutCubic(T t) {
    --t;
    return t * t * t + 1;
}
// acceleration until halfway, then deceleration
template <typename T> T easeInOutCubic(T t) {
    return (t < 0.5 ? 4 * t * t * t : (t - 1) * (2 * t - 2) * (2 * t - 2) + 1);
}
// accelerating from zero velocity
template <typename T> T easeInQuart(T t) { return t * t * t * t; }
// decelerating to zero velocity
template <typename T> T easeOutQuart(T t) {
    --t;
    return 1 - t * t * t * t;
}
// acceleration until halfway, then deceleration
template <typename T> T easeInOutQuart(T t) {
    if (t < 0.5) {
        return 8 * t * t * t * t;
    } else {
        --t;
        return 1 - 8 * t * t * t * t;
    }
}
// accelerating from zero velocity
template <typename T> T easeInQuint(T t) { return t * t * t * t * t; }
// decelerating to zero velocity
template <typename T> T easeOutQuint(T t) {
    --t;
    return 1 + t * t * t * t * t;
}
// acceleration until halfway, then deceleration
template <typename T> T easeInOutQuint(T t) {
    if (t < 0.5) {
        return 16 * t * t * t * t * t;
    } else {
        --t;
        return 1 + 16 * t * t * t * t * t;
    }
}

template <typename T> T easeOutBounce(T t) {
    if (t < 1 / 2.75) {
        return 7.5625 * t * t;
    } else if (t < 2 / 2.75) {
        t -= 1.5 / 2.75;
        return 7.5625 * t * t + 0.75;
    } else if (t < 2.5 / 2.75) {
        t -= 2.25 / 2.75;
        return 7.5625 * t * t + 0.9375;
    } else {
        t -= 2.625 / 2.75;
        return 7.5625 * t * t + 0.984375;
    }
}

}

#endif
