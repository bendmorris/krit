#ifndef KRIT_UTILS_COLOR
#define KRIT_UTILS_COLOR

#include "krit/math/Vec.h"
#include <cstdint>

namespace krit {

// struct Color : public Vec4 {
//     float &r() { return (*this)[0]; }
//     float &g() { return (*this)[1]; }
//     float &b() { return (*this)[2]; }
//     float &a() { return (*this)[3]; }

//     setTo(float r, float g, float b, float a);
//     setToHex(uint32_t i);
// };

struct Color {
    static Color black(float alpha = 1.0) {
        return Color(0.0, 0.0, 0.0, alpha);
    }
    static Color white(float alpha = 1.0) {
        return Color(1.0, 1.0, 1.0, alpha);
    }

    float r = 0.0;
    float g = 0.0;
    float b = 0.0;
    float a = 0.0;

    Color() {}
    Color(const Color &) = default;
    Color(float r, float g, float b, float a = 1.0) : r(r), g(g), b(b), a(a) {}
    Color(unsigned c)
        : r(((c & 0xff0000) >> 16) / 255.0), g(((c & 0xff00) >> 8) / 255.0),
          b((c & 0xff) / 255.0), a(((c & 0xff000000) >> 24) / 255.0) {}
    Color(unsigned c, float a)
        : r(((c & 0xff0000) >> 16) / 255.0), g(((c & 0xff00) >> 8) / 255.0),
          b((c & 0xff) / 255.0), a(a) {}

    Color &operator=(unsigned c) {
        r = ((c & 0xff0000) >> 16) / 255.0;
        g = ((c & 0xff00) >> 8) / 255.0;
        b = (c & 0xff) / 255.0;
        return *this;
    }

    void setTo(const Color &c) { *this = c; }
    void setTo(unsigned c) { *this = c; }
    void setTo(float r, float g, float b) {
        this->r = r;
        this->g = g;
        this->b = b;
    }
    void setTo(float r, float g, float b, float a) {
        this->r = r;
        this->g = g;
        this->b = b;
        this->a = a;
    }

    bool operator==(const Color &other) {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
    bool operator!=(const Color &other) { return !(*this == other); }

    Color operator*(const Color &other) {
        return Color(this->r * other.r, this->g * other.g, this->b * other.b,
                     this->a * other.a);
    }
    Color operator*(float t) { return Color(r * t, g * t, b * t, a * t); }
    Color operator+(const Color &other) {
        return Color(r + other.r, g + other.g, b + other.b, a + other.a);
    }
    Color operator-(const Color &other) {
        return Color(r - other.r, g - other.g, b - other.b, a - other.a);
    }

    void lerpInPlace(const Color &other, float mix) {
        *this = this->lerp(other, mix);
    }
    void lerpInPlace(unsigned c, float mix) { *this = this->lerp(c, mix); }

    Color lerp(const Color &other, float mix) {
        if (mix <= 0) {
            return *this;
        } else if (mix >= 1) {
            return other;
        } else {
            float rm = 1 - mix;
            return Color(r * rm + other.r * mix, g * rm + other.g * mix,
                         b * rm + other.b * mix, a * rm + other.a * mix);
        }
    }

    Color multiply(const Color &other) {
        return Color(r * other.r, g * other.g, b * other.b, a * other.a);
    }

    Color withAlpha(float alpha) { return Color(r, g, b, alpha); }

    uint32_t rgb() {
        return (static_cast<uint32_t>(r * 0xff) << 16) |
               (static_cast<uint32_t>(g * 0xff) << 8) |
               static_cast<uint32_t>(b * 0xff);
    }

    uint32_t bgra() {
        return (static_cast<uint32_t>(b * 0xff) << 24) |
               (static_cast<uint32_t>(g * 0xff) << 16) |
               (static_cast<uint32_t>(r * 0xff) << 8) |
               static_cast<uint32_t>(a * 0xff);
    }

    uint32_t rgba() {
        return (static_cast<uint32_t>(r * 0xff) << 24) |
               (static_cast<uint32_t>(g * 0xff) << 16) |
               (static_cast<uint32_t>(b * 0xff) << 8) |
               static_cast<uint32_t>(a * 0xff);
    }

    uint32_t argb() {
        return (static_cast<uint32_t>(a * 0xff) << 24) |
               (static_cast<uint32_t>(r * 0xff) << 16) |
               (static_cast<uint32_t>(g * 0xff) << 8) |
               static_cast<uint32_t>(b * 0xff);
    }

    uint32_t abgr() {
        return (static_cast<uint32_t>(a * 0xff) << 24) |
               (static_cast<uint32_t>(b * 0xff) << 16) |
               (static_cast<uint32_t>(g * 0xff) << 8) |
               static_cast<uint32_t>(r * 0xff);
    }
};

}

#endif
