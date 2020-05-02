#ifndef KRIT_RENDER_SHADER
#define KRIT_RENDER_SHADER

#include "krit/render/Gl.h"
#include "GLES3/gl3.h"
#include <cstdlib>
#include <cstdio>
#include <unordered_map>

using namespace krit;

namespace krit {

class DrawCall;

struct Shader {
    GLuint program;
    const char *vertexSource;
    const char *fragmentSource;
    GLint positionIndex;
    GLint texCoordIndex;
    std::unordered_map<std::string, GLint> uniformLocations;

    Shader(const char *vertexSource, const char *fragmentSource):
        vertexSource(vertexSource),
        fragmentSource(fragmentSource) {}

    ~Shader();

    GLint getUniformLocation(const std::string&);
    virtual void init();
    virtual void bind();
    virtual void unbind();
};

struct SpriteShader: public Shader {
    static SpriteShader defaultTextureSpriteShader;
    static SpriteShader defaultColorSpriteShader;

    GLint matrixIndex;
    GLint colorIndex;

    SpriteShader(const char *v, const char *f): Shader(v, f) {}

    virtual void init() override;
    virtual void bindOrtho(GLfloat *matrix);
    virtual void unbind() override;
    virtual void prepare(DrawCall *drawCall, RenderFloat *buffer);

    virtual int bytesPerVertex() {
        return (2 /* position */ + 1 /* color */ + (this->texCoordIndex == -1 ? 0 : 2)) * sizeof(GLfloat);
    }
};

}

#endif
