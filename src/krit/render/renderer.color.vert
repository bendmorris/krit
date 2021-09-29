R"(
#version 330
#ifdef GLES
precision mediump float;
#endif
// Krit color vertex shader
uniform mat4 uMatrix;
in vec4 aPosition;
in vec4 aColor;
out vec4 vColor;

void main(void) {
    vColor = vec4(aColor.rgb * aColor.a, aColor.a);
    gl_Position = uMatrix * aPosition;
}
)"
