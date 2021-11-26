#ifndef KRIT_RENDER
#define KRIT_RENDER

#if KRIT_USE_GLEW
#include <GL/glew.h>
#else
#include <GLES3/gl3.h>
#endif

#define KRIT_GL_TEX_PARAM GL_TEXTURE_2D
#define KRIT_GL_TEX_IMAGE_2D(width, height, format) glTexImage2D(KRIT_GL_TEX_PARAM, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, 0)

namespace krit {

typedef GLfloat RenderFloat;

#if KRIT_RELEASE
#define checkForGlErrors(...)
#else
#define STRINGIZE_DETAIL(x) #x
#define STRINGIZE(x) STRINGIZE_DETAIL(x)
#define checkForGlErrors(...)                                                  \
    _checkForGlErrors(__FILE__ ":" STRINGIZE(__LINE__) ": " __VA_ARGS__)
void _checkForGlErrors(const char *fmt, ...);
#endif

void printProgramInfoLog(GLuint);
void printShaderInfoLog(GLuint);

}

#endif
