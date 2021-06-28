#ifndef KRIT_RENDER
#define KRIT_RENDER

#include <GL/glew.h>

namespace krit {

typedef GLfloat RenderFloat;

void checkForGlErrors(const char *fmt, ...);
void printProgramInfoLog(GLuint);
void printShaderInfoLog(GLuint);

}

#endif
