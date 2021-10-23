#include "krit/render/SpriteShader.h"
#include "krit/Engine.h"
#include "krit/TaskManager.h"
#include "krit/math/Point.h"
#include "krit/math/Triangle.h"
#include "krit/render/DrawCall.h"
#include "krit/render/DrawCommand.h"
#include "krit/render/Shader.h"
#include "krit/utils/Color.h"
#include "krit/utils/Panic.h"

namespace krit {

SpriteShader::SpriteShader(Shader &shader) : ShaderInstance(shader) {
    matrixIndex = shader.getUniformLocation("uMatrix");
}

void SpriteShader::bind(RenderContext &ctx) { ShaderInstance::bind(ctx); }

void SpriteShader::unbind() { ShaderInstance::unbind(); }

static void *offset(int n) {
    return (void*)(sizeof(RenderFloat)*n);
}

void SpriteShader::prepare(RenderContext &ctx, DrawCall *drawCall,
                           RenderFloat *buffer) {
    DrawCommandBuffer *buf = ctx.drawCommandBuffer;
    int stride = shader.bytesPerVertex;
    bool hasTexCoord = shader.texCoordIndex > -1;
    bool hasColor = shader.colorIndex > -1;
    int i = 0;
    if (hasTexCoord && hasColor) {
        for (size_t t = 0; t < drawCall->length(); ++t) {
            TriangleData &tri = buf->triangles[drawCall->indices[t]];
            uint32_t c = tri.color.abgr();
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
        glVertexAttribPointer(shader.positionIndex, 2, GL_FLOAT, GL_FALSE,
                              stride, offset(0));
        glVertexAttribPointer(shader.colorIndex, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                              stride, offset(2));
        glVertexAttribPointer(shader.texCoordIndex, 2, GL_FLOAT, GL_FALSE,
                              stride, offset(3));
        checkForGlErrors("attrib pointers");
    } else if (hasColor) {
        for (size_t t = 0; t < drawCall->length(); ++t) {
            TriangleData &tri = buf->triangles[drawCall->indices[t]];
            uint32_t c = tri.color.abgr();
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
        glVertexAttribPointer(shader.positionIndex, 2, GL_FLOAT, GL_FALSE,
                              stride, offset(0));
        glVertexAttribPointer(shader.colorIndex, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                              stride, offset(2));
        checkForGlErrors("attrib pointers");
    } else if (hasTexCoord) {
        for (size_t t = 0; t < drawCall->length(); ++t) {
            TriangleData &tri = buf->triangles[drawCall->indices[t]];
            buffer[i++] = tri.t.p1.x;
            buffer[i++] = tri.t.p1.y;
            buffer[i++] = tri.uv.p1.x;
            buffer[i++] = tri.uv.p1.y;
            buffer[i++] = tri.t.p2.x;
            buffer[i++] = tri.t.p2.y;
            buffer[i++] = tri.uv.p2.x;
            buffer[i++] = tri.uv.p2.y;
            buffer[i++] = tri.t.p3.x;
            buffer[i++] = tri.t.p3.y;
            buffer[i++] = tri.uv.p3.x;
            buffer[i++] = tri.uv.p3.y;
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, i * sizeof(RenderFloat), buffer);
        checkForGlErrors("bufferSubData");
        glVertexAttribPointer(shader.positionIndex, 2, GL_FLOAT, GL_FALSE,
                              stride, offset(0));
        glVertexAttribPointer(shader.texCoordIndex, 2, GL_FLOAT, GL_FALSE,
                              stride, offset(2));
        checkForGlErrors("attrib pointers");
    } else {
        for (size_t t = 0; t < drawCall->length(); ++t) {
            TriangleData &tri = buf->triangles[drawCall->indices[t]];
            buffer[i++] = tri.t.p1.x;
            buffer[i++] = tri.t.p1.y;
            buffer[i++] = tri.t.p2.x;
            buffer[i++] = tri.t.p2.y;
            buffer[i++] = tri.t.p3.x;
            buffer[i++] = tri.t.p3.y;
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, i * sizeof(RenderFloat), buffer);
        checkForGlErrors("bufferSubData");
        glVertexAttribPointer(shader.positionIndex, 2, GL_FLOAT, GL_FALSE,
                              stride, offset(0));
        checkForGlErrors("attrib pointers");
    }
}

}
