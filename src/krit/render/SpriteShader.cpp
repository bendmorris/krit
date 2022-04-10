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

void SpriteShader::prepareVertex(RenderFloat *buf, RenderFloat x, RenderFloat y,
                                 RenderFloat z, RenderFloat tx, RenderFloat ty,
                                 const Color &c) {
    bool hasTexCoord = shader.texCoordIndex > -1;
    bool hasColor = shader.colorIndex > -1;
    size_t i = 0;
    if (hasTexCoord && hasColor) {
        buf[i++] = x;
        buf[i++] = y;
        buf[i++] = z;
        buf[i++] = 1.0;
        buf[i++] = tx;
        buf[i++] = ty;
        buf[i++] = c.r;
        buf[i++] = c.g;
        buf[i++] = c.b;
        buf[i++] = c.a;
    } else if (hasColor) {
        buf[i++] = x;
        buf[i++] = y;
        buf[i++] = z;
        buf[i++] = 1.0;
        buf[i++] = c.r;
        buf[i++] = c.g;
        buf[i++] = c.b;
        buf[i++] = c.a;
    } else if (hasTexCoord) {
        buf[i++] = x;
        buf[i++] = y;
        buf[i++] = z;
        buf[i++] = 1.0;
        buf[i++] = tx;
        buf[i++] = ty;
    } else {
        buf[i++] = x;
        buf[i++] = y;
        buf[i++] = z;
        buf[i++] = 1.0;
    }
}

}
