#version 330 core

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPos;

void main() {
    gl_FragDepth = gl_FragCoord.z;
    if (gl_FrontFacing) {
        vec3 normal = normalize(Normal);
        vec3 lightDir = normalize(lightPos - FragPos);
        float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
        gl_FragDepth += bias;
    }
}
