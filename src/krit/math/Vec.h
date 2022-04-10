#ifndef KRIT_MATH_VEC
#define KRIT_MATH_VEC

#include <array>
#include <cmath>
#include <initializer_list>
#include <type_traits>

namespace krit {

template <typename T, size_t N> struct Vec {
    Vec() : v{0} {}
    template <typename U> Vec(const Vec<U, N> &other) : v(other.v) {}
    template <typename... Arg> Vec(Arg &&... vals) : v{vals...} {}

    template <typename U> void copyFrom(const Vec<U, N> &other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] = other[i];
        }
    }

    T &operator[](size_t i) { return v[i]; }
    const T &operator[](size_t i) const { return v[i]; }

    template <typename U> bool operator==(const Vec<U, N> &other) {
        for (size_t i = 0; i < N; ++i) {
            if (v[i] != other[i]) {
                return false;
            }
        }
        return true;
    }

    template <typename U> bool operator!=(const Vec<U, N> &other) {
        for (size_t i = 0; i < N; ++i) {
            if (v[i] != other[i]) {
                return true;
            }
        }
        return false;
    }

    template <typename U> void operator+=(const Vec<U, N> &other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] += other[i];
        }
    }
    void operator+=(T other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] += other;
        }
    }
    template <typename U> Vec<T, N> operator+(const Vec<U, N> &other) const {
        Vec<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = v[i] + other[i];
        }
        return result;
    }

    template <typename U> void operator-=(const Vec<U, N> &other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] -= other[i];
        }
    }
    void operator-=(T other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] -= other;
        }
    }
    template <typename U> Vec<T, N> operator-(const Vec<U, N> &other) const {
        Vec<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = v[i] - other[i];
        }
        return result;
    }

    template <typename U> void operator*=(const Vec<U, N> &other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] *= other[i];
        }
    }
    void operator*=(T other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] *= other;
        }
    }
    template <typename U> Vec<T, N> operator*(const Vec<U, N> &other) const {
        Vec<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = v[i] * other[i];
        }
        return result;
    }
    Vec<T, N> operator*(T val) const {
        Vec<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = v[i] * val;
        }
        return result;
    }

    template <typename U> void operator/=(const Vec<U, N> &other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] /= other[i];
        }
    }
    void operator/=(T other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] /= other;
        }
    }
    template <typename U> Vec<T, N> operator/(const Vec<U, N> &other) const {
        Vec<T, N> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = v[i] / other[i];
        }
        return result;
    }

    template <typename U> float distance(const Vec<U, N> &other) {
        return sqrt(squaredDistance(other));
    }

    template <typename U> float squaredDistance(const Vec<U, N> &other) {
        float dist = 0;
        for (size_t i = 0; i < N; ++i) {
            dist += pow(v[i] - other[i], 2);
        }
        return dist;
    }

    std::array<T, N> v;
};

template <typename T> struct Vec2 : public Vec<T, 2> {
    Vec2(T x = 0, T y = 0) : Vec<T, 2>{x, y} {}
    T &x() { return (*this)[0]; }
    T &y() { return (*this)[1]; }
    const T &x() const { return (*this)[0]; }
    const T &y() const { return (*this)[1]; }
    void setTo(T x = 0, T y = 0) {
        this->x() = x;
        this->y() = y;
    }
};

template <typename T> struct Vec3 : public Vec<T, 3> {
    Vec3(T x = 0, T y = 0, T z = 0) : Vec<T, 3>{x, y, z} {}
    T &x() { return (*this)[0]; }
    T &y() { return (*this)[1]; }
    T &z() { return (*this)[2]; }
    const T &x() const { return (*this)[0]; }
    const T &y() const { return (*this)[1]; }
    const T &z() const { return (*this)[2]; }
    void setTo(T x = 0, T y = 0, T z = 0) {
        this->x() = x;
        this->y() = y;
        this->z() = z;
    }
};

template <typename T> struct Vec4 : public Vec<T, 4> {
    Vec4(T x = 0, T y = 0, T z = 0, T w = 0) : Vec<T, 4>{x, y, z, w} {}
    T &x() { return (*this)[0]; }
    T &y() { return (*this)[1]; }
    T &z() { return (*this)[2]; }
    T &w() { return (*this)[3]; }
    const T &x() const { return (*this)[0]; }
    const T &y() const { return (*this)[1]; }
    const T &z() const { return (*this)[2]; }
    const T &w() const { return (*this)[3]; }
    void setTo(T x = 0, T y = 0, T z = 0, T w = 0) {
        this->x() = x;
        this->y() = y;
        this->z() = z;
        this->w() = w;
    }
};

using Vec2f = Vec2<float>;
using Vec3f = Vec3<float>;
using Vec4f = Vec4<float>;
using Vec2i = Vec2<int>;
using Vec3i = Vec3<int>;
using Vec4i = Vec4<int>;

}

#endif
