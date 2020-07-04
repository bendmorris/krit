#ifndef KRIT_PARTICLES_INTERPOLATION
#define KRIT_PARTICLES_INTERPOLATION

#include <string>

namespace krit {

typedef float (*InterpFunc)(float);

enum InterpolationFunctionType {
    InterpolationLinear,
    InterpolationFunctionPointer,
    InterpolationSpline,
};

struct SplineSegment {
    float start = 0;
    float p[4] = {0, 0, 1, 1};

    float evaluate(float t, float end);
};

struct SplineData {
    SplineSegment segments[4] = {SplineSegment()};

    SplineData() {}

    float evaluate(float t);
};

struct InterpolationFunction {
    static InterpolationFunction linearInterpolation;

    std::string name;
    InterpolationFunctionType type = InterpolationLinear;
    union {
        InterpFunc function;
        SplineData spline;
    };

    InterpolationFunction(const std::string &name, InterpolationFunctionType type): name(name), type(type) {}
    InterpolationFunction(InterpolationFunctionType type): type(type) {}

    float evaluate(float t);
};

}

#endif