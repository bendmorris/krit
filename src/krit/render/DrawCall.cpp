#include "krit/Math.h"
#include "krit/render/DrawCall.h"

namespace krit {

template <typename T> T minOf3(T a, T b, T c) {
    return a > b ? (b > c ? c : b) : (a > c ? c : a);
}

template <typename T> T maxOf3(T a, T b, T c) {
    return a < b ? (b < c ? c : b) : (a < c ? c : a);
}

Rectangle TriangleData::bounds() {
    double x1 = minOf3(this->t.p1.x, this->t.p2.x, this->t.p3.x),
        y1 = minOf3(this->t.p1.y, this->t.p2.y, this->t.p3.y),
        x2 = maxOf3(this->t.p1.x, this->t.p2.x, this->t.p3.x),
        y2 = maxOf3(this->t.p1.y, this->t.p2.y, this->t.p3.y);
    return Rectangle(x1, y1, x2 - x1, y2 - y1);
}

}
