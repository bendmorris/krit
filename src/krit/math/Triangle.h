#ifndef KRIT_MATH_TRIANGLE
#define KRIT_MATH_TRIANGLE

#include "./Point.h"

namespace krit {

struct Triangle {
    Point p1;
    Point p2;
    Point p3;

    Triangle() {}
    Triangle(const Triangle&) = default;
    Triangle(Point &p1, Point &p2, Point &p3) : p1(p1), p2(p2), p3(p3) {}
    Triangle(double a, double b, double c, double d, double e, double f): p1(a, b), p2(c, d), p3(e, f) {}

    Triangle &scale(double s) {
        this->p1.multiply(s);
        this->p2.multiply(s);
        this->p3.multiply(s);
        return *this;
    }

    Triangle &scale(double sx, double sy) {
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
};

}
#endif
