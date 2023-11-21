#include "krit/math/Matrix.h"

namespace krit {

static Matrix4 getAxisRotation(float x, float y, float z, float rads) {
    Matrix4 m;
    m.identity();

    float c = std::cos(-rads);
    float s = std::sin(-rads);
    float t = 1.0 - c;

    m[0] = c + x * x * t;
    m[5] = c + y * y * t;
    m[10] = c + z * z * t;

    float tmp1 = x * y * t;
    float tmp2 = z * s;
    m[4] = tmp1 + tmp2;
    m[1] = tmp1 - tmp2;
    tmp1 = x * z * t;
    tmp2 = y * s;
    m[8] = tmp1 - tmp2;
    m[2] = tmp1 + tmp2;
    tmp1 = y * z * t;
    tmp2 = x * s;
    m[9] = tmp1 + tmp2;
    m[6] = tmp1 - tmp2;

    return m;
}

Matrix4 Matrix4::operator*(const Matrix4 &other) const {
    float m111 = v[0], m121 = v[4], m131 = v[8], m141 = v[12], m112 = v[1],
          m122 = v[5], m132 = v[9], m142 = v[13], m113 = v[2], m123 = v[6],
          m133 = v[10], m143 = v[14], m114 = v[3], m124 = v[7], m134 = v[11],
          m144 = v[15], m211 = other[0], m221 = other[4], m231 = other[8],
          m241 = other[12], m212 = other[1], m222 = other[5], m232 = other[9],
          m242 = other[13], m213 = other[2], m223 = other[6], m233 = other[10],
          m243 = other[14], m214 = other[3], m224 = other[7], m234 = other[11],
          m244 = other[15];
    Matrix4 m;
    m.v[0] = m111 * m211 + m112 * m221 + m113 * m231 + m114 * m241;
    m.v[1] = m111 * m212 + m112 * m222 + m113 * m232 + m114 * m242;
    m.v[2] = m111 * m213 + m112 * m223 + m113 * m233 + m114 * m243;
    m.v[3] = m111 * m214 + m112 * m224 + m113 * m234 + m114 * m244;
    m.v[4] = m121 * m211 + m122 * m221 + m123 * m231 + m124 * m241;
    m.v[5] = m121 * m212 + m122 * m222 + m123 * m232 + m124 * m242;
    m.v[6] = m121 * m213 + m122 * m223 + m123 * m233 + m124 * m243;
    m.v[7] = m121 * m214 + m122 * m224 + m123 * m234 + m124 * m244;
    m.v[8] = m131 * m211 + m132 * m221 + m133 * m231 + m134 * m241;
    m.v[9] = m131 * m212 + m132 * m222 + m133 * m232 + m134 * m242;
    m.v[10] = m131 * m213 + m132 * m223 + m133 * m233 + m134 * m243;
    m.v[11] = m131 * m214 + m132 * m224 + m133 * m234 + m134 * m244;
    m.v[12] = m141 * m211 + m142 * m221 + m143 * m231 + m144 * m241;
    m.v[13] = m141 * m212 + m142 * m222 + m143 * m232 + m144 * m242;
    m.v[14] = m141 * m213 + m142 * m223 + m143 * m233 + m144 * m243;
    m.v[15] = m141 * m214 + m142 * m224 + m143 * m234 + m144 * m244;
    return m;
}

void Matrix4::operator*=(const Matrix4 &other) {
    float m111 = v[0], m121 = v[4], m131 = v[8], m141 = v[12], m112 = v[1],
          m122 = v[5], m132 = v[9], m142 = v[13], m113 = v[2], m123 = v[6],
          m133 = v[10], m143 = v[14], m114 = v[3], m124 = v[7], m134 = v[11],
          m144 = v[15], m211 = other[0], m221 = other[4], m231 = other[8],
          m241 = other[12], m212 = other[1], m222 = other[5], m232 = other[9],
          m242 = other[13], m213 = other[2], m223 = other[6], m233 = other[10],
          m243 = other[14], m214 = other[3], m224 = other[7], m234 = other[11],
          m244 = other[15];
    v[0] = m111 * m211 + m112 * m221 + m113 * m231 + m114 * m241;
    v[1] = m111 * m212 + m112 * m222 + m113 * m232 + m114 * m242;
    v[2] = m111 * m213 + m112 * m223 + m113 * m233 + m114 * m243;
    v[3] = m111 * m214 + m112 * m224 + m113 * m234 + m114 * m244;
    v[4] = m121 * m211 + m122 * m221 + m123 * m231 + m124 * m241;
    v[5] = m121 * m212 + m122 * m222 + m123 * m232 + m124 * m242;
    v[6] = m121 * m213 + m122 * m223 + m123 * m233 + m124 * m243;
    v[7] = m121 * m214 + m122 * m224 + m123 * m234 + m124 * m244;
    v[8] = m131 * m211 + m132 * m221 + m133 * m231 + m134 * m241;
    v[9] = m131 * m212 + m132 * m222 + m133 * m232 + m134 * m242;
    v[10] = m131 * m213 + m132 * m223 + m133 * m233 + m134 * m243;
    v[11] = m131 * m214 + m132 * m224 + m133 * m234 + m134 * m244;
    v[12] = m141 * m211 + m142 * m221 + m143 * m231 + m144 * m241;
    v[13] = m141 * m212 + m142 * m222 + m143 * m232 + m144 * m242;
    v[14] = m141 * m213 + m142 * m223 + m143 * m233 + m144 * m243;
    v[15] = m141 * m214 + m142 * m224 + m143 * m234 + m144 * m244;
}

Vec4f Matrix4::operator*(const Vec4f &vec) const {
    Vec4f result;
    result[0] = v[0] * vec[0] + v[4] * vec[1] + v[8] * vec[2] + v[12] * vec[3];
    result[1] = v[1] * vec[0] + v[5] * vec[1] + v[9] * vec[2] + v[13] * vec[3];
    result[2] = v[2] * vec[0] + v[6] * vec[1] + v[10] * vec[2] + v[14] * vec[3];
    result[3] = v[3] * vec[0] + v[7] * vec[1] + v[11] * vec[2] + v[15] * vec[3];
    return result;
}

Vec3f Matrix4::operator*(const Vec3f &vec) const {
    Vec3f result;
    result[0] = v[0] * vec[0] + v[4] * vec[1] + v[8] * vec[2] + v[12] * 1;
    result[1] = v[1] * vec[0] + v[5] * vec[1] + v[9] * vec[2] + v[13] * 1;
    result[2] = v[2] * vec[0] + v[6] * vec[1] + v[10] * vec[2] + v[14] * 1;
    return result;
}

Triangle Matrix4::operator*(const Triangle &vec) const {
    Vec3f p1 = (*this) * vec.p1, p2 = (*this) * vec.p2, p3 = (*this) * vec.p3;
    return Triangle(p1, p2, p3);
}

/**
 * Rotate counterclockwise around the z-axis
 */
void Matrix4::rotate(float angle) {
    if (angle) {
        (*this) *= getAxisRotation(0, 0, 1, angle);
    }
}

/**
 * Rotate around x-axis, away from screen.
 */
void Matrix4::pitch(float angle) {
    if (angle) {
        (*this) *= getAxisRotation(1, 0, 0, angle);
    }
}

/**
 * Rotate around y-axis.
 */
void Matrix4::roll(float angle) {
    if (angle) {
        (*this) *= getAxisRotation(0, 1, 0, angle);
    }
}

void Matrix4::translate(float x, float y, float z) {
    v[12] += x;
    v[13] += y;
    v[14] += z;
}

void Matrix4::scale(float sx, float sy, float sz) {
    Matrix4 m;
    m.identity();
    m[0] = sx;
    m[5] = sy;
    m[10] = sz;
    (*this) *= m;
}

void Matrix4::debugPrint() {
    printf("%.6f %.6f %.6f %.6f\n%.6f %.6f %.6f %.6f\n%.6f %.6f %.6f "
           "%.6f\n%.6f %.6f %.6f %.6f\n",
           v[0], v[4], v[8], v[12], v[1], v[5], v[9], v[13], v[2], v[6], v[10],
           v[14], v[3], v[7], v[11], v[15]);
}

void Matrix4::invert() {
    float d =
        1 / ((v[0] * v[5] - v[4] * v[1]) * (v[10] * v[15] - v[14] * v[11]) -
             (v[0] * v[9] - v[8] * v[1]) * (v[6] * v[15] - v[14] * v[7]) +
             (v[0] * v[13] - v[12] * v[1]) * (v[6] * v[11] - v[10] * v[7]) +
             (v[4] * v[9] - v[8] * v[5]) * (v[2] * v[15] - v[14] * v[3]) -
             (v[4] * v[13] - v[12] * v[5]) * (v[2] * v[11] - v[10] * v[3]) +
             (v[8] * v[13] - v[12] * v[9]) * (v[2] * v[7] - v[6] * v[3]));

    float m11 = v[0];
    float m21 = v[4];
    float m31 = v[8];
    float m41 = v[12];
    float m12 = v[1];
    float m22 = v[5];
    float m32 = v[9];
    float m42 = v[13];
    float m13 = v[2];
    float m23 = v[6];
    float m33 = v[10];
    float m43 = v[14];
    float m14 = v[3];
    float m24 = v[7];
    float m34 = v[11];
    float m44 = v[15];

    v[0] = d * (m22 * (m33 * m44 - m43 * m34) - m32 * (m23 * m44 - m43 * m24) +
                m42 * (m23 * m34 - m33 * m24));
    v[1] = -d * (m12 * (m33 * m44 - m43 * m34) - m32 * (m13 * m44 - m43 * m14) +
                 m42 * (m13 * m34 - m33 * m14));
    v[2] = d * (m12 * (m23 * m44 - m43 * m24) - m22 * (m13 * m44 - m43 * m14) +
                m42 * (m13 * m24 - m23 * m14));
    v[3] = -d * (m12 * (m23 * m34 - m33 * m24) - m22 * (m13 * m34 - m33 * m14) +
                 m32 * (m13 * m24 - m23 * m14));
    v[4] = -d * (m21 * (m33 * m44 - m43 * m34) - m31 * (m23 * m44 - m43 * m24) +
                 m41 * (m23 * m34 - m33 * m24));
    v[5] = d * (m11 * (m33 * m44 - m43 * m34) - m31 * (m13 * m44 - m43 * m14) +
                m41 * (m13 * m34 - m33 * m14));
    v[6] = -d * (m11 * (m23 * m44 - m43 * m24) - m21 * (m13 * m44 - m43 * m14) +
                 m41 * (m13 * m24 - m23 * m14));
    v[7] = d * (m11 * (m23 * m34 - m33 * m24) - m21 * (m13 * m34 - m33 * m14) +
                m31 * (m13 * m24 - m23 * m14));
    v[8] = d * (m21 * (m32 * m44 - m42 * m34) - m31 * (m22 * m44 - m42 * m24) +
                m41 * (m22 * m34 - m32 * m24));
    v[9] = -d * (m11 * (m32 * m44 - m42 * m34) - m31 * (m12 * m44 - m42 * m14) +
                 m41 * (m12 * m34 - m32 * m14));
    v[10] = d * (m11 * (m22 * m44 - m42 * m24) - m21 * (m12 * m44 - m42 * m14) +
                 m41 * (m12 * m24 - m22 * m14));
    v[11] =
        -d * (m11 * (m22 * m34 - m32 * m24) - m21 * (m12 * m34 - m32 * m14) +
              m31 * (m12 * m24 - m22 * m14));
    v[12] =
        -d * (m21 * (m32 * m43 - m42 * m33) - m31 * (m22 * m43 - m42 * m23) +
              m41 * (m22 * m33 - m32 * m23));
    v[13] = d * (m11 * (m32 * m43 - m42 * m33) - m31 * (m12 * m43 - m42 * m13) +
                 m41 * (m12 * m33 - m32 * m13));
    v[14] =
        -d * (m11 * (m22 * m43 - m42 * m23) - m21 * (m12 * m43 - m42 * m13) +
              m41 * (m12 * m23 - m22 * m13));
    v[15] = d * (m11 * (m22 * m33 - m32 * m23) - m21 * (m12 * m33 - m32 * m13) +
                 m31 * (m12 * m23 - m22 * m13));
}

}
