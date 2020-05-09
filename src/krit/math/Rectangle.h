#ifndef KRIT_MATH_RECTANGLE
#define KRIT_MATH_RECTANGLE

#include "krit/math/Point.h"

namespace krit {

template <typename T, typename Self> struct BaseRectangle {
    T x, y, width, height;

    BaseRectangle<T, Self>() {}
    BaseRectangle<T, Self>(T x, T y, T width, T height) : x(x), y(y), width(width), height(height) {}

    bool operator==(const Self &other) {
        return this->x == other.x && this->y == other.y &&
            this->width == other.width && this->height == other.height;
    }

    T top() const { return this->y; }
    T bottom() const { return this->y + this->height; }
    T left() const { return this->x; }
    T right() const { return this->x + this->width; }

    Point center() {
        return Point(this->x + this->width/2, this->y + this->height/2);
    }

    bool eq(const Self &other) const {
        return
            this->x == other.x &&
            this->y == other.y &&
            this->width == other.width &&
            this->height == other.height;
    }

    bool overlaps(const Self &other) const {
        return this->right() >= other.left() &&
            this->bottom() >= other.top() &&
            this->left() <= other.right() &&
            this->top() <= other.bottom();
    }

    Self &joinInPlace(const Self &other) {
        T xi = min(x, other.x);
        width = max(x + width, other.x + other.width) - xi;
        x = xi;
        T yi = min(y, other.y);
        height = max(y + height, other.y + other.height) - yi;
        y = yi;
        return static_cast<Self&>(*this);
    }

    Self join(const Self &other) {
        T left = min(x, other.x);
        T right = max(x + width, other.x + other.width);
        T top = min(y, other.y);
        T bottom = max(y + height, other.y + other.height);
        return Self(
            left,
            top,
            right - left,
            bottom - top
        );
    }

    Self overlap(const Self &other) {
        T xi = max(x, other.x);
        T yi = max(y, other.y);
        return Self(
            xi, yi,
            min(right(), other.right()) - xi,
            min(bottom(), other.bottom()) - yi
        );
    }

    template <typename U, typename V> bool contains(BasePoint<U, V> &p) {
        return p.x >= this->x && p.x <= (this->x + this->width) &&
            p.y >= this->y && p.y <= (this->y + this->height);
    }

    Self &setTo(const Self &other) {
        return this->setTo(other.x, other.y, other.width, other.height);
    }

    Self &setTo(T x, T y, T w, T h) {
        this->x = x;
        this->y = y;
        this->width = w;
        this->height = h;
        return static_cast<Self&>(*this);
    }

    void debugPrint() {
        printf("%i,%i %ix%i\n", x, y, width, height);
    }
};

struct Rectangle: public BaseRectangle<double, Rectangle> {
    Rectangle() {}
    Rectangle(double x, double y, double width, double height) : BaseRectangle<double, Rectangle>(x, y, width, height) {}
    template<typename T, typename U> Rectangle(BasePoint<double, T> p, BasePoint<double, U> d) : Rectangle(p.x, p.y, d.x, d.y) {}
};

struct IntRectangle: public BaseRectangle<int, IntRectangle> {
    IntRectangle() {}
    IntRectangle(int x, int y, int width, int height) : BaseRectangle<int, IntRectangle>(x, y, width, height) {}
};

}

#endif
