#include <stdint.h>
#include <utility>
#include <vector>

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
struct RenderContext;

Shader::~Shader() {
    GLuint program = this->program;
    if (program) {
        TaskManager::instance->pushRender(
            [program](RenderContext &) { glDeleteProgram(program); });
    }
}

void Shader::init() {
    if (!this->program) {
        GLint status;

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        {
            const char *s = this->vertexSource.c_str();
            glShaderSource(vertexShader, 1, &s, nullptr);
            glCompileShader(vertexShader);
            glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
            if (status == GL_FALSE) {
                printShaderInfoLog(vertexShader);
                panic("failed to compile vertex shader:\n\n%s",
                      vertexSource.c_str());
            }
            checkForGlErrors("compile vertex");
        }

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        {
            const char *s = this->fragmentSource.c_str();
            glShaderSource(fragmentShader, 1, &s, nullptr);
            glCompileShader(fragmentShader);
            glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
            if (status == GL_FALSE) {
                printShaderInfoLog(fragmentShader);
                panic("failed to compile fragment shader:\n\n%s",
                      vertexSource.c_str());
            }
            checkForGlErrors("compile fragment");
        }

        this->program = glCreateProgram();
        checkForGlErrors("create program");
        glAttachShader(this->program, vertexShader);
        glAttachShader(this->program, fragmentShader);
        glLinkProgram(this->program);
        glGetProgramiv(this->program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            printProgramInfoLog(program);
            panic("failed to link program");
        }
        checkForGlErrors("link program");

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        this->positionIndex = glGetAttribLocation(program, "aPosition");
        checkForGlErrors("positionIndex");
        this->texCoordIndex = glGetAttribLocation(program, "aTexCoord");
        checkForGlErrors("texCoordIndex");
        this->colorIndex = glGetAttribLocation(program, "aColor");
        checkForGlErrors("colorIndex");

        bytesPerVertex =
            (2 /* position */ + (colorIndex == -1 ? 0 : 1) /* color */ +
             (this->texCoordIndex == -1 ? 0 : 2)) *
            sizeof(GLfloat);

        // get uniform info
        GLint count, size;
        GLenum type;

        const GLsizei bufSize = 128;
        GLchar name[bufSize];
        GLsizei length;
        glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);

        uniforms.resize(count);
        for (int i = 0; i < count; i++) {
            glGetActiveUniform(program, (GLuint)i, bufSize, &length, &size,
                               &type, name);
            GLint location = glGetUniformLocation(program, name);

            uniforms[i] = (UniformInfo){.name = std::string(name, length),
                                        .location = location};
            // printf("%s = %i\n", name, location);
        }
        checkForGlErrors("uniform info");
    }
}

void Shader::bind(RenderContext &ctx) {
    this->init();
    checkForGlErrors("shader init");
    glUseProgram(this->program);
    checkForGlErrors("glUseProgram");
    if (positionIndex > -1) {
        glEnableVertexAttribArray(positionIndex);
        checkForGlErrors("bind positionIndex");
    }
    if (texCoordIndex > -1) {
        glEnableVertexAttribArray(texCoordIndex);
        checkForGlErrors("bind texCoordIndex");
    }
    if (colorIndex > -1) {
        glEnableVertexAttribArray(colorIndex);
        checkForGlErrors("bind colorIndex");
    }
}

void Shader::unbind() {
    if (positionIndex > -1) {
        glDisableVertexAttribArray(positionIndex);
    }
    if (texCoordIndex > -1) {
        glDisableVertexAttribArray(texCoordIndex);
    }
    if (colorIndex > -1) {
        glDisableVertexAttribArray(colorIndex);
    }
    glUseProgram(0);
    checkForGlErrors("unbind");
}

GLint Shader::getUniformLocation(const std::string &name) {
    this->init();
    for (size_t i = 0; i < this->uniforms.size(); ++i) {
        if (this->uniforms[i].name == name) {
            return this->uniforms[i].location;
        }
    }
    return -1;
}

ShaderInstance::ShaderInstance(Shader &shader) : shader(shader) {
    shader.init();
    uniforms.resize(shader.uniforms.size());
}

void ShaderInstance::setUniform(const std::string &name, UniformValue value) {
    shader.init();
    if (uniforms.size() < shader.uniforms.size()) {
        uniforms.resize(shader.uniforms.size());
    }
    for (size_t i = 0; i < shader.uniforms.size(); ++i) {
        if (shader.uniforms[i].name == name) {
            uniforms[i] = (UniformValueInfo){
                .location = shader.uniforms[i].location, .value = value};
            break;
        }
    }
}

void ShaderInstance::bind(RenderContext &ctx) {
    shader.bind(ctx);
    int textureIndex = 1;
    for (size_t idx = 0; idx < uniforms.size(); ++idx) {
        auto &info = uniforms[idx];
        int i = info.location;
        if (i < 0) {
            continue;
        }
        auto &uniform = info.value;
        switch (uniform.type) {
            case UniformEmpty: {
                // automatic uniforms
                auto &uniformName = shader.uniforms[idx].name;
                i = shader.uniforms[idx].location;
                if (uniformName == "uTime") {
                    glUniform1f(i, ctx.engine->elapsed);
                    checkForGlErrors("glUniform1f");
                } else if (uniformName == "uResolution") {
                    IntDimensions size = ctx.window->size();
                    glUniform2f(i, size.x, size.y);
                    checkForGlErrors("glUniform2f");
                } else if (uniformName == "uSize") {
                    IntDimensions size = ctx.size();
                    glUniform2f(i, size.x, size.y);
                    checkForGlErrors("glUniform2f");
                } else if (uniformName == "uScale") {
                    ScaleFactor &scale = ctx.camera->scale;
                    glUniform2f(i, scale.x, scale.y);
                    checkForGlErrors("glUniform2f");
                }
                break;
            }
            case UniformInt: {
                glUniform1i(i, uniform.intValue);
                checkForGlErrors("glUniform1i %i", uniform.intValue);
                break;
            }
            case UniformTexture: {
                glActiveTexture(GL_TEXTURE0 + textureIndex);
                glBindTexture(GL_TEXTURE_2D, uniform.intValue);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                                GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                                GL_LINEAR);
                checkForGlErrors("glBindTexture");
                glUniform1i(i, textureIndex++);
                checkForGlErrors("glUniform1i %i", textureIndex - 1);
                break;
            }
            case UniformFbTexture: {
                glActiveTexture(GL_TEXTURE0 + textureIndex);
                glBindTexture(GL_TEXTURE_2D,
                              uniform.fbPtrValue->getTexture().texture);
                checkForGlErrors("glBindTexture");
                glUniform1i(i, textureIndex++);
                checkForGlErrors("glUniform1i %i", textureIndex - 1);
                break;
            }
            case UniformFloat: {
                glUniform1f(i, uniform.floatValue);
                checkForGlErrors("glUniform1f");
                break;
            }
            case UniformVec2: {
                glUniform2f(i, uniform.vec2Value[0], uniform.vec2Value[1]);
                checkForGlErrors("glUniform2f");
                break;
            }
            case UniformVec3: {
                glUniform3f(i, uniform.vec3Value[0], uniform.vec3Value[1],
                            uniform.vec3Value[2]);
                checkForGlErrors("glUniform3f");
                break;
            }
            case UniformVec4: {
                glUniform4f(i, uniform.vec4Value[0], uniform.vec4Value[1],
                            uniform.vec4Value[2], uniform.vec4Value[3]);
                checkForGlErrors("glUniform4f");
                break;
            }
            case UniformFloat1v: {
                glUniform1fv(i, uniform.floatData.length,
                             uniform.floatData.data);
                checkForGlErrors("glUniform1fv");
                break;
            }
            case UniformFloat2v: {
                glUniform2fv(i, uniform.floatData.length,
                             uniform.floatData.data);
                checkForGlErrors("glUniform2fv");
                break;
            }
            case UniformFloat3v: {
                glUniform3fv(i, uniform.floatData.length,
                             uniform.floatData.data);
                checkForGlErrors("glUniform3fv");
                break;
            }
            case UniformFloat4v: {
                glUniform4fv(i, uniform.floatData.length,
                             uniform.floatData.data);
                checkForGlErrors("glUniform4fv");
                break;
            }
            default: {
                panic("unknown uniform type: %i\n", uniform.type);
            }
        }
    }
}

}
