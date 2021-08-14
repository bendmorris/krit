#include "krit/render/SceneShader.h"
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

SceneShader::SceneShader(Shader &shader) : ShaderInstance(shader) {
    if (shader.texCoordIndex > -1 || shader.colorIndex > -1) {
        panic("SceneShader does not support aTexCoord or aColor attributes");
    }
    matrixIndex = shader.getUniformLocation("uMatrix");
}

void SceneShader::bind(RenderContext &ctx) { ShaderInstance::bind(ctx); }

void SceneShader::unbind() { ShaderInstance::unbind(); }

void SceneShader::prepare(RenderContext &ctx, RenderFloat *buffer) {
    checkForGlErrors("vertex attrib pointer");
    int stride = shader.bytesPerVertex;
    int i = 0;
    RenderFloat *origin = nullptr;
    buffer[i++] = -1;
    buffer[i++] = -1;
    buffer[i++] = 1;
    buffer[i++] = -1;
    buffer[i++] = -1;
    buffer[i++] = 1;
    buffer[i++] = -1;
    buffer[i++] = 1;
    buffer[i++] = 1;
    buffer[i++] = -1;
    buffer[i++] = 1;
    buffer[i++] = 1;
    glBufferSubData(GL_ARRAY_BUFFER, 0, i * sizeof(RenderFloat), buffer);
    checkForGlErrors("SceneShader bufferSubData");
    glVertexAttribPointer(shader.positionIndex, 2, GL_FLOAT, GL_FALSE, stride,
                          origin);
    checkForGlErrors("SceneShader attrib pointers");
}

}
