#include "krit/render/SpriteShader.h"
#include "krit/Engine.h"
#include "krit/TaskManager.h"
#include "krit/math/Point.h"
#include "krit/math/Triangle.h"
#include "krit/render/DrawCall.h"
#include "krit/render/DrawCommand.h"
#include "krit/render/Shader.h"
#include "krit/utils/Color.h"
#include "krit/utils/Panic.h"

namespace krit {

SpriteShader::SpriteShader(Shader &shader) : ShaderInstance(shader) {
    matrixIndex = shader.getUniformLocation("uMatrix");
}

void SpriteShader::bind(RenderContext &ctx) { ShaderInstance::bind(ctx); }

void SpriteShader::unbind() { ShaderInstance::unbind(); }

}
