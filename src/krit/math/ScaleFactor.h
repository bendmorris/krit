#ifndef KRIT_MATH_SCALE_FACTOR
#define KRIT_MATH_SCALE_FACTOR

#include "krit/math/Vec.h"

namespace krit {

struct ScaleFactor : public Vec2f {
    ScaleFactor(float x = 1, float y = 1) : Vec2f(x, y) {}

    void setTo(float s) { Vec2f::setTo(s, s); }
    void setTo(float x, float y) { Vec2f::setTo(x, y); }
};

}

#endif
