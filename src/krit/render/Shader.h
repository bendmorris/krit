#ifndef KRIT_RENDER_SHADER
#define KRIT_RENDER_SHADER

#include "krit/render/Gl.h"
#include "krit/render/RenderContext.h"
#include "krit/render/Uniform.h"
#include <GL/glew.h>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace krit {

struct DrawCommandBuffer;
struct DrawCall;

struct UniformInfo {
    std::string name;
};

struct Shader {
    GLuint program = 0;
    std::string vertexSource;
    std::string fragmentSource;
    GLint positionIndex = 0;
    GLint texCoordIndex = 0;
    GLint colorIndex = 0;
    size_t bytesPerVertex = 0;
    std::vector<UniformInfo> uniforms;

    Shader(const std::string &vertexSource, const std::string &fragmentSource)
        : vertexSource(vertexSource), fragmentSource(fragmentSource) {}

    Shader(const char *vertexSource, const char *fragmentSource)
        : vertexSource(vertexSource), fragmentSource(fragmentSource) {}

    Shader(std::string_view vertexSource, std::string_view fragmentSource)
        : vertexSource(vertexSource), fragmentSource(fragmentSource) {}

    Shader(std::shared_ptr<std::string_view> vertexSource,
           std::shared_ptr<std::string_view> fragmentSource)
        : vertexSource(*vertexSource), fragmentSource(*fragmentSource) {}

    virtual ~Shader();

    GLint getUniformLocation(const std::string &);
    virtual void init();
    virtual void bind(RenderContext &ctx);
    virtual void unbind();
};

struct ShaderInstance {
    Shader &shader;
    std::vector<UniformValue> uniforms;

    ShaderInstance(Shader &shader);
    ShaderInstance(Shader *shader) : ShaderInstance(*shader) {}

    virtual void init() { shader.init(); }
    virtual void bind(RenderContext &ctx);
    virtual void unbind() { shader.unbind(); }

    void setUniform(const std::string &name, UniformValue value) {
        shader.init();
        int index = shader.getUniformLocation(name);
        uniforms[index] = value;
    }

    void clear() {
        for (size_t i = 0; i < uniforms.size(); ++i) {
            uniforms[i] = UniformValue();
        }
    }
};

}

#endif
