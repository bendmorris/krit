#ifndef KRIT_MATH_MEASUREMENT
#define KRIT_MATH_MEASUREMENT

namespace krit {

enum MeasurementType {
    Absolute,
    Percent,
};

struct Measurement {
    MeasurementType type;
    float value;

    Measurement(float value) : type(Absolute), value(value) {}
    Measurement(MeasurementType type, float value) : type(type), value(value) {}

    float measure(float max) {
        return type == Percent ? (value * max / 100) : value;
    }
};

struct AnchoredMeasurement {
    Measurement value;
    float anchor;

    AnchoredMeasurement(Measurement value, float anchor)
        : value(value), anchor(anchor) {}

    float measure(float max, float size) {
        return this->value.measure(max) - size * this->anchor;
    }
};

}
#endif
