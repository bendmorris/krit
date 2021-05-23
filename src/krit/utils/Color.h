#ifndef KRIT_UTILS_COLOR
#define KRIT_UTILS_COLOR

#include <cstdint>

namespace krit {

struct Color {
    static Color black(float alpha = 1.0) { return Color(0.0, 0.0, 0.0, alpha); }
    static Color white(float alpha = 1.0) { return Color(1.0, 1.0, 1.0, alpha); }

    float r = 0.0;
    float g = 0.0;
    float b = 0.0;
    float a = 0.0;

    Color() {}
    Color(const Color&) = default;
    Color(float r, float g, float b, float a = 1.0): r(r), g(g), b(b), a(a) {}
    Color(unsigned c): r(((c & 0xff0000) >> 16) / 255.0), g(((c & 0xff00) >> 8) / 255.0), b((c & 0xff) / 255.0), a(((c & 0xff000000) >> 24) / 255.0) {}
    Color(unsigned c, float a): r(((c & 0xff0000) >> 16) / 255.0), g(((c & 0xff00) >> 8) / 255.0), b((c & 0xff) / 255.0), a(a) {}

    Color &operator=(unsigned c) {
        r = ((c & 0xff0000) >> 16) / 255.0;
        g = ((c & 0xff00) >> 8) / 255.0;
        b = (c & 0xff) / 255.0;
        return *this;
    }

    void setTo(unsigned c) { *this = c; }

    bool operator==(const Color &other) { return r == other.r && g == other.g && b == other.b && a == other.a; }
    bool operator!=(const Color &other) { return !(*this == other); }

    Color operator*(const Color &other) { return Color(this->r * other.r, this->g * other.g, this->b * other.b, this->a * other.a); }

    void lerpInPlace(const Color &other, float mix) { *this = this->lerp(other, mix); }
    void lerpInPlace(unsigned c, float mix) { *this = this->lerp(c, mix); }

    Color lerp(const Color &other, float mix) {
        if (mix <= 0) {
            return *this;
        } else if (mix >= 1) {
            return other;
        } else {
            float rm = 1 - mix;
            return Color(
                r * rm + other.r * mix,
                g * rm + other.g * mix,
                b * rm + other.b * mix,
                a * rm + other.a * mix
            );
        }
    }

    Color multiply(const Color &other) {
        return Color(r * other.r, g * other.g, b * other.b, a * other.a);
    }

    Color withAlpha(float alpha) {
        return Color(r, g, b, alpha);
    }
};

}

#endif
