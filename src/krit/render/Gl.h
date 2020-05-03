#ifndef KRIT_RENDER
#define KRIT_RENDER

#include <GL/glew.h>
#include <SDL.h>
#include "SDL2/SDL_opengl.h"
#include <string>

namespace krit {

typedef GLfloat RenderFloat;

void checkForGlErrors(const char *fmt, ...);
void printProgramInfoLog(GLuint);
void printShaderInfoLog(GLuint);

}

#endif
