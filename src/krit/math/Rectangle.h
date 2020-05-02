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

    T top() { return this->y; }
    T bottom() { return this->y + this->height; }
    T left() { return this->x; }
    T right() { return this->x + this->width; }

    bool eq(Self &other) {
        return
            this->x == other.x &&
            this->y == other.y &&
            this->width == other.width &&
            this->height == other.height;
    }

    bool overlaps(Self &other) {
        return this->right() >= other.left() &&
            this->bottom() >= other.top() &&
            this->left() <= other.right() &&
            this->top() <= other.bottom();
    }

    Self join(Self &other) {
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

    template <typename U, typename V> bool contains(BaseRectangle<U, V> &other) {
        return (
            other.x >= left() &&
            other.x <= right() &&
            other.y >= top() &&
            other.y <= bottom()
        );
    }


    template <typename U, typename V> bool contains(BasePoint<U, V> &p) {
        return p.x >= this->x && p.x <= (this->x + this->width) &&
            p.y >= this->y && p.y <= (this->y + this->height);
    }

    Self &setTo(Self &other) {
        return this->setTo(other.x, other.y, other.width, other.height);
    }

    Self &setTo(T x, T y, T w, T h) {
        this->x = x;
        this->y = y;
        this->width = w;
        this->height = h;
        return static_cast<Self&>(*this);
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
