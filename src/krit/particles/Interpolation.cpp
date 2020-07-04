#include "krit/particles/Interpolation.h"
#include <cmath>

namespace krit {

InterpolationFunction InterpolationFunction::linearInterpolation("linear", InterpolationLinear);

float InterpolationFunction::evaluate(float t) {
    switch (type) {
        case InterpolationFunctionPointer: {
            return function(t);
        }
        case InterpolationSpline: {
            return spline.evaluate(t);
        }
        default: {
            return t;
        }
    }
}

float SplineData::evaluate(float t) {
    for (int i = 0; i < 3; ++i) {
        if (t < segments[i + 1].start) {
            return segments[i].evaluate(t, segments[i + 1].start);
        }
    }
    return segments[3].evaluate(t, 1);
}

float SplineSegment::evaluate(float untransformedT, float end) {
    float t = (untransformedT - start) / (end - start);
    float it = 1 - t;
    return it*it*it*p[0] + 3*it*it*t*p[1] + 3*it*t*t*p[2] + t*t*t*p[3];
}

}