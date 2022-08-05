#ifndef KRIT_MATH_VEC
#define KRIT_MATH_VEC

#include <array>
#include <cmath>
#include <initializer_list>
#include <type_traits>

namespace krit {

template <typename T, size_t N, typename Self> struct Vec {
    Vec() : v{0} {}
    template <typename... Arg> Vec(Arg &&... vals) : v{vals...} {}

    void copyFrom(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] = other[i];
        }
    }

    T &operator[](size_t i) { return v[i]; }
    const T &operator[](size_t i) const { return v[i]; }

    bool operator==(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            if (v[i] != other[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            if (v[i] != other[i]) {
                return true;
            }
        }
        return false;
    }

    void operator+=(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] += other[i];
        }
    }
    void operator+=(T other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] += other;
        }
    }
    Self operator+(const Self &other) const {
        Self result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = v[i] + other[i];
        }
        return result;
    }

    void operator-=(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] -= other[i];
        }
    }
    void operator-=(T other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] -= other;
        }
    }
    Self operator-(const Self &other) const {
        Self result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = v[i] - other[i];
        }
        return result;
    }

    void operator*=(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] *= other[i];
        }
    }
    void operator*=(T other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] *= other;
        }
    }
    Self operator*(const Self &other) const {
        Self result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = v[i] * other[i];
        }
        return result;
    }
    Self operator*(T val) const {
        Self result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = v[i] * val;
        }
        return result;
    }

    void operator/=(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] /= other[i];
        }
    }
    void operator/=(T other) {
        for (size_t i = 0; i < N; ++i) {
            v[i] /= other;
        }
    }
    Self operator/(const Self &other) const {
        Self result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = v[i] / other[i];
        }
        return result;
    }

    float distance(const Self &other) {
        return sqrt(squaredDistance(other));
    }

    float squaredDistance(const Self &other) {
        float dist = 0;
        for (size_t i = 0; i < N; ++i) {
            dist += pow(v[i] - other[i], 2);
        }
        return dist;
    }

    void invert() {
        for (size_t i = 0; i < N; ++i) {
            v[i] = -v[i];
        }
    }

    float length() { return distance(Self()); }

    void normalize(float size) {
        float normal = size / length();
        for (size_t i = 0; i < N; ++i) {
            v[i] *= normal;
        }
    }

    std::array<T, N> v;
};

template <typename T> struct Vec2 : public Vec<T, 2, Vec2<T>> {
    static Vec2<T> *create(T x, T y) { return new Vec2<T>(x, y); }
    Vec2(T x = 0, T y = 0) : Vec<T, 2, Vec2<T>>{x, y} {}
    T &x() { return (*this)[0]; }
    T &y() { return (*this)[1]; }
    const T &x() const { return (*this)[0]; }
    const T &y() const { return (*this)[1]; }
    Vec2<T> perpendicular() { return Vec2<T>(-y(), x()); }
    float zcross(const Vec2<T> &other) {
        return (x() * other.y()) - (y() * other.x());
    }
    float dot(const Vec2<T> &other) {
        return (x() * other.x()) + (y() * other.y());
    }
    void setTo(T x = 0, T y = 0) {
        this->x() = x;
        this->y() = y;
    }
};

template <typename T> struct Vec3 : public Vec<T, 3, Vec3<T>> {
    static Vec3<T> *create(T x, T y, T z) { return new Vec3<T>(x, y, z); }
    Vec3(T x = 0, T y = 0, T z = 0) : Vec<T, 3, Vec3<T>>{x, y, z} {}
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

template <typename T> struct Vec4 : public Vec<T, 4, Vec4<T>> {
    Vec4(T x = 0, T y = 0, T z = 0, T w = 0) : Vec<T, 4, Vec4<T>>{x, y, z, w} {}
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
