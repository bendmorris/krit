#ifndef KRIT_MATH_MATRIX
#define KRIT_MATH_MATRIX

#include "krit/math/Triangle.h"
#include "krit/math/Vec.h"
#include <array>
#include <cmath>

namespace krit {

/**
 * [
 *   0  4  8 12
 *   1  5  9 13
 *   2  6 10 14
 *   3  7 11 15
 * ]
 */
struct Matrix4 {
    std::array<float, 16> v{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    float *data() { return &v[0]; }

    void operator*=(const Matrix4 &other);
    Matrix4 operator*(const Matrix4 &other) const;
    Vec3f operator*(const Vec3f &vec) const;
    Vec4f operator*(const Vec4f &vec) const;
    Triangle operator*(const Triangle &vec) const;

    float &operator[](size_t i) { return v[i]; }
    const float &operator[](size_t i) const { return v[i]; }

    void clear() { v = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; }
    void identity() { v = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1}; }
    void invert();

    /**
     * Rotate counterclockwise around the z-axis
     */
    void rotate(float angle);

    /**
     * Rotate around x-axis, away from screen.
     */
    void pitch(float angle);

    void roll(float angle);

    void translate(float x, float y, float z = 0);

    void scale(float sx = 1, float sy = 1, float sz = 1);

    float &a() { return v[0]; }
    const float &a() const { return v[0]; }
    float &b() { return v[1]; }
    const float &b() const { return v[1]; }
    float &c() { return v[4]; }
    const float &c() const { return v[4]; }
    float &d() { return v[5]; }
    const float &d() const { return v[5]; }
    float &tx() { return v[12]; }
    const float &tx() const { return v[12]; }
    float &ty() { return v[13]; }
    const float &ty() const { return v[13]; }
    float &tz() { return v[14]; }
    const float &tz() const { return v[14]; }

    void debugPrint();
};

}
#endif
