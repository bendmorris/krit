R"(
#version 330
#ifdef GLES
precision mediump float;
#endif
// Krit texture vertex shader
uniform mat4 uMatrix;
in vec4 aPosition;
in vec2 aTexCoord;
in vec4 aColor;
out vec2 vTexCoord;
out vec4 vColor;

void main(void) {
    vColor = vec4(aColor.rgb * aColor.a, aColor.a);
    vTexCoord = aTexCoord;
    gl_Position = uMatrix * aPosition;
}
)"
