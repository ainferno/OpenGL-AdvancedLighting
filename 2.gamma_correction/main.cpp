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
#include "text_renderer.h"

#include <iostream>
#include <cstdio>

GLFWwindow* init_gl_window(const std::string& name);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const std::string& path, bool corrected);

unsigned int SCR_WIDTH  = 1200;
unsigned int SCR_HEIGHT = 900;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

bool blinn = false;
bool blinnKeyPressed = false;

bool showGammaUI = false;
bool gammaKeyPressed = false;
float gammaValue = 2.2f;
bool draggingSlider = false;
double mouseX = 0.0, mouseY = 0.0;

// slider layout (recalculated on resize)
float sliderX, sliderY, sliderW, sliderH;
float handleW;
float buttonX, buttonY, buttonW, buttonH;
float panelX, panelY, panelW, panelH;
const float GAMMA_MIN = 0.5f;
const float GAMMA_MAX = 5.0f;

unsigned int uiVAO = 0, uiVBO = 0;

void initUI() {
    glGenVertexArrays(1, &uiVAO);
    glGenBuffers(1, &uiVBO);
    glBindVertexArray(uiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, uiVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * 6, nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void renderQuad(Shader& uiShader, float x, float y, float w, float h, glm::vec4 color) {
    glm::mat4 proj = glm::ortho(0.0f, (float)SCR_WIDTH, 0.0f, (float)SCR_HEIGHT);
    float by = (float)SCR_HEIGHT - y - h;

    float verts[] = {
        x,     by + h,
        x,     by,
        x + w, by,
        x,     by + h,
        x + w, by,
        x + w, by + h
    };

    uiShader.use();
    uiShader.setMat4("projection", proj);
    uiShader.setVec4("uColor", color.r, color.g, color.b, color.a);

    glBindVertexArray(uiVAO);
    glBindBuffer(GL_ARRAY_BUFFER, uiVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void updateSliderLayout() {
    panelW = 420.0f;
    panelH = 160.0f;
    panelX = ((float)SCR_WIDTH - panelW) / 2.0f;
    panelY = ((float)SCR_HEIGHT - panelH) / 2.0f;

    sliderX = panelX + 30.0f;
    sliderY = panelY + 65.0f;
    sliderW = panelW - 60.0f;
    sliderH = 20.0f;
    handleW = 12.0f;

    buttonW = 120.0f;
    buttonH = 32.0f;
    buttonX = panelX + (panelW - buttonW) / 2.0f;
    buttonY = panelY + panelH - buttonH - 15.0f;
}

float gammaToSliderX(float g) {
    float t = (g - GAMMA_MIN) / (GAMMA_MAX - GAMMA_MIN);
    return sliderX + t * (sliderW - handleW);
}

float sliderXToGamma(float sx) {
    float t = (sx - sliderX) / (sliderW - handleW);
    if (t < 0.0f) { t = 0.0f; }
    if (t > 1.0f) { t = 1.0f; }
    return GAMMA_MIN + t * (GAMMA_MAX - GAMMA_MIN);
}

bool pointInRect(double px, double py, float rx, float ry, float rw, float rh) {
    return px >= rx && px <= rx + rw && py >= ry && py <= ry + rh;
}

int main() {
    GLFWwindow* window = init_gl_window("Gamma Correction");
    if (!window) { return -1; }

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader shader("2.gamma_correction/1.advanced_lighting.vs", "2.gamma_correction/1.advanced_lighting.fs");
    Shader uiShader("shaders/ui.vs", "shaders/ui.fs");
    Shader textShader("shaders/text.vs", "shaders/text.fs");

    TextRenderer textRenderer;
    textRenderer.init();
    initUI();
    updateSliderLayout();

    float planeVertices[] = {
         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,

         10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,  10.0f,  0.0f,
        -10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,   0.0f, 10.0f,
         10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,  10.0f, 10.0f
    };

    unsigned int planeVAO, planeVBO;
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

    unsigned int floorTexture = loadTexture("../resources/textures/wood.png", true);

    shader.use();
    shader.setInt("texture1", 0);

    glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.zoom),
                                                 (float)SCR_WIDTH / (float)SCR_HEIGHT,
                                                 0.1f, 100.0f);
        glm::mat4 view = camera.getViewMatrix();
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);
        shader.setVec3("viewPos", camera.position);
        shader.setVec3("lightPos", lightPos);
        shader.setInt("blinn", blinn);
        shader.setFloat("gamma", gammaValue);

        glBindVertexArray(planeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        if (showGammaUI) {
            glDisable(GL_DEPTH_TEST);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            renderQuad(uiShader, 0, 0, (float)SCR_WIDTH, (float)SCR_HEIGHT, glm::vec4(0.0f, 0.0f, 0.0f, 0.5f));
            renderQuad(uiShader, panelX, panelY, panelW, panelH, glm::vec4(0.15f, 0.15f, 0.15f, 0.95f));
            renderQuad(uiShader, sliderX, sliderY, sliderW, sliderH, glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));

            float handleX = gammaToSliderX(gammaValue);
            float fillW = handleX - sliderX + handleW / 2.0f;
            renderQuad(uiShader, sliderX, sliderY, fillW, sliderH, glm::vec4(0.2f, 0.7f, 0.3f, 1.0f));
            renderQuad(uiShader, handleX, sliderY - 3.0f, handleW, sliderH + 6.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

            bool hover = pointInRect(mouseX, mouseY, buttonX, buttonY, buttonW, buttonH);
            glm::vec4 btnColor = hover ? glm::vec4(0.3f, 0.6f, 0.9f, 1.0f) : glm::vec4(0.2f, 0.5f, 0.8f, 1.0f);
            renderQuad(uiShader, buttonX, buttonY, buttonW, buttonH, btnColor);

            char buf[32];
            std::snprintf(buf, sizeof(buf), "Gamma: %.2f", gammaValue);
            textRenderer.renderText(textShader, buf, panelX + 30.0f, panelY + 18.0f, 2.5f, SCR_WIDTH, SCR_HEIGHT);
            textRenderer.renderText(textShader, "Apply", buttonX + 10.0f, buttonY + 6.0f, 2.5f, SCR_WIDTH, SCR_HEIGHT);

            glEnable(GL_DEPTH_TEST);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &planeVAO);
    glDeleteBuffers(1, &planeVBO);
    glDeleteVertexArrays(1, &uiVAO);
    glDeleteBuffers(1, &uiVBO);
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
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return nullptr;
    }

    glEnable(GL_DEPTH_TEST);
    stbi_set_flip_vertically_on_load(true);
    return window;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        if (showGammaUI) {
            showGammaUI = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        } else {
            glfwSetWindowShouldClose(window, true);
        }
    }

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gammaKeyPressed) {
        showGammaUI = !showGammaUI;
        if (showGammaUI) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
        gammaKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) {
        gammaKeyPressed = false;
    }

    if (showGammaUI) { return; }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) { camera.processKeyboard(FORWARD,  deltaTime); }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) { camera.processKeyboard(BACKWARD, deltaTime); }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) { camera.processKeyboard(LEFT,     deltaTime); }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) { camera.processKeyboard(RIGHT,    deltaTime); }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !blinnKeyPressed) {
        blinn = !blinn;
        blinnKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
        blinnKeyPressed = false;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
    updateSliderLayout();
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    mouseX = xposIn;
    mouseY = yposIn;

    if (showGammaUI) {
        if (draggingSlider) {
            gammaValue = sliderXToGamma((float)mouseX);
        }
        return;
    }

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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (!showGammaUI || button != GLFW_MOUSE_BUTTON_LEFT) { return; }

    if (action == GLFW_PRESS) {
        if (pointInRect(mouseX, mouseY, sliderX, sliderY - 5.0f, sliderW, sliderH + 10.0f)) {
            draggingSlider = true;
            gammaValue = sliderXToGamma((float)mouseX);
        }

        if (pointInRect(mouseX, mouseY, buttonX, buttonY, buttonW, buttonH)) {
            showGammaUI = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
    }

    if (action == GLFW_RELEASE) {
        draggingSlider = false;
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (showGammaUI) { return; }
    camera.processScroll(static_cast<float>(yoffset));
}

unsigned int loadTexture(const std::string& path, bool corrected) {
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
    GLenum tFormat;
    if (corrected) {
        tFormat = (nrChannels == 4) ? GL_SRGB_ALPHA : GL_SRGB;
    } else {
        tFormat = format;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, tFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    return texture;
}
