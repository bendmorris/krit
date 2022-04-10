#ifndef KRIT_RENDER_UNIFORM
#define KRIT_RENDER_UNIFORM

#include "krit/render/ImageData.h"
#include "krit/utils/Slice.h"

namespace krit {

struct FrameBuffer;

enum UniformValueType {
    UniformEmpty,
    UniformInt,
    UniformFloat,
    UniformFloatPtr,
    UniformVec2,
    UniformVec3,
    UniformVec4,
    UniformFloat1v,
    UniformFloat2v,
    UniformFloat3v,
    UniformFloat4v,
    UniformTexture,
    UniformFbTexture,
};

struct UniformValue {
    UniformValueType type;

    union {
        int intValue;
        float floatValue;
        float vec2Value[2];
        float vec3Value[3];
        float vec4Value[4];
        Slice<float> floatData;
        ImageData *imgPtrValue;
        FrameBuffer *fbPtrValue;
        float *floatPtrValue;
    };

    UniformValue() : type(UniformEmpty), intValue(0) {}
    UniformValue(int f) : type(UniformInt), intValue(f) {}
    UniformValue(float f) : type(UniformFloat), floatValue(f) {}
    UniformValue(float *f) : type(UniformFloatPtr), floatPtrValue(f) {}
    UniformValue(float a, float b) : type(UniformVec2), vec2Value{a, b} {}
    UniformValue(float a, float b, float c)
        : type(UniformVec3), vec3Value{a, b, c} {}
    UniformValue(float a, float b, float c, float d)
        : type(UniformVec4), vec4Value{a, b, c, d} {}
    UniformValue(size_t N, size_t c, float *v)
        : type(static_cast<UniformValueType>(UniformFloat1v + N - 1)),
          floatData(v, c) {}
    UniformValue(ImageData &img)
        : type(UniformTexture), imgPtrValue(&img) {}
    UniformValue(FrameBuffer &fb)
        : type(UniformFbTexture), fbPtrValue(&fb) {}
    UniformValue(const Color &c)
        : type(UniformVec4), vec4Value{c.r, c.g, c.b, c.a} {}
    // template <typename T>
    // UniformValue(const BasePoint<T> &p)
    //     : type(UniformVec2), vec2Value{p.x, p.y} {}
};

}

#endif
