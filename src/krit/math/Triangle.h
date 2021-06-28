#ifndef KRIT_MATH_TRIANGLE
#define KRIT_MATH_TRIANGLE

#include "krit/math/Point.h"
#include "krit/math/Rectangle.h"
#include <stdio.h>

namespace krit {

template <typename T> T minOf3(T a, T b, T c) {
    return a > b ? (b > c ? c : b) : (a > c ? c : a);
}

template <typename T> T maxOf3(T a, T b, T c) {
    return a < b ? (b < c ? c : b) : (a < c ? c : a);
}

struct Triangle {
    Point p1;
    Point p2;
    Point p3;

    Triangle() {}
    Triangle(const Triangle &) = default;
    Triangle(Point &p1, Point &p2, Point &p3) : p1(p1), p2(p2), p3(p3) {}
    template <typename T, typename U>
    Triangle(BasePoint<T, U> &p1, BasePoint<T, U> &p2, BasePoint<T, U> &p3)
        : p1(p1.x, p1.y), p2(p2.x, p2.y), p3(p3.x, p3.y) {}
    Triangle(float a, float b, float c, float d, float e, float f)
        : p1(a, b), p2(c, d), p3(e, f) {}

    Triangle &scale(float s) {
        this->p1.multiply(s);
        this->p2.multiply(s);
        this->p3.multiply(s);
        return *this;
    }

    Triangle &scale(float sx, float sy) {
        this->p1.multiply(sx, sy);
        this->p2.multiply(sx, sy);
        this->p3.multiply(sx, sy);
        return *this;
    }

    Triangle &translate(Point &p) {
        this->p1.add(p.x, p.y);
        this->p2.add(p.x, p.y);
        this->p3.add(p.x, p.y);
        return *this;
    }

    void debugPrint() {
        printf("%.2f,%.2f %.2f,%.2f %.2f,%.2f\n", p1.x, p1.y, p2.x, p2.y, p3.x,
               p3.y);
    }

    Rectangle bounds() {
        float x1 = minOf3(p1.x, p2.x, p3.x), y1 = minOf3(p1.y, p2.y, p3.y),
              x2 = maxOf3(p1.x, p2.x, p3.x), y2 = maxOf3(p1.y, p2.y, p3.y);
        return Rectangle(x1, y1, x2 - x1, y2 - y1);
    }
};

}
#endif
