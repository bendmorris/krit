#ifndef KRIT_MATH_POINT
#define KRIT_MATH_POINT

#include <cmath>

namespace krit {

template <typename T, class Self> class BasePoint {
    public:
        T x = 0, y = 0;

        BasePoint<T, Self>() {}
        BasePoint<T, Self>(T x, T y): x(x), y(y) {}

        bool operator==(const BasePoint<T, Self> &other) {
            return this->x == other.x && this->y == other.y;
        }

        bool operator!=(const BasePoint<T, Self> &other) {
            return this->x != other.x || this->y != other.y;
        }

        Self &lerpInPlace(Self &other, float mix) {
            Self &derived = static_cast<Self&>(*this);
            derived.x = derived.x * (1 - mix) + other.x * mix;
            derived.y = derived.y * (1 - mix) + other.y * mix;
            return derived;
        }

        Self lerp(Self &other, float mix) {
            if (mix >= 1) return other;
            else if (mix <= 0) return static_cast<Self&>(*this);
            else return Self(
                this->x * (1 - mix) + other.x * mix,
                this->y * (1 - mix) + other.y * mix
            );
        }

        Self &setTo(T x, T y) {
            Self &derived = static_cast<Self&>(*this);
            derived.x = x;
            derived.y = y;
            return derived;
        }

        Self &setTo(T v) {
            Self &derived = static_cast<Self&>(*this);
            derived.x = derived.y = v;
            return derived;
        }

        template <typename U> Self &setTo(const U &v) {
            Self &derived = static_cast<Self&>(*this);
            derived.setTo(v.x, v.y);
            return derived;
        }

        Self &copyFrom(const Self &other) {
            Self &derived = static_cast<Self&>(*this);
            derived.x = other.x;
            derived.y = other.y;
            return derived;
        }

        Self &invert() {
            Self &derived = static_cast<Self&>(*this);
            derived.x *= -1;
            derived.y *= -1;
            return derived;
        }

        T length() {
            Self &derived = static_cast<Self&>(*this);
            return std::sqrt(derived.x * derived.x + derived.y * derived.y);
        }

        T squaredDistance(Self &other) {
            Self &derived = static_cast<Self&>(*this);
            T dx = derived.x - other.x;
            T dy = derived.y - other.y;
            return dx * dx + dy * dy;
        }

        T squaredDistance(Self &other, T yFactor) {
            Self &derived = static_cast<Self&>(*this);
            T dx = derived.x - other.x;
            T dy = derived.y - other.y;
            return dx * dx + dy * dy * yFactor;
        }

        T distance(T x, T y) {
            T dx = this->x - x;
            T dy = this->y - y;
            return sqrt(dx * dx + dy * dy);
        }

        T distance(Self &other) {
            Self &derived = static_cast<Self&>(*this);
            T dx = derived.x - other.x;
            T dy = derived.y - other.y;
            return sqrt(dx * dx + dy * dy);
        }

        T distance(Self &other, T yFactor) {
            Self &derived = static_cast<Self&>(*this);
            T dx = derived.x - other.x;
            T dy = derived.y - other.y;
            return sqrt(dx * dx + (dy * dy) * yFactor);
        }

        bool eq(Self &other) {
            Self &derived = static_cast<Self&>(*this);
            return derived.x == other.x && derived.y == other.y;
        }

        Self &normalize(T size) {
            Self &derived = static_cast<Self&>(*this);
            T normal = size / derived.length();
            derived.x *= normal;
            derived.y *= normal;
            return derived;
        }

        Self &perpendicular() {
            Self &derived = static_cast<Self&>(*this);
            return derived.setTo(-derived.y, derived.x);
        }

        Self &add(Self &other) {
            Self &derived = static_cast<Self&>(*this);
            derived.x += other.x;
            derived.y += other.y;
            return derived;
        }

        Self &add(T v) {
            Self &derived = static_cast<Self&>(*this);
            derived.x += v;
            derived.y += v;
            return derived;
        }

        Self &add(T x, T y) {
            Self &derived = static_cast<Self&>(*this);
            derived.x += x;
            derived.y += y;
            return derived;
        }

        Self &subtract(Self &other) {
            Self &derived = static_cast<Self&>(*this);
            derived.x -= other.x;
            derived.y -= other.y;
            return derived;
        }

        Self &subtract(T v) {
            Self &derived = static_cast<Self&>(*this);
            derived.x -= v;
            derived.y -= v;
            return derived;
        }

        Self &subtract(T x, T y) {
            Self &derived = static_cast<Self&>(*this);
            derived.x -= x;
            derived.y -= y;
            return derived;
        }

        Self &multiply(Self &other) {
            Self &derived = static_cast<Self&>(*this);
            derived.x *= other.x;
            derived.y *= other.y;
            return derived;
        }

        Self &multiply(T v) {
            Self &derived = static_cast<Self&>(*this);
            derived.x *= v;
            derived.y *= v;
            return derived;
        }

        Self &multiply(T x, T y) {
            Self &derived = static_cast<Self&>(*this);
            derived.x *= x;
            derived.y *= y;
            return derived;
        }

        Self &divide(Self &other) {
            Self &derived = static_cast<Self&>(*this);
            derived.x /= other.x;
            derived.y /= other.y;
            return derived;
        }

        Self &divide(T v) {
            Self &derived = static_cast<Self&>(*this);
            derived.x /= v;
            derived.y /= v;
            return derived;
        }

        Self &divide(T x, T y) {
            Self &derived = static_cast<Self&>(*this);
            derived.x /= x;
            derived.y /= y;
            return derived;
        }

};

class Point: public BasePoint<float, Point> {
    public:
        Point() {}
        Point(float x, float y) : BasePoint<float, Point>(x, y) {}
};
class IntPoint: public BasePoint<int, IntPoint> {
    public:
        IntPoint() {}
        IntPoint(int x, int y) : BasePoint<int, IntPoint>(x, y) {}
};
class floatPoint: public BasePoint<float, floatPoint> {
    public:
        floatPoint() {}
        floatPoint(float x, float y) : BasePoint<float, floatPoint>(x, y) {}
};

}
#endif
