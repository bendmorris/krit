R"(#version 330
#ifdef GL_ES
precision mediump float;
#endif
// Krit color fragment shader
in vec4 vColor;
out vec4 FragColor;

void main(void) {
    FragColor = vColor;
}
)"
