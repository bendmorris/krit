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
    Vec3f p1;
    Vec3f p2;
    Vec3f p3;

    Triangle() {}
    Triangle(const Triangle &) = default;
    Triangle(Vec2f &p1, Vec2f &p2, Vec2f &p3)
        : p1(p1.x(), p1.y()), p2(p2.x(), p2.y()), p3(p3.x(), p3.y()) {}
    Triangle(Vec3f &p1, Vec3f &p2, Vec3f &p3) : p1(p1), p2(p2), p3(p3) {}
    Triangle(float a, float b, float c, float d, float e, float f)
        : p1(a, b, 0), p2(c, d, 0), p3(e, f, 0) {}

    Triangle &scale(float s) {
        this->p1 *= s;
        this->p2 *= s;
        this->p3 *= s;
        return *this;
    }

    Triangle &scale(float sx, float sy) {
        this->p1 *= Vec3f(sx, sy, 0);
        this->p2 *= Vec3f(sx, sy, 0);
        this->p3 *= Vec3f(sx, sy, 0);
        return *this;
    }

    Triangle &translate(Vec3f &p) {
        this->p1 += p;
        this->p2 += p;
        this->p3 += p;
        return *this;
    }

    void debugPrint() {
        printf("%.2f,%.2f %.2f,%.2f %.2f,%.2f\n", p1.x(), p1.y(), p2.x(),
               p2.y(), p3.x(), p3.y());
    }

    Rectangle bounds() {
        float x1 = minOf3(p1.x(), p2.x(), p3.x()),
              y1 = minOf3(p1.y(), p2.y(), p3.y()),
              x2 = maxOf3(p1.x(), p2.x(), p3.x()),
              y2 = maxOf3(p1.y(), p2.y(), p3.y());
        return Rectangle(x1, y1, x2 - x1, y2 - y1);
    }
};

}
#endif
