#ifndef KRIT_RENDER_SHADER
#define KRIT_RENDER_SHADER

#include "krit/render/Gl.h"
#include "krit/render/RenderContext.h"
#include "krit/render/Uniform.h"
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
    GLint location;
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

    template <typename T>
    Shader(const T &vertexSource,
           std::shared_ptr<std::string_view> fragmentSource)
        : vertexSource(vertexSource), fragmentSource(*fragmentSource) {}

    template <typename T>
    Shader(std::shared_ptr<std::string_view> vertexSource,
           const T &fragmentSource)
        : vertexSource(*vertexSource), fragmentSource(fragmentSource) {}

    template <typename T, typename U>
    Shader(const T &vertexSource, const U &fragmentSource)
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

struct UniformValueInfo {
    int location = 0;
    UniformValue value;
};

struct ShaderInstance {
    Shader &shader;
    std::vector<UniformValueInfo> uniforms;

    ShaderInstance(Shader &shader);
    ShaderInstance(Shader *shader) : ShaderInstance(*shader) {}

    virtual void init() { shader.init(); }
    virtual void bind(RenderContext &ctx);
    virtual void unbind() { shader.unbind(); }

    void setUniform(const std::string &name, UniformValue value);

    void clear() {
        for (size_t i = 0; i < uniforms.size(); ++i) {
            uniforms[i].location = -1;
            uniforms[i].value = UniformValue();
        }
    }
};

}

#endif
