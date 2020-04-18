#include "krit/render/DrawCall.h"
#include "krit/render/Renderer.h"
#include "krit/render/Shader.h"
#include "krit/utils/Panic.h"

using namespace krit;

namespace krit {

static uint32_t colorToInt(Color &c) {
    return (static_cast<uint32_t>(c.a * 0xff) << 24) |
        (static_cast<uint32_t>(c.b * 0xff) << 16) |
        (static_cast<uint32_t>(c.g * 0xff) << 8) |
        (static_cast<uint32_t>(c.r * 0xff))
    ;
}

void Shader::init() {
    if (!this->initialized) {
        GLint status;

        GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexShader, 1, &this->vertexSource, nullptr);
        glCompileShader(vertexShader);
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            printShaderInfoLog(vertexShader);
            panic("failed to compile vertex shader");
        }
        checkForGlErrors("compile vertex");

        GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragmentShader, 1, &this->fragmentSource, nullptr);
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
        if (status == GL_FALSE) {
            printShaderInfoLog(fragmentShader);
            panic("failed to compile fragment shader");
        }
        checkForGlErrors("compile fragment");

        this->program = glCreateProgram();
        checkForGlErrors("create program");
        glAttachShader(this->program, vertexShader);
        glAttachShader(this->program, fragmentShader);
        glLinkProgram(this->program);
        glGetProgramiv(this->program, GL_LINK_STATUS, &status);
        if (status == GL_FALSE) {
            printProgramInfoLog(program);
            panic("failed to link program");
        }
        checkForGlErrors("link program");

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        this->positionIndex = glGetAttribLocation(this->program, "aPosition");
        checkForGlErrors("positionIndex");
        this->texCoordIndex = glGetAttribLocation(this->program, "aTexCoord");
        checkForGlErrors("texCoordIndex");

        this->initialized = true;
    }
}

void Shader::bind() {
    this->init();
    glUseProgram(this->program);
    checkForGlErrors("glUseProgram");
}

void Shader::unbind() {
    glUseProgram(0);
    checkForGlErrors("unbind");
}

GLint Shader::getUniformLocation(const std::string &name) {
    auto it = uniformLocations.find(name);
    if (it != uniformLocations.end()) {
        return it->second;
    }
    GLint loc = glGetUniformLocation(program, name.c_str());
    checkForGlErrors("get uniform location: program %i, %s", program, name.c_str());
    uniformLocations[name] = loc;
    return loc;
}

void SpriteShader::init() {
    if (!this->initialized) {
        Shader::init();
        this->matrixIndex = glGetUniformLocation(this->program, "uMatrix");
        checkForGlErrors("matrixIndex");
        this->colorIndex = glGetAttribLocation(this->program, "aColor");
        checkForGlErrors("colorIndex");
    }
}

void SpriteShader::bindOrtho(GLfloat *matrix) {
    Shader::bind();
    glUniformMatrix4fv(this->matrixIndex, 1, GL_FALSE, matrix);
    checkForGlErrors("glUniformMatrix4fv");
    glEnableVertexAttribArray(this->positionIndex);
    checkForGlErrors("bind positionIndex");
    glEnableVertexAttribArray(this->colorIndex);
    checkForGlErrors("bind colorIndex");
    if (this->texCoordIndex > -1) {
        glEnableVertexAttribArray(this->texCoordIndex);
        checkForGlErrors("bind texCoordIndex");
    }
}

void SpriteShader::unbind() {
    glDisableVertexAttribArray(this->positionIndex);
    glDisableVertexAttribArray(this->colorIndex);
    if (this->texCoordIndex > -1) {
        glDisableVertexAttribArray(this->texCoordIndex);
    }
    Shader::unbind();
}

void SpriteShader::prepare(DrawCall *drawCall, RenderFloat *buffer) {
    int stride = this->bytesPerVertex();
    bool hasTexCoord = this->texCoordIndex > -1;
    int i = 0;
    RenderFloat *origin = nullptr;
    if (hasTexCoord) {
        for (int t = 0; t < drawCall->length(); ++t) {
            TriangleData &tri = drawCall->data[t];
            uint32_t c = colorToInt(tri.color);
            buffer[i++] = tri.t.p1.x;
            buffer[i++] = tri.t.p1.y;
            static_cast<uint32_t *>(static_cast<void *>(buffer))[i++] = c;
            buffer[i++] = tri.uv.p1.x;
            buffer[i++] = tri.uv.p1.y;
            buffer[i++] = tri.t.p2.x;
            buffer[i++] = tri.t.p2.y;
            static_cast<uint32_t *>(static_cast<void *>(buffer))[i++] = c;
            buffer[i++] = tri.uv.p2.x;
            buffer[i++] = tri.uv.p2.y;
            buffer[i++] = tri.t.p3.x;
            buffer[i++] = tri.t.p3.y;
            static_cast<uint32_t *>(static_cast<void *>(buffer))[i++] = c;
            buffer[i++] = tri.uv.p3.x;
            buffer[i++] = tri.uv.p3.y;
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, i * sizeof(RenderFloat), buffer);
        checkForGlErrors("bufferSubData");
        glVertexAttribPointer(this->positionIndex, 2, GL_FLOAT, GL_FALSE, stride, origin);
        glVertexAttribPointer(this->colorIndex, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, origin + 2);
        glVertexAttribPointer(this->texCoordIndex, 2, GL_FLOAT, GL_FALSE, stride, origin + 3);
        checkForGlErrors("attrib pointers");
    } else {
        for (int t = 0; t < drawCall->length(); ++t) {
            TriangleData &tri = drawCall->data[t];
            uint32_t c = colorToInt(tri.color);
            buffer[i++] = tri.t.p1.x;
            buffer[i++] = tri.t.p1.y;
            static_cast<uint32_t *>(static_cast<void *>(buffer))[i++] = c;
            buffer[i++] = tri.t.p2.x;
            buffer[i++] = tri.t.p2.y;
            static_cast<uint32_t *>(static_cast<void *>(buffer))[i++] = c;
            buffer[i++] = tri.t.p3.x;
            buffer[i++] = tri.t.p3.y;
            static_cast<uint32_t *>(static_cast<void *>(buffer))[i++] = c;
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, i * sizeof(RenderFloat), buffer);
        checkForGlErrors("bufferSubData");
        glVertexAttribPointer(this->positionIndex, 2, GL_FLOAT, GL_FALSE, stride, origin);
        glVertexAttribPointer(this->colorIndex, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, origin + 2);
        checkForGlErrors("attrib pointers");
    }
}

SpriteShader SpriteShader::defaultTextureSpriteShader = SpriteShader(R"(
// Krit texture vertex shader
attribute vec4 aPosition;
attribute vec2 aTexCoord;
attribute vec4 aColor;
varying vec2 vTexCoord;
varying vec4 vColor;
uniform mat4 uMatrix;

void main(void) {
    vColor = vec4(aColor.rgb * aColor.a, aColor.a);
    vTexCoord = aTexCoord;
    gl_Position = uMatrix * aPosition;
}
)",R"(// Krit texture fragment shader
varying vec4 vColor;
varying vec2 vTexCoord;
uniform sampler2D uImage;

void main(void) {
    vec4 color = texture2D(uImage, vTexCoord);
    if (color.a == 0.0) {
        gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);
    } else {
        gl_FragColor = color * vColor;
    }
}
)");

SpriteShader SpriteShader::defaultColorSpriteShader = SpriteShader(R"(// Krit color vertex shader
attribute vec4 aPosition;
attribute vec4 aColor;
varying vec4 vColor;
uniform mat4 uMatrix;

void main(void) {
    vColor = vec4(aColor.rgb * aColor.a, aColor.a);
    gl_Position = uMatrix * aPosition;
})",R"(
// Krit color fragment shader
varying vec4 vColor;

void main(void) {
    gl_FragColor = vColor;
}
)");

}
