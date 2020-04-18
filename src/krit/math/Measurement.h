#ifndef KRIT_MATH_MEASUREMENT
#define KRIT_MATH_MEASUREMENT

namespace krit {

enum MeasurementType {
    Absolute,
    Percent,
};

struct Measurement {
    MeasurementType type;
    double value;

    Measurement(double value): type(Absolute), value(value) {}
    Measurement(MeasurementType type, double value): type(type), value(value) {}

    double measure(double max) {
        switch (this->type) {
            case Absolute:
                return this->value;
            case Percent:
                return this->value * max / 100;
        }
    }
};

struct AnchoredMeasurement {
    Measurement value;
    double anchor;

    AnchoredMeasurement(Measurement value, double anchor): value(value), anchor(anchor) {}

    double measure(double max, double size) {
        return this->value.measure(max) - size * this->anchor;
    }
};

}
#endif
