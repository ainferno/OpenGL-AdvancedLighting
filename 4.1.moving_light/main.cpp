#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#undef STB_IMAGE_IMPLEMENTATION

#include "shaders.h"
#include "camera.h"

#include <iostream>
#include <vector>
#include <cmath>

GLFWwindow* init_gl_window(const std::string& name);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const std::string& path);

unsigned int SCR_WIDTH  = 1200;
unsigned int SCR_HEIGHT = 900;

Camera camera(glm::vec3(0.0f, 2.0f, 8.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool bloom = true;
bool bloomKeyPressed = false;
float exposure = 1.0f;

// --- Bloom FBO globals ---
unsigned int hdrFBO = 0;
unsigned int colorBuffers[2] = {0, 0};
unsigned int rboDepth = 0;
unsigned int pingpongFBO[2] = {0, 0};
unsigned int pingpongBuffers[2] = {0, 0};
unsigned int quadVAO = 0, quadVBO;

// --- Geometry globals ---
unsigned int planeVAO, planeVBO;
unsigned int cubeVAO, cubeVBO;
unsigned int sphereVAO, sphereVBO, sphereEBO;
unsigned int sphereIndexCount = 0;

void initPlane() {
    float planeVertices[] = {
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
        -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,

         25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f,
        -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
         25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f
    };
    glGenVertexArrays(1, &planeVAO);
    glGenBuffers(1, &planeVBO);
    glBindVertexArray(planeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
}

void initCube() {
    float vertices[] = {
        // back face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
        // front face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
        // left face
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        // right face
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        // bottom face
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
        // top face
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
    };
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
}

void initSphere() {
    const unsigned int X_SEGMENTS = 32;
    const unsigned int Y_SEGMENTS = 16;
    const float PI = 3.14159265359f;

    std::vector<float> data;
    std::vector<unsigned int> indices;

    for (unsigned int y = 0; y <= Y_SEGMENTS; ++y) {
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x) {
            float xSeg = (float)x / (float)X_SEGMENTS;
            float ySeg = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSeg * 2.0f * PI) * std::sin(ySeg * PI);
            float yPos = std::cos(ySeg * PI);
            float zPos = std::sin(xSeg * 2.0f * PI) * std::sin(ySeg * PI);

            data.push_back(xPos);
            data.push_back(yPos);
            data.push_back(zPos);
            data.push_back(xPos);
            data.push_back(yPos);
            data.push_back(zPos);
            data.push_back(xSeg);
            data.push_back(ySeg);
        }
    }

    for (unsigned int y = 0; y < Y_SEGMENTS; ++y) {
        for (unsigned int x = 0; x < X_SEGMENTS; ++x) {
            unsigned int i0 = y * (X_SEGMENTS + 1) + x;
            unsigned int i1 = i0 + X_SEGMENTS + 1;
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i0 + 1);
            indices.push_back(i1);
            indices.push_back(i1 + 1);
            indices.push_back(i0 + 1);
        }
    }
    sphereIndexCount = (unsigned int)indices.size();

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glBindVertexArray(0);
}

void renderScene(Shader& shader) {
    glm::mat4 model = glm::mat4(1.0f);

    // floor
    shader.setMat4("model", model);
    glBindVertexArray(planeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // cube 1
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0f));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // cube 2
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // cube 3
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0f));
    model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0f, 0.0f, 1.0f)));
    model = glm::scale(model, glm::vec3(0.25f));
    shader.setMat4("model", model);
    glDrawArrays(GL_TRIANGLES, 0, 36);

    // sphere
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-1.0f, 0.5f, -1.0f));
    model = glm::scale(model, glm::vec3(0.5f));
    shader.setMat4("model", model);
    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);
}

unsigned int createDepthMapFBO(unsigned int width, unsigned int height, unsigned int& depthMap) {
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return fbo;
}

void createBloomFramebuffers() {
    if (hdrFBO) {
        glDeleteFramebuffers(1, &hdrFBO);
        glDeleteTextures(2, colorBuffers);
        glDeleteRenderbuffers(1, &rboDepth);
        glDeleteFramebuffers(2, pingpongFBO);
        glDeleteTextures(2, pingpongBuffers);
    }

    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);

    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; ++i) {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }

    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    unsigned int attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "HDR framebuffer not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffers);
    for (unsigned int i = 0; i < 2; ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffers[i], 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Ping-pong framebuffer " << i << " not complete!" << std::endl;
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderQuad() {
    if (quadVAO == 0) {
        float quadVertices[] = {
            -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
             1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
             1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
        };
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glBindVertexArray(0);
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

int main() {
    GLFWwindow* window = init_gl_window("Moving light — Sun & Moon");
    if (!window) { return -1; }

    Shader shader("4.1.moving_light/1.advanced_lighting.vs", "4.1.moving_light/1.advanced_lighting.fs");
    Shader shaderShadow("4.1.moving_light/shadow.vs", "4.1.moving_light/shadow.fs");
    Shader shaderLightCube("4.1.moving_light/light_cube.vs", "4.1.moving_light/light_cube.fs");
    Shader shaderBlur("4.1.moving_light/blur.vs", "4.1.moving_light/blur.fs");
    Shader shaderFinal("4.1.moving_light/bloom_final.vs", "4.1.moving_light/bloom_final.fs");

    initPlane();
    initCube();
    initSphere();

    unsigned int floorTexture = loadTexture("../resources/textures/wood.png");

    // --- Two shadow map FBOs ---
    const unsigned int SHADOW_W = 2048, SHADOW_H = 2048;
    unsigned int depthMapSun, depthMapMoon;
    unsigned int fboSun  = createDepthMapFBO(SHADOW_W, SHADOW_H, depthMapSun);
    unsigned int fboMoon = createDepthMapFBO(SHADOW_W, SHADOW_H, depthMapMoon);

    // --- Shader texture units ---
    shader.use();
    shader.setInt("floorTexture", 0);
    shader.setInt("shadowMap1", 1);
    shader.setInt("shadowMap2", 2);

    shaderBlur.use();
    shaderBlur.setInt("image", 0);
    shaderFinal.use();
    shaderFinal.setInt("scene", 0);
    shaderFinal.setInt("bloomBlur", 1);

    createBloomFramebuffers();

    // --- Light colors ---
    glm::vec3 sunColor(1.0f, 0.9f, 0.7f);
    glm::vec3 moonColor(0.4f, 0.5f, 0.7f);
    glm::vec3 sunCubeColor(5.0f, 4.25f, 2.5f);
    glm::vec3 moonCubeColor(1.8f, 2.1f, 3.0f);

    // --- Orbit params ---
    float sunOrbitRadius  = 80.0f;
    float sunOrbitSpeed   = 0.1f;
    float moonOrbitRadius = 20.0f;
    float moonOrbitSpeed  = 0.03f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // --- Sun orbit ---
        float sunAngle = currentFrame * sunOrbitSpeed;
        float sunOrbits = sunAngle / (2.0f * glm::pi<float>());
        float sunTiltNorm = std::fmod(sunOrbits / 9.0f, 2.0f);
        float sunTiltDeg = (sunTiltNorm <= 1.0f ? sunTiltNorm : 2.0f - sunTiltNorm) * 45.0f;
        glm::mat4 sunTilt = glm::rotate(glm::mat4(1.0f), glm::radians(sunTiltDeg), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::vec3 baseSun(sunOrbitRadius * std::cos(sunAngle), sunOrbitRadius * std::sin(sunAngle), 0.0f);
        glm::vec3 sunPos = glm::vec3(sunTilt * glm::vec4(baseSun, 1.0f));

        // --- Moon orbit (independent) ---
        float moonAngle = currentFrame * moonOrbitSpeed;
        float moonOrbits = moonAngle / (2.0f * glm::pi<float>());
        float moonTiltNorm = std::fmod(moonOrbits / 7.0f, 2.0f);
        float moonTiltDeg = (moonTiltNorm <= 1.0f ? moonTiltNorm : 2.0f - moonTiltNorm) * 30.0f;
        glm::mat4 moonTilt = glm::rotate(glm::mat4(1.0f), glm::radians(moonTiltDeg), glm::vec3(0.0f, 0.0f, 1.0f));

        glm::vec3 baseMoon(moonOrbitRadius * std::cos(moonAngle), moonOrbitRadius * std::sin(moonAngle), 0.0f);
        glm::vec3 moonPos = glm::vec3(moonTilt * glm::vec4(baseMoon, 1.0f));

        // --- Horizon factor: fade out when light goes below ground ---
        float sunFactor  = glm::clamp(sunPos.y / 2.0f, 0.0f, 1.0f);
        float moonFactor = glm::clamp(moonPos.y / 2.0f, 0.0f, 1.0f);

        // --- Elevation: 0..1 ---
        float sunElevation  = glm::clamp(sunPos.y / sunOrbitRadius, 0.0f, 1.0f);
        float moonElevation = glm::clamp(moonPos.y / moonOrbitRadius, 0.0f, 1.0f);

        // --- Light space matrices ---
        float sunDist  = glm::length(sunPos);
        float moonDist = glm::length(moonPos);
        glm::mat4 sunProjection  = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, sunDist + 10.0f);
        glm::mat4 moonProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, moonDist + 10.0f);

        glm::mat4 lightViewSun  = glm::lookAt(sunPos,  glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightViewMoon = glm::lookAt(moonPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 lightSpaceMatrixSun  = sunProjection  * lightViewSun;
        glm::mat4 lightSpaceMatrixMoon = moonProjection * lightViewMoon;

        // --- 1. Shadow pass: Sun ---
        glDisable(GL_CULL_FACE);
        if (sunFactor > 0.0f) {
            shaderShadow.use();
            shaderShadow.setMat4("lightSpaceMatrix", lightSpaceMatrixSun);
            shaderShadow.setVec3("lightPos", sunPos);

            glViewport(0, 0, SHADOW_W, SHADOW_H);
            glBindFramebuffer(GL_FRAMEBUFFER, fboSun);
            glClear(GL_DEPTH_BUFFER_BIT);
            renderScene(shaderShadow);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }

        // --- 2. Shadow pass: Moon ---
        if (moonFactor > 0.0f) {
            shaderShadow.use();
            shaderShadow.setMat4("lightSpaceMatrix", lightSpaceMatrixMoon);
            shaderShadow.setVec3("lightPos", moonPos);

            glViewport(0, 0, SHADOW_W, SHADOW_H);
            glBindFramebuffer(GL_FRAMEBUFFER, fboMoon);
            glClear(GL_DEPTH_BUFFER_BIT);
            renderScene(shaderShadow);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        glEnable(GL_CULL_FACE);

        // --- 3. Scene pass → HDR FBO ---
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

        // Day/night sky color
        float sunH = glm::clamp(sunPos.y / sunOrbitRadius, -1.0f, 1.0f);
        glm::vec3 nightSky(0.02f, 0.02f, 0.05f);
        glm::vec3 daySky(0.4f, 0.55f, 0.8f);
        glm::vec3 sunsetSky(0.6f, 0.25f, 0.1f);
        float dayBlend = glm::clamp(sunH * 3.0f, 0.0f, 1.0f);
        float sunsetBlend = glm::clamp(1.0f - std::abs(sunH) * 5.0f, 0.0f, 1.0f);
        glm::vec3 skyColor = glm::mix(nightSky, daySky, dayBlend) + sunsetSky * sunsetBlend;
        glm::vec3 skyAmbient = skyColor * 0.15f;

        glClear(GL_DEPTH_BUFFER_BIT);
        float skyClear[] = {skyColor.r, skyColor.g, skyColor.b, 1.0f};
        float black[] = {0.0f, 0.0f, 0.0f, 1.0f};
        glClearBufferfv(GL_COLOR, 0, skyClear);
        glClearBufferfv(GL_COLOR, 1, black);
        glCullFace(GL_BACK);

        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom),
                                                 (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                 0.1f, 100.0f);
        glm::mat4 view = camera.getViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setMat4("lightSpaceMatrix1", lightSpaceMatrixSun);
        shader.setMat4("lightSpaceMatrix2", lightSpaceMatrixMoon);
        shader.setVec3("viewPos", camera.position);

        shader.setVec3("lightPos1", sunPos);
        shader.setVec3("lightPos2", moonPos);
        shader.setVec3("lightColor1", sunColor);
        shader.setVec3("lightColor2", moonColor);
        shader.setFloat("ambientStrength1", 0.1f);
        shader.setFloat("ambientStrength2", 0.05f);
        shader.setFloat("lightIntensity1", sunFactor);
        shader.setFloat("lightIntensity2", moonFactor);
        shader.setFloat("lightElevation1", sunElevation);
        shader.setFloat("lightElevation2", moonElevation);
        shader.setVec3("skyAmbient", skyAmbient);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMapSun);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMapMoon);

        renderScene(shader);

        // --- 4. Light spheres ---
        shaderLightCube.use();
        shaderLightCube.setMat4("projection", projection);
        shaderLightCube.setMat4("view", view);

        // Sun sphere
        float sunCubeScale = sunOrbitRadius * 0.05f;
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sunPos);
        model = glm::scale(model, glm::vec3(sunCubeScale));
        shaderLightCube.setMat4("model", model);
        shaderLightCube.setVec3("lightColor", sunCubeColor);
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);

        // Moon sphere
        float moonCubeScale = moonOrbitRadius * 0.05f;
        model = glm::mat4(1.0f);
        model = glm::translate(model, moonPos);
        model = glm::scale(model, glm::vec3(moonCubeScale));
        shaderLightCube.setMat4("model", model);
        shaderLightCube.setVec3("lightColor", moonCubeColor);
        glDrawElements(GL_TRIANGLES, sphereIndexCount, GL_UNSIGNED_INT, 0);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // --- 4. Blur pass: ping-pong Gaussian blur on bright pixels ---
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        shaderBlur.use();
        glActiveTexture(GL_TEXTURE0);
        for (unsigned int i = 0; i < amount; ++i) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            shaderBlur.setBool("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongBuffers[!horizontal]);
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration) {
                first_iteration = false;
            }
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // --- 5. Final composition: ACES Filmic + bloom ---
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderFinal.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffers[!horizontal]);
        shaderFinal.setBool("bloom", bloom);
        shaderFinal.setFloat("exposure", exposure);
        renderQuad();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);
    glDeleteFramebuffers(1, &fboSun);
    glDeleteFramebuffers(1, &fboMoon);
    glDeleteTextures(1, &depthMapSun);
    glDeleteTextures(1, &depthMapMoon);
    glDeleteFramebuffers(1, &hdrFBO);
    glDeleteTextures(2, colorBuffers);
    glDeleteRenderbuffers(1, &rboDepth);
    glDeleteFramebuffers(2, pingpongFBO);
    glDeleteTextures(2, pingpongBuffers);
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

GLFWwindow* init_gl_window(const std::string& name) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);

    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    SCR_WIDTH  = mode->width;
    SCR_HEIGHT = mode->height;

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, name.c_str(), NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwSetWindowPos(window, 0, 0);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    stbi_set_flip_vertically_on_load(true);
    return window;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { camera.processKeyboard(FORWARD,  deltaTime); }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { camera.processKeyboard(BACKWARD, deltaTime); }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { camera.processKeyboard(LEFT,     deltaTime); }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { camera.processKeyboard(RIGHT,    deltaTime); }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bloomKeyPressed) {
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE) {
        bloomKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        exposure -= 0.5f * deltaTime;
        if (exposure < 0.0f) { exposure = 0.0f; }
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        exposure += 0.5f * deltaTime;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
    if (width > 0 && height > 0) {
        createBloomFramebuffers();
    }
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    camera.processMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.processScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(const std::string& path) {
    int width, height, nrChannels;
    unsigned char* data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
    if (!data) {
        std::cerr << "Failed to load texture: " << path << std::endl;
        return 0;
    }

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return texture;
}
