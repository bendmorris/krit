R"(#version 300 es
// renderer.texture.frag
#ifdef GL_ES
precision highp float;
#endif
// Krit texture fragment shader
uniform sampler2D uImage;
// uniform float uDepth;
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
