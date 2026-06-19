#pragma once

#include <array>
#include <cmath>
#include <initializer_list>
#include <tuple>
#include <type_traits>

namespace krit {

template <typename T> struct Vec1Base {
    T x{0};

    Vec1Base(T x = 0) : x(x) {}
};

template <typename T> struct Vec2Base {
    T x{0}, y{0};

    Vec2Base(T x = 0, T y = 0) : x(x), y(y) {}
};

template <typename T> struct Vec3Base {
    T x{0}, y{0}, z{0};

    Vec3Base(T x = 0, T y = 0, T z = 0) : x(x), y(y), z(z) {}
};

template <typename T> struct Vec4Base {
    T x{0}, y{0}, z{0}, w{0};

    Vec4Base(T x = 0, T y = 0, T z = 0, T w = 0) : x(x), y(y), z(z), w(w) {}
};

template <typename T, size_t N, typename Self>
struct Vec
    : public std::tuple_element_t<N - 1, std::tuple<Vec1Base<T>, Vec2Base<T>,
                                                    Vec3Base<T>, Vec4Base<T>>> {
    template <typename... Arg>
    Vec(Arg &&...vals)
        : std::tuple_element_t<N - 1, std::tuple<Vec1Base<T>, Vec2Base<T>,
                                                 Vec3Base<T>, Vec4Base<T>>>(
              vals...) {}

    void copyFrom(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            (*this)[i] = other[i];
        }
    }

    T &operator[](size_t i) { return ((T *)&this->x)[i]; }
    const T &operator[](size_t i) const { return ((T *)&this->x)[i]; }

    bool operator==(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            if ((*this)[i] != other[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            if ((*this)[i] != other[i]) {
                return true;
            }
        }
        return false;
    }

    void operator+=(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            (*this)[i] += other[i];
        }
    }
    void operator+=(T other) {
        for (size_t i = 0; i < N; ++i) {
            (*this)[i] += other;
        }
    }
    Self operator+(const Self &other) const {
        Self result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = (*this)[i] + other[i];
        }
        return result;
    }

    void operator-=(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            (*this)[i] -= other[i];
        }
    }
    void operator-=(T other) {
        for (size_t i = 0; i < N; ++i) {
            (*this)[i] -= other;
        }
    }
    Self operator-(const Self &other) const {
        Self result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = (*this)[i] - other[i];
        }
        return result;
    }

    void operator*=(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            (*this)[i] *= other[i];
        }
    }
    void operator*=(T other) {
        for (size_t i = 0; i < N; ++i) {
            (*this)[i] *= other;
        }
    }
    Self operator*(const Self &other) const {
        Self result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = (*this)[i] * other[i];
        }
        return result;
    }
    Self operator*(T val) const {
        Self result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = (*this)[i] * val;
        }
        return result;
    }

    void operator/=(const Self &other) {
        for (size_t i = 0; i < N; ++i) {
            (*this)[i] /= other[i];
        }
    }
    void operator/=(T other) {
        for (size_t i = 0; i < N; ++i) {
            (*this)[i] /= other;
        }
    }
    Self operator/(const Self &other) const {
        Self result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = (*this)[i] / other[i];
        }
        return result;
    }

    float distance(const Self &other) { return sqrt(squaredDistance(other)); }

    float squaredDistance(const Self &other) {
        float dist = 0;
        for (size_t i = 0; i < N; ++i) {
            dist += pow((*this)[i] - other[i], 2);
        }
        return dist;
    }

    void invert() {
        for (size_t i = 0; i < N; ++i) {
            (*this)[i] = -(*this)[i];
        }
    }

    float length() { return distance(Self()); }

    void normalize(float size = 1) {
        float normal = size / length();
        for (size_t i = 0; i < N; ++i) {
            (*this)[i] *= normal;
        }
    }
};

template <typename T> struct Vec2 : public Vec<T, 2, Vec2<T>> {
    Vec2(T x = 0, T y = 0) : Vec<T, 2, Vec2<T>>{x, y} {}
    Vec2<T> perpendicular() { return Vec2<T>(-this->y, this->x); }
    float zcross(const Vec2<T> &other) {
        return (this->x * other.y) - (this->y * other.x);
    }
    float dot(const Vec2<T> &other) {
        return (this->x * other.x) + (this->y * other.y);
    }
    void setTo(T x = 0, T y = 0) {
        this->x = x;
        this->y = y;
    }
};

template <typename T> struct Vec3 : public Vec<T, 3, Vec3<T>> {
    Vec3(T x = 0, T y = 0, T z = 0) : Vec<T, 3, Vec3<T>>{x, y, z} {}
    void setTo(T x = 0, T y = 0, T z = 0) {
        this->x = x;
        this->y = y;
        this->z = z;
    }
};

template <typename T> struct Vec4 : public Vec<T, 4, Vec4<T>> {
    Vec4(T x = 0, T y = 0, T z = 0, T w = 0) : Vec<T, 4, Vec4<T>>{x, y, z, w} {}
    void setTo(T x = 0, T y = 0, T z = 0, T w = 0) {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }
};

using Vec2f = Vec2<float>;
using Vec3f = Vec3<float>;
using Vec4f = Vec4<float>;
using Vec2i = Vec2<int>;
using Vec3i = Vec3<int>;
using Vec4i = Vec4<int>;

}
