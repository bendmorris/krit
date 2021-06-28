#ifndef KRIT_MATH_MATRIX
#define KRIT_MATH_MATRIX

#include <cmath>

namespace krit {

struct Matrix {
    float a = 1, b = 0, c = 0, d = 1, tx = 0, ty = 0;

    Matrix() {}
    Matrix(float a, float b, float c, float d, float tx, float ty)
        : a(a), b(b), c(c), d(d), tx(tx), ty(ty) {}

    Matrix &setTo(float a, float b, float c, float d, float tx, float ty) {
        this->a = a;
        this->b = b;
        this->c = c;
        this->d = d;
        this->tx = tx;
        this->ty = ty;
        return *this;
    }

    Matrix &rotate(float rads) {
        float rcos = std::cos(rads);
        float rsin = std::sin(rads);
        float a0 = this->a * rcos - this->b * rsin;
        this->b = this->a * rsin + this->b * rcos;
        this->a = a0;

        float c0 = this->c * rcos - this->d * rsin;
        this->d = this->c * rsin + this->d * rcos;
        this->c = c0;

        float t0 = this->tx * rcos - this->ty * rsin;
        this->ty = this->tx * rsin + this->ty * rcos;
        this->tx = t0;

        return *this;
    }

    Matrix &scale(float sx, float sy) {
        this->a *= sx;
        this->b *= sy;
        this->c *= sx;
        this->d *= sy;
        this->tx *= sx;
        this->ty *= sy;
        return *this;
    }

    Matrix &translate(float tx, float ty) {
        this->tx += tx;
        this->ty += ty;
        return *this;
    }
};

}
#endif
