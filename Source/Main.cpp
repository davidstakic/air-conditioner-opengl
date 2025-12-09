#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Header/Util.h"
#include "../TextRenderer.h"
#include "../Rectangle.h"
#include "../TexturedQuad.h"
#include "../Circle.h"

#include <chrono>
#include <thread>
#include <iostream>
#include <string>

bool klima_on = false;
bool lavor_full = false;
float precaga_offset = 0.0f;
float precaga_speed = 0.0008f;
float zeljenaTemp = 24.0f;
float izmerenaTemp = 30.0f;
float waterHeight = 0.0f;

GLFWcursor* cursor;
GLFWcursor* cursorPressed;

unsigned snowflake;
unsigned fire;
unsigned check;
unsigned signature;

bool isMouseOverCircle(double mouse_x, double mouse_y, Circle& c, int width, int height) {
    double x = (mouse_x / width) * 2.0 - 1.0;
    double y = 1.0 - (mouse_y / height) * 2.0;
    double dx = x - c.cx;
    double dy = y - c.cy;
    return (dx * dx + dy * dy) <= (c.radius * c.radius);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        glfwSetCursor(window, cursorPressed);

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        Circle* lampica = (Circle*)glfwGetWindowUserPointer(window);
        if (!lampica) return;

        if (isMouseOverCircle(mouseX, mouseY, *lampica, width, height)) {
            if (!klima_on && !lavor_full) klima_on = true;
            else if (klima_on) klima_on = false;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        glfwSetCursor(window, cursor);
    }
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_UP)
            zeljenaTemp = std::min(40.0f, zeljenaTemp + 1.0f);
        if (key == GLFW_KEY_DOWN)
            zeljenaTemp = std::max(-10.0f, zeljenaTemp - 1.0f);
        if (key == GLFW_KEY_SPACE) {
            lavor_full = false;
            waterHeight = 0.0f;
        }
        if (key == GLFW_KEY_ESCAPE)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

void preprocessTexture(unsigned& texture, const char* filepath) {
    texture = loadImageToTexture(filepath);
    glBindTexture(GL_TEXTURE_2D, texture);

    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    if (!primary || !mode) return -1;

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Air Conditioner", primary, NULL);
    if (!window) return -1;
    glfwMakeContextCurrent(window);

    glfwSwapInterval(0);
    if (glewInit() != GLEW_OK) return -1;

    unsigned int rectShader = createShader("rect.vert", "rect.frag");
    unsigned int textShader = createShader("text.vert", "text.frag");
    unsigned int textureShader = createShader("texture.vert", "texture.frag");

    TextRenderer textRenderer;
    if (!textRenderer.init("Resources/DS-DIGI.TTF", textShader)) {
        std::cerr << "Failed to initialize TextRenderer\n";
        return -1;
    }

    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    float aspect = (float)fbw / (float)fbh;

    Rectangle klima(-0.378f, 0.6885f, 0.378f, 0.9415f);
    Rectangle precaga_light(-0.304f, 0.692f, 0.304f, 0.728f);
    Rectangle precaga_dark(-0.304f, 0.692f, 0.304f, 0.728f);
    Circle lampica(0.355f, 0.71f, 0.010f, aspect);

    float ekranGap = 0.015f;
    Rectangle ekran1(-0.20f - ekranGap, 0.8f, -0.12f - ekranGap, 0.88f);
    Rectangle ekran2(-0.04f, 0.8f, 0.04f, 0.88f);
    Rectangle ekran3(0.12f + ekranGap, 0.8f, 0.20f + ekranGap, 0.88f);
    Rectangle lavor(-0.22f, -0.805f, 0.22f, -0.545f);
    TexturedQuad iconQuad(0.14f + ekranGap, 0.81f, 0.18f + ekranGap, 0.87f);
    TexturedQuad signatureQuad(-0.99f, -0.99f, -0.65f, -0.87f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.8f, 0.6f, 1.0f);

    preprocessTexture(snowflake, "Resources/snowflake.png");
    preprocessTexture(fire, "Resources/fire.png");
    preprocessTexture(check, "Resources/check.png");
    preprocessTexture(signature, "Resources/signature.png");

    glfwSetWindowUserPointer(window, &lampica);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, keyboard_callback);

    cursor = loadImageToCursor("Resources/remote_control.png");
    cursorPressed = loadImageToCursor("Resources/remote_control_pressed.png");
    glfwSetCursor(window, cursor);

    const double targetFPS = 75.0;
    const double targetFrameTime = 1.0 / targetFPS;

    const float waterTopCoord = -0.56f;
    const float waterBottomCoord = -0.78f;
    const float waterHeightMax = waterTopCoord - waterBottomCoord;
    const float ABS_WATER_FILL_SPEED = 0.5f;

    while (!glfwWindowShouldClose(window)) {
        double frameStart = glfwGetTime();

        float max_offset = 0.036f;
        if (klima_on) {
            if (precaga_offset < max_offset) precaga_offset += precaga_speed;
            if (izmerenaTemp < zeljenaTemp) izmerenaTemp = std::min(zeljenaTemp, izmerenaTemp + 0.01f);
            else if (izmerenaTemp > zeljenaTemp) izmerenaTemp = std::max(zeljenaTemp, izmerenaTemp - 0.01f);
        }
        else {
            if (precaga_offset > 0.0f) precaga_offset -= precaga_speed;
        }

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(rectShader);

        int colorLoc = glGetUniformLocation(rectShader, "uColor");
        int uYOffsetLoc = glGetUniformLocation(rectShader, "uYOffset");
        int texLoc = glGetUniformLocation(textureShader, "uTexture");
        int opacityLoc = glGetUniformLocation(textureShader, "uOpacity");

        if (uYOffsetLoc != -1) glUniform1f(uYOffsetLoc, 0.0f);
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); klima.draw();
        glUniform4f(colorLoc, 0.2f, 0.2f, 0.2f, 1.0f); precaga_dark.draw();

        if (uYOffsetLoc != -1) glUniform1f(uYOffsetLoc, precaga_offset);
        glUniform4f(colorLoc, 0.5f, 0.5f, 0.5f, 1.0f); precaga_light.draw();

        if (uYOffsetLoc != -1) glUniform1f(uYOffsetLoc, 0.0f);
        if (klima_on) glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
        else glUniform4f(colorLoc, 0.2f, 0.2f, 0.2f, 1.0f);
        lampica.draw();

        glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 1.0f);
        ekran1.draw(); ekran2.draw(); ekran3.draw();

        glUniform4f(colorLoc, 0.8f, 0.8f, 0.8f, 0.3f);
        lavor.draw();

        glUniform4f(colorLoc, 0.1f, 0.7f, 0.8f, 1.0f);
        float currentWaterTop = waterBottomCoord + waterHeight;
        Rectangle dynamicWater(-0.205f, currentWaterTop, 0.205f, waterBottomCoord);
        dynamicWater.draw();

        if (klima_on) {
            std::string s1 = std::to_string((int)zeljenaTemp);
            std::string s2 = std::to_string((int)izmerenaTemp);

            textRenderer.render(s1, -0.203f, 0.825f, 0.0012f, Color(1, 1, 1));
            textRenderer.render(s2, -0.028f, 0.825f, 0.0012f, Color(1, 1, 1));

            if (izmerenaTemp < zeljenaTemp)
                glBindTexture(GL_TEXTURE_2D, fire);
            else if (izmerenaTemp > zeljenaTemp)
                glBindTexture(GL_TEXTURE_2D, snowflake);
            else
                glBindTexture(GL_TEXTURE_2D, check);

            glUseProgram(textureShader);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D,
                izmerenaTemp < zeljenaTemp ? fire :
                izmerenaTemp > zeljenaTemp ? snowflake : check);

            glUniform1i(texLoc, 0);
            glUniform1f(opacityLoc, 1.0f);
            iconQuad.draw();
        }

        glUseProgram(textureShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, signature);
        glUniform1f(opacityLoc, 0.5f);
        signatureQuad.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();

        double frameEnd = glfwGetTime();
        double frameTime = frameEnd - frameStart;

        if (klima_on && !lavor_full) {
            waterHeight += ABS_WATER_FILL_SPEED * frameTime;
            if (waterHeight >= waterHeightMax) {
                waterHeight = waterHeightMax;
                lavor_full = true;
                klima_on = false;
            }
        }

        double sleepTime = targetFrameTime - frameTime;
        if (sleepTime > 0.0) {
            auto sleepDur = std::chrono::duration<double>(sleepTime);
            std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(sleepDur));
            while (glfwGetTime() - frameStart < targetFrameTime) {}
        }
    }

    textRenderer.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
