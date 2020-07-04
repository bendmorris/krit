    
#ifndef KRIT_PARTICLES_PARTICLEPROPERTIES
#define KRIT_PARTICLES_PARTICLEPROPERTIES

#include "krit/particles/Interpolation.h"
#include "krit/utils/Color.h"
#include "krit/Utils.h"
#include <string>
#include <type_traits>
#include <unordered_map>

namespace krit {

struct InterpolationFunction;

struct PropertyPair {
    float first = 0;
    float second = 0;
};

using StaticProperty = std::pair<float, float>;

struct DynamicProperty {
    InterpolationFunction *f = &InterpolationFunction::linearInterpolation;
    StaticProperty start;
    StaticProperty end;

    DynamicProperty(float a, float b): start(a, a), end(b, b) {}
    DynamicProperty(float a, float b, float c, float d): start(a, b), end(c, d) {}
};

struct ResolvedDynamicProperty {
    InterpolationFunction *f = &InterpolationFunction::linearInterpolation;
    float start = 0;
    float end = 0;

    ResolvedDynamicProperty() {}
    ResolvedDynamicProperty(float a, float b): start(a), end(b) {}
};

struct ResolvedProperties {
    float originX;
    float originY;
    float direction;
    float duration;
    ResolvedDynamicProperty red;
    ResolvedDynamicProperty green;
    ResolvedDynamicProperty blue;
    ResolvedDynamicProperty alpha;
    ResolvedDynamicProperty scale;
    ResolvedDynamicProperty rotation;
    ResolvedDynamicProperty distance;
    ResolvedDynamicProperty orthoDistance;

    float evaluate(const ResolvedDynamicProperty &prop, float t) {
        float interp = prop.f->evaluate(t);
        return prop.start * (1 - interp) + prop.end * interp;
    }
};

struct UnresolvedProperties {
    StaticProperty originX;
    StaticProperty originY;
    StaticProperty direction;
    StaticProperty duration;
    DynamicProperty red;
    DynamicProperty green;
    DynamicProperty blue;
    DynamicProperty alpha;
    DynamicProperty scale;
    DynamicProperty rotation;
    DynamicProperty distance;
    DynamicProperty orthoDistance;

    UnresolvedProperties():
        originX(0, 0),
        originY(0, 0),
        direction(0, 0),
        duration(0, 0),
        red(1, 1),
        green(1, 1),
        blue(1, 1),
        alpha(1, 1),
        scale(1, 1),
        rotation(0, 0),
        distance(0, 0),
        orthoDistance(0, 0)
    {}

    static void add(StaticProperty &lhs, const StaticProperty &rhs) {
        lhs.first += rhs.first;
        lhs.second += rhs.second;
    }

    static void add(DynamicProperty &lhs, const DynamicProperty &rhs) {
        add(lhs.start, rhs.start);
        add(lhs.end, rhs.end);
    }

    static void resolve(const StaticProperty &from, float &to) {
        std::uniform_real_distribution<float> r(0, 1);
        float t = r(rng);
        to = from.first * (1 - t) + from.second * t;
    }

    static void resolve(const DynamicProperty &from, ResolvedDynamicProperty &to) {
        std::uniform_real_distribution<float> r(0, 1);
        float t = r(rng), t2 = r(rng);
        to.f = from.f;
        to.start = from.start.first * (1 - t) + from.start.second * t;
        to.end = from.end.first * (1 - t2) + from.end.second * t2;
    }

    UnresolvedProperties &set(DynamicProperty &prop, float val) {
        prop.start.first = prop.start.second = prop.end.first = prop.end.second = val;
        return *this;
    }

    UnresolvedProperties &set(DynamicProperty &prop, float startVal, float endVal) {
        prop.start.first = prop.start.second = startVal;
        prop.end.first = prop.end.second = endVal;
        return *this;
    }

    UnresolvedProperties &set(DynamicProperty &prop, float startVal1, float startVal2, float endVal1, float endVal2) {
        prop.start.first = startVal1;
        prop.start.second = startVal2;
        prop.end.first = endVal1;
        prop.end.second = endVal2;
        return *this;
    }

    UnresolvedProperties &set(StaticProperty &prop, float val) {
        prop.first = prop.second = val;
        return *this;
    }

    UnresolvedProperties &set(StaticProperty &prop, float val1, float val2) {
        prop.first = val1;
        prop.second = val2;
        return *this;
    }

    #define SETTER(x, y) \
        UnresolvedProperties &set##x(float a) { return set(y, a); } \
        UnresolvedProperties &set##x(float a, float b) { return set(y, a, b); }
    SETTER(OriginX, originX)
    SETTER(OriginY, originY)
    SETTER(Direction, direction)
    SETTER(Duration, duration)
    #undef SETTER
    #define SETTER(x, y) \
        UnresolvedProperties &set##x(float a) { return set(y, a); } \
        UnresolvedProperties &set##x(float a, float b) { return set(y, a, b); } \
        UnresolvedProperties &set##x(float a, float b, float c, float d) { return set(y, a, b, c, d); }
    SETTER(Red, red)
    SETTER(Green, green)
    SETTER(Blue, blue)
    SETTER(Alpha, alpha)
    SETTER(Scale, scale)
    SETTER(Rotation, rotation)
    SETTER(Distance, distance)
    SETTER(OrthoDistance, orthoDistance)
    #undef SETTER
    
    UnresolvedProperties &setColor(Color v) { setRed(v.r); setBlue(v.b); setGreen(v.g); setAlpha(v.a); return *this; }
    UnresolvedProperties &setColor(Color v1, Color v2) {
        setRed(v1.r, v2.r); 
        setBlue(v1.b, v2.b); 
        setGreen(v1.g, v2.g); 
        setAlpha(v1.a, v2.a);
        return *this;
    }
    UnresolvedProperties &setColor(Color v1, Color v2, Color v3, Color v4) {
        setRed(v1.r, v2.r, v3.r, v4.r); 
        setBlue(v1.b, v2.b, v3.b, v4.b); 
        setGreen(v1.g, v2.g, v3.g, v4.g);
        setAlpha(v1.a, v2.a, v3.a, v4.a);
        return *this;
    }

    UnresolvedProperties &operator+=(const UnresolvedProperties &rhs) {
        add(originX, rhs.originX);
        add(originY, rhs.originY);
        add(direction, rhs.direction);
        add(duration, rhs.duration);
        add(red, rhs.red);
        add(green, rhs.green);
        add(blue, rhs.blue);
        add(alpha, rhs.alpha);
        add(scale, rhs.scale);
        add(rotation, rhs.rotation);
        add(distance, rhs.distance);
        add(orthoDistance, rhs.orthoDistance);
        return *this;
    }

    void resolve(ResolvedProperties &to) {
        resolve(originX, to.originX);
        resolve(originY, to.originY);
        resolve(direction, to.direction);
        resolve(duration, to.duration);
        resolve(red, to.red);
        resolve(green, to.green);
        resolve(blue, to.blue);
        resolve(alpha, to.alpha);
        resolve(scale, to.scale);
        resolve(rotation, to.rotation);
        resolve(distance, to.distance);
        resolve(orthoDistance, to.orthoDistance);
    }
};

}

#endif