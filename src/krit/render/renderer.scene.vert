R"(#version 300 es
// renderer.scene.vert
#ifdef GL_ES
precision highp float;
#endif
in vec4 aPosition;
out vec2 vTexCoord;

void main(void) {
    float ox = (aPosition.x + 1.0) / 2.0;
    float oy = (aPosition.y + 1.0) / 2.0;
    vTexCoord = vec2(ox, oy);
    gl_Position = aPosition;
}
)"
