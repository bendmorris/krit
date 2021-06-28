#ifndef KRIT_RENDER_SHADER
#define KRIT_RENDER_SHADER

#include <GL/glew.h>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>

#include "krit/render/Gl.h"

namespace krit {

struct DrawCommandBuffer;
struct DrawCall;

struct Shader {
    GLuint program = 0;
    const char *vertexSource;
    const char *fragmentSource;
    GLint positionIndex = 0;
    GLint texCoordIndex = 0;
    size_t bytesPerVertex = 0;
    std::unordered_map<std::string, GLint> uniformLocations;

    Shader(const char *vertexSource, const char *fragmentSource)
        : vertexSource(vertexSource), fragmentSource(fragmentSource) {}

    ~Shader();

    GLint getUniformLocation(const std::string &);
    virtual void init();
    virtual void bind();
    virtual void unbind();
};

struct SpriteShader : public Shader {
    GLint matrixIndex;
    GLint colorIndex;

    SpriteShader(const char *v, const char *f) : Shader(v, f) {}

    virtual void init() override;
    virtual void bindOrtho(GLfloat *matrix);
    virtual void unbind() override;
    virtual void prepare(DrawCommandBuffer *buf, DrawCall *drawCall,
                         RenderFloat *buffer);
};

}

#endif
