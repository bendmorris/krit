R"(#version 330
#ifdef GLES
precision mediump float;
#endif
// Krit color fragment shader
in vec4 vColor;
out vec4 FragColor;

void main(void) {
    FragColor = vColor;
}
)"
