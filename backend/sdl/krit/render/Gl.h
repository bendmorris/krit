#ifndef KRIT_RENDER
#define KRIT_RENDER

#include "SDL2/SDL.h"
#include <string>

namespace krit {

typedef GLfloat RenderFloat;

void checkForGlErrors(const char *fmt, ...);
void printProgramInfoLog(GLuint);
void printShaderInfoLog(GLuint);

}

#endif
