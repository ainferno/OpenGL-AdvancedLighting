#version 330 core
in vec2 TexCoords;

uniform sampler2D fontAtlas;
uniform vec3 textColor;

out vec4 FragColor;

void main() {
    float a = texture(fontAtlas, TexCoords).r;
    if (a < 0.5) {
        discard;
    }
    FragColor = vec4(textColor, 1.0);
}
