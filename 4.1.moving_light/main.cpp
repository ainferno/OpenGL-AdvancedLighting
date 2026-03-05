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

int main() {
    GLFWwindow* window = init_gl_window("Moving light — Sun & Moon");
    if (!window) { return -1; }

    Shader shader("4.1.moving_light/1.advanced_lighting.vs", "4.1.moving_light/1.advanced_lighting.fs");
    Shader shaderShadow("4.1.moving_light/shadow.vs", "4.1.moving_light/shadow.fs");
    Shader shaderLightCube("4.1.moving_light/light_cube.vs", "4.1.moving_light/light_cube.fs");

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

    // --- Light colors ---
    glm::vec3 sunColor(1.0f, 0.9f, 0.7f);
    glm::vec3 moonColor(0.4f, 0.5f, 0.7f);
    glm::vec3 sunCubeColor(1.0f, 0.85f, 0.5f);
    glm::vec3 moonCubeColor(0.6f, 0.7f, 1.0f);

    // --- Orbit params ---
    float orbitRadius = 8.0f;
    float orbitSpeed  = 5.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // --- Compute light positions ---
        float angle = currentFrame * orbitSpeed;
        float orbitsCompleted = angle / (2.0f * glm::pi<float>());
        // Tilt oscillates: +5 deg per orbit, ping-pong between 0 and 45
        float tiltNorm = std::fmod(orbitsCompleted / 9.0f, 2.0f);
        float tiltDeg = (tiltNorm <= 1.0f ? tiltNorm : 2.0f - tiltNorm) * 45.0f;
        glm::mat4 tiltMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(tiltDeg), glm::vec3(1.0f, 0.0f, 0.0f));

        glm::vec3 baseSun(orbitRadius * std::cos(angle), orbitRadius * std::sin(angle), 0.0f);
        glm::vec3 sunPos  = glm::vec3(tiltMatrix * glm::vec4(baseSun, 1.0f));
        glm::vec3 moonPos = -sunPos;

        // --- Horizon factor: fade out when light goes below ground ---
        float sunFactor  = glm::clamp(sunPos.y / 2.0f, 0.0f, 1.0f);
        float moonFactor = glm::clamp(moonPos.y / 2.0f, 0.0f, 1.0f);

        // --- Elevation: sin(angle above horizon), 0..1 ---
        float sunElevation  = glm::clamp(sunPos.y / orbitRadius, 0.0f, 1.0f);
        float moonElevation = glm::clamp(moonPos.y / orbitRadius, 0.0f, 1.0f);

        // --- Light space matrices ---
        glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 1.0f, 20.0f);

        glm::mat4 lightViewSun  = glm::lookAt(sunPos,  glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 lightViewMoon = glm::lookAt(moonPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

        glm::mat4 lightSpaceMatrixSun  = lightProjection * lightViewSun;
        glm::mat4 lightSpaceMatrixMoon = lightProjection * lightViewMoon;

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

        // --- 3. Scene pass ---
        glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
        glClearColor(0.05f, 0.05f, 0.08f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, depthMapSun);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMapMoon);

        renderScene(shader);

        // --- 4. Light cubes ---
        shaderLightCube.use();
        shaderLightCube.setMat4("projection", projection);
        shaderLightCube.setMat4("view", view);

        glBindVertexArray(cubeVAO);

        // Sun cube
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sunPos);
        model = glm::scale(model, glm::vec3(0.4f));
        shaderLightCube.setMat4("model", model);
        shaderLightCube.setVec3("lightColor", sunCubeColor);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // Moon cube
        model = glm::mat4(1.0f);
        model = glm::translate(model, moonPos);
        model = glm::scale(model, glm::vec3(0.2f));
        shaderLightCube.setMat4("model", model);
        shaderLightCube.setVec3("lightColor", moonCubeColor);
        glDrawArrays(GL_TRIANGLES, 0, 36);

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
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
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
