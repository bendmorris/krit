#ifndef KRIT_MATH_DIMENSIONS
#define KRIT_MATH_DIMENSIONS

#include "./Point.h"

namespace krit {

template <typename T, class Self> class BaseDimensions: public BasePoint<T, Self> {
    public:
        BaseDimensions<T, Self>() {}
        BaseDimensions<T, Self>(BasePoint<T, Self> *base): BasePoint<T, Self>(base->x, base->y) {}
        BaseDimensions<T, Self>(T x, T y): BasePoint<T, Self>(x, y) {}

        T &width() { return this->x; }
        T &height() { return this->y; }

        T area() {
            return this->x * this->y;
        }
};

class Dimensions: public BaseDimensions<double, Dimensions> {
    public:
        Dimensions() {}
        Dimensions(double x, double y): BaseDimensions<double, Dimensions>(x, y) {}
};
class IntDimensions: public BaseDimensions<int, IntDimensions> {
    public:
        IntDimensions() {}
        IntDimensions(int x, int y): BaseDimensions<int, IntDimensions>(x, y) {}
};

}
#endif
