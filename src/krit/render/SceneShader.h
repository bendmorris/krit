#ifndef KRIT_RENDER_SCENESHADER
#define KRIT_RENDER_SCENESHADER

#include "krit/render/BlendMode.h"
#include "krit/render/RenderContext.h"
#include "krit/render/Shader.h"

namespace krit {

struct SceneShader : public ShaderInstance {
    GLint matrixIndex;
    BlendMode blend = BlendMode::Alpha;

    SceneShader(Shader &shader);
    SceneShader(Shader *shader) : SceneShader(*shader) {}
    virtual ~SceneShader() {}

    virtual void bind() override;
    virtual void unbind() override;
};

}

#endif
