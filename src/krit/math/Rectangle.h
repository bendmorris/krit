#ifndef KRIT_MATH_RECTANGLE
#define KRIT_MATH_RECTANGLE

#include "krit/math/Point.h"
#include <algorithm>
#include <cmath>
#include <stdio.h>

namespace krit {

template <typename T, typename Self> struct BaseRectangle {
    T x, y, width, height;

    BaseRectangle<T, Self>() {}
    BaseRectangle<T, Self>(T x, T y, T width, T height)
        : x(x), y(y), width(width), height(height) {}

    bool operator==(const Self &other) {
        return this->x == other.x && this->y == other.y &&
               this->width == other.width && this->height == other.height;
    }

    bool operator!() {
        return !(x || y || width || height);
    }

    T top() const { return this->y; }
    T bottom() const { return this->y + this->height; }
    T left() const { return this->x; }
    T right() const { return this->x + this->width; }

    Point center() {
        return Point(this->x + this->width / 2, this->y + this->height / 2);
    }

    bool eq(const Self &other) const {
        return this->x == other.x && this->y == other.y &&
               this->width == other.width && this->height == other.height;
    }

    bool overlaps(const Self &other) const {
        return this->right() >= other.left() && this->bottom() >= other.top() &&
               this->left() <= other.right() && this->top() <= other.bottom();
    }

    Self &joinInPlace(const Self &other) {
        T xi = std::min(x, other.x);
        width = std::max(x + width, other.x + other.width) - xi;
        x = xi;
        T yi = std::min(y, other.y);
        height = std::max(y + height, other.y + other.height) - yi;
        y = yi;
        return static_cast<Self &>(*this);
    }

    Self join(const Self &other) {
        T left = std::min(x, other.x);
        T right = std::max(x + width, other.x + other.width);
        T top = std::min(y, other.y);
        T bottom = std::max(y + height, other.y + other.height);
        return Self(left, top, right - left, bottom - top);
    }

    Self overlap(const Self &other) {
        T xi = std::max(x, other.x);
        T yi = std::max(y, other.y);
        T w = std::min(right(), other.right()) - xi;
        T h = std::min(bottom(), other.bottom()) - yi;
        return w > 0  && h > 0 ? Self(xi, yi, w, h) : Self();
    }

    template <typename U> bool contains(Vec<U, 2> &p) {
        return p[0] >= this->x && p[0] <= (this->x + this->width) &&
               p[1] >= this->y && p[1] <= (this->y + this->height);
    }

    bool contains(T x, T y) {
        return x >= this->x && x <= (this->x + this->width) &&
               y >= this->y && y <= (this->y + this->height);
    }

    Self &copyFrom(const Self &other) {
        return this->setTo(other.x, other.y, other.width, other.height);
    }

    Self &setTo(T x, T y, T w, T h) {
        this->x = x;
        this->y = y;
        this->width = w;
        this->height = h;
        return static_cast<Self &>(*this);
    }

    void debugPrint() { printf("%.2f,%.2f %.2fx%.2f\n", (float)x, (float)y, (float)width, (float)height); }
};

struct Rectangle : public BaseRectangle<float, Rectangle> {
    Rectangle() {}
    Rectangle(float x, float y, float width, float height)
        : BaseRectangle<float, Rectangle>(x, y, width, height) {}
    template <typename T, typename U>
    Rectangle(Vec2f &p, Vec2f &d)
        : Rectangle(p.x(), p.y(), d.x(), d.y()) {}
};

struct IntRectangle : public BaseRectangle<int, IntRectangle> {
    IntRectangle() {}
    IntRectangle(int x, int y, int width, int height)
        : BaseRectangle<int, IntRectangle>(x, y, width, height) {}
};

}

#endif
