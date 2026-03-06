#version 330 core
out vec4 FragColor;

in VS_OUT {
    vec3 FragPos;
    vec3 Normal;
    vec2 TexCoords;
    vec4 FragPosLightSpace1;
    vec4 FragPosLightSpace2;
} fs_in;

uniform sampler2D floorTexture;
uniform sampler2D shadowMap1;
uniform sampler2D shadowMap2;

uniform vec3 lightPos1;
uniform vec3 lightPos2;
uniform vec3 lightColor1;
uniform vec3 lightColor2;
uniform float ambientStrength1;
uniform float ambientStrength2;
uniform float lightIntensity1;
uniform float lightIntensity2;
uniform vec3 viewPos;
uniform vec3 skyAmbient;

float ShadowCalculation(vec4 fragPosLightSpace, sampler2D shadowMap) {
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w * 0.5 + 0.5;

    if (projCoords.z > 1.0) {
        return 0.0;
    }

    float currentDepth = projCoords.z;
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
            shadow += currentDepth > pcfDepth ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;

    return shadow;
}

vec3 calcLight(vec3 lightPos, vec3 lightColor, float ambientStr, float intensity,
               vec4 fragPosLightSpace, sampler2D shadowMap,
               vec3 normal, vec3 viewDir, vec3 color) {
    if (intensity <= 0.0) {
        return vec3(0.0);
    }

    vec3 ambient = ambientStr * lightColor;

    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;

    float shadow = ShadowCalculation(fragPosLightSpace, shadowMap);

    return intensity * (ambient + (1.0 - shadow) * (diffuse + specular)) * color;
}

void main() {
    vec3 color = texture(floorTexture, fs_in.TexCoords).rgb;
    vec3 normal = normalize(fs_in.Normal);
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);

    vec3 result = skyAmbient * color;
    result += calcLight(lightPos1, lightColor1, ambientStrength1, lightIntensity1,
                        fs_in.FragPosLightSpace1, shadowMap1, normal, viewDir, color);
    result += calcLight(lightPos2, lightColor2, ambientStrength2, lightIntensity2,
                        fs_in.FragPosLightSpace2, shadowMap2, normal, viewDir, color);

    FragColor = vec4(result, 1.0);
}
