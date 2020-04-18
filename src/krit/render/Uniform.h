#ifndef KRIT_RENDER_UNIFORM
#define KRIT_RENDER_UNIFORM

namespace krit {

enum UniformValueType {
    UniformInt,
    UniformFloat,
    UniformVec2,
    UniformVec3,
    UniformVec4,
    UniformFloat1v,
    UniformFloat2v,
    UniformFloat3v,
    UniformFloat4v,
    UniformTexture,
};

struct UniformValue {
    UniformValueType type;

    union {
        int intValue;
        float floatValue;
        float vec2Value[2];
        float vec3Value[3];
        float vec4Value[4];
        std::pair<size_t, float*> floatData;
    };

    UniformValue(int f): type(UniformInt), intValue(f) {}
    UniformValue(float f): type(UniformFloat), floatValue(f) {}
    UniformValue(float a, float b): type(UniformVec2), vec2Value {a, b} {}
    UniformValue(float a, float b, float c): type(UniformVec3), vec3Value {a, b, c} {}
    UniformValue(float a, float b, float c, float d): type(UniformVec4), vec4Value {a, b, c, d} {}
    UniformValue(size_t N, size_t c, float *v): type(static_cast<UniformValueType>(UniformFloat1v + N - 1)), floatData(make_pair(c, v)) {}
    UniformValue(ImageData img): type(UniformTexture), intValue(img.texture) {}
};

}

#endif
