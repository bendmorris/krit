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

void SceneShader::bind() { ShaderInstance::bind(); }

void SceneShader::unbind() { ShaderInstance::unbind(); }

}
