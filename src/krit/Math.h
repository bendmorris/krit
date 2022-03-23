#ifndef KRIT_MATH
#define KRIT_MATH

#include "krit/math/Dimensions.h"
#include "krit/math/Matrix.h"
#include "krit/math/Measurement.h"
#include "krit/math/Point.h"
#include "krit/math/Range.h"
#include "krit/math/Rectangle.h"
#include "krit/math/ScaleFactor.h"
#include "krit/math/Triangle.h"

namespace krit {

#define M_PI 3.14159265358979323846
#define SMALL_AMOUNT 0.00000001

template <typename T> T clamp(T v, T min, T max) {
    assert(min <= max);
    if (v < min)
        return min;
    if (v > max)
        return max;
    return v;
}

template <typename T> T lerp(T v1, T v2, T mix) {
    if (mix <= 0)
        return v1;
    else if (mix >= 1)
        return v2;
    else
        return (1 - mix) * v1 + mix * v2;
}

/**
 * Smooth, S-shaped interpolation between 0 and 1.
 *
 * Providing a template argument will apply the function to its result,
 * N times, creating an increasingly steep slope.
 */
template <int N = 0> float smoothStep(float n) {
    return smoothStep<0>(smoothStep<N - 1>(n));
}

template <> float smoothStep<0>(float n);

}

#endif
