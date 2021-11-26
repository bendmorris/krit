R"(#version 300 es
#ifdef GL_ES
precision mediump float;
#endif
// Krit texture fragment shader
uniform sampler2D uImage;
in vec4 vColor;
in vec2 vTexCoord;
out vec4 FragColor;

void main(void) {
    vec4 color = texture(uImage, vTexCoord) * vColor;
    if (color.a == 0.0) {
        discard;
    } else {
        FragColor = color;
    }
}
)"
