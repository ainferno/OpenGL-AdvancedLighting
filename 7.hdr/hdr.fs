#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D hdrBuffer;
uniform bool hdr;
uniform float exposure;

vec3 ACESFilmic(vec3 x) {
    float a = 2.51;
    float b = 0.03;
    float c = 2.43;
    float d = 0.59;
    float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

void main() {
    const float gamma = 2.2;
    vec3 hdrColor = texture(hdrBuffer, TexCoords).rgb;

    vec3 result;
    if (hdr) {
        result = ACESFilmic(hdrColor * exposure);
        result = pow(result, vec3(1.0 / gamma));
    } else {
        result = pow(hdrColor, vec3(1.0 / gamma));
    }

    FragColor = vec4(result, 1.0);
}
