#include <SDL.h>
#include <cstdio>
#include "krit/render/Gl.h"

namespace krit {

void checkForGlErrors(const char *fmt, ...) {
    GLenum err = glGetError();
    if (err) {
        va_list args;
        va_start(args, fmt);
        fprintf(stderr, "GL error: %i ", err);
        vfprintf(stderr, fmt, args);
        fputs("\n", stderr);
    }
}

void printProgramInfoLog(GLuint program) {
    if (glIsProgram(program)) {
        int len;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &len);
        if (len > 0) {
            GLchar *s = static_cast<char*>(malloc(len));
            glGetProgramInfoLog(program, len, &len, s);
            puts(s);
            free(s);
        } else {
            puts("no log");
        }
    } else {
        printf("%i is not a program\n", program);
    }
}

void printShaderInfoLog(GLuint shader) {
    if (glIsShader(shader)) {
        int len;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &len);
        if (len > 0) {
            GLchar *s = static_cast<GLchar*>(malloc(len));
            glGetShaderInfoLog(shader, len, &len, s);
            puts(s);
            free(s);
        } else {
            puts("no log");
        }
    } else {
        printf("%i is not a shader\n", shader);
    }
}

}
