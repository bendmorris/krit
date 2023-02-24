R"(#version 300 es
#ifdef GL_ES
precision mediump float;
#endif

uniform sampler2D uImage;
in vec2 vTexCoord;
out vec4 FragColor;

void main(void) {
    vec4 color = texture(uImage, vTexCoord);
    if (color.a == 0.0) {
        discard;
    } else {
        FragColor = color;
    }
}
)"
