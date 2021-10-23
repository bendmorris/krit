#ifndef KRIT_RENDER
#define KRIT_RENDER

#if KRIT_USE_GLEW
#include <GL/glew.h>
#else
#include <GLES2/gl2.h>
#endif

namespace krit {

typedef GLfloat RenderFloat;

void checkForGlErrors(const char *fmt, ...);
void printProgramInfoLog(GLuint);
void printShaderInfoLog(GLuint);

}

#endif
