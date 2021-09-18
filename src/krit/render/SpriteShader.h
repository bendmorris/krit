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
    void prepare(RenderContext &ctx, DrawCall *drawCall, RenderFloat *buffer);
};

}

#endif