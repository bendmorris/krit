#ifndef KRIT_RENDER_SPRITESHADER
#define KRIT_RENDER_SPRITESHADER

#include "krit/render/Shader.h"

namespace krit {

struct SpriteShader : public ShaderInstance {
    GLint matrixIndex;

    SpriteShader(Shader &shader);
    SpriteShader(Shader *shader) : SpriteShader(*shader) {}
    virtual ~SpriteShader() {}

    virtual void bind(RenderContext &ctx) override;
    virtual void unbind() override;

    void prepareVertex(RenderFloat *buf, RenderFloat x, RenderFloat y,
                       RenderFloat tx, RenderFloat ty, const Color &c) {
        bool hasTexCoord = shader.texCoordIndex > -1;
        bool hasColor = shader.colorIndex > -1;
        size_t i = 0;
        if (hasTexCoord && hasColor) {
            buf[i++] = x;
            buf[i++] = y;
            buf[i++] = tx;
            buf[i++] = ty;
            buf[i++] = c.r;
            buf[i++] = c.g;
            buf[i++] = c.b;
            buf[i++] = c.a;
        } else if (hasColor) {
            buf[i++] = x;
            buf[i++] = y;
            buf[i++] = c.r;
            buf[i++] = c.g;
            buf[i++] = c.b;
            buf[i++] = c.a;
        } else if (hasTexCoord) {
            buf[i++] = x;
            buf[i++] = y;
            buf[i++] = tx;
            buf[i++] = ty;
        } else {
            buf[i++] = x;
            buf[i++] = y;
        }
    }
};

}

#endif