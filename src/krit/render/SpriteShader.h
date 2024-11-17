#ifndef KRIT_RENDER_SPRITESHADER
#define KRIT_RENDER_SPRITESHADER

#include "krit/render/Shader.h"

namespace krit {

struct SpriteShader : public ShaderInstance {
    GLint matrixIndex;

    SpriteShader(Shader &shader);
    SpriteShader(Shader *shader) : SpriteShader(*shader) {}
    virtual ~SpriteShader() {}

    virtual void bind() override;
    virtual void unbind() override;

    void prepareVertex(RenderFloat *buf, RenderFloat x, RenderFloat y, RenderFloat z,
                       RenderFloat tx, RenderFloat ty, const Color &c);
};

}

#endif