#ifndef KRIT_MATH_MATRIX
#define KRIT_MATH_MATRIX

#include <cmath>

namespace krit {

class Matrix {
    public:
        double a = 1, b = 0, c = 0, d = 1, tx = 0, ty = 0;

        Matrix() {}
        Matrix(double a, double b, double c, double d, double tx, double ty)
            : a(a), b(b), c(c), d(d), tx(tx), ty(ty) {}

        Matrix &setTo(double a, double b, double c, double d, double tx, double ty) {
            this->a = a;
            this->b = b;
            this->c = c;
            this->d = d;
            this->tx = tx;
            this->ty = ty;
            return *this;
        }

        Matrix &rotate(double rads) {
            double rcos = std::cos(rads);
            double rsin = std::sin(rads);
            double a0 = this->a * rcos - this->b * rsin;
            this->b = this->a * rsin + this->b * rcos;
            this->a = a0;

            double c0 = this->c * rcos - this->d * rsin;
            this->d = this->c * rsin + this->d * rcos;
            this->c = c0;

            double t0 = this->tx * rcos - this->ty * rsin;
            this->ty = this->tx * rsin + this->ty * rcos;
            this->tx = t0;

            return *this;
        }

        Matrix &scale(double sx, double sy) {
            this->a *= sx;
            this->b *= sy;
            this->c *= sx;
            this->d *= sy;
            this->tx *= sx;
            this->ty *= sy;
            return *this;
        }

        Matrix &translate(double tx, double ty) {
            this->tx += tx;
            this->ty += ty;
            return *this;
        }
};

}
#endif
