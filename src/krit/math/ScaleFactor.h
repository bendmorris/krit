#ifndef KRIT_MATH_SCALE_FACTOR
#define KRIT_MATH_SCALE_FACTOR

#include "./Point.h"

namespace krit {

struct ScaleFactor: public BasePoint<float, ScaleFactor> {
    ScaleFactor(float x, float y): BasePoint<float, ScaleFactor>(x, y) {}
    ScaleFactor(): ScaleFactor(1, 1) {}
};

}

#endif
