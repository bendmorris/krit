#ifndef KRIT_MATH_SCALE_FACTOR
#define KRIT_MATH_SCALE_FACTOR

#include "./Point.h"

using namespace krit;

namespace krit {

class ScaleFactor: public BasePoint<double, ScaleFactor> {
    public:
        ScaleFactor(double x, double y): BasePoint<double, ScaleFactor>(x, y) {}
        ScaleFactor(): ScaleFactor(1, 1) {}
};

}

#endif
