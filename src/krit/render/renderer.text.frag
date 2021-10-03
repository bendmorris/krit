R"(#version 330
#ifdef GLES
precision mediump float;
#endif
// Krit text fragment shader
uniform sampler2D uImage;
in vec4 vColor;
in vec2 vTexCoord;
out vec4 FragColor;

void main(void) {
    vec4 color = texture(uImage, vTexCoord);
    if (color.r == 0.0) {
        discard;
    } else {
        FragColor = vec4(color.r, color.r, color.r, color.r) * vColor;
    }
}
)"
