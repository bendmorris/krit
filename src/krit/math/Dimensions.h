#ifndef KRIT_MATH_DIMENSIONS
#define KRIT_MATH_DIMENSIONS

#include "krit/math/Point.h"

namespace krit {

template <typename T, typename Self> struct BaseDimensions: public BasePoint<T, Self> {
    BaseDimensions<T, Self>() {}
    BaseDimensions<T, Self>(BasePoint<T, Self> *base): BasePoint<T, Self>(base->x, base->y) {}
    BaseDimensions<T, Self>(T x, T y): BasePoint<T, Self>(x, y) {}

    T &width() { return this->x; }
    T &height() { return this->y; }

    T area() {
        return this->x * this->y;
    }
};

struct Dimensions: public BaseDimensions<float, Dimensions> {
    Dimensions() {}
    Dimensions(float x, float y): BaseDimensions<float, Dimensions>(x, y) {}
};
struct IntDimensions: public BaseDimensions<int, IntDimensions> {
    IntDimensions() {}
    IntDimensions(int x, int y): BaseDimensions<int, IntDimensions>(x, y) {}
};

}
#endif
