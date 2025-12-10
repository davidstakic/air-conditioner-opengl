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

bool is_turned_on = false;
bool is_full = false;
float flap_offset = 0.0f;
float flap_speed = 0.0008f;
float desired_temperature = 24.0f;
float measured_temperature = 31.0f;
float water_height = 0.0f;

GLFWcursor* cursor;
GLFWcursor* cursor_pressed;

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
        glfwSetCursor(window, cursor_pressed);

        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);

        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        Circle* light = (Circle*)glfwGetWindowUserPointer(window);
        if (!light) return;

        if (isMouseOverCircle(mouseX, mouseY, *light, width, height)) {
            if (!is_turned_on && !is_full) is_turned_on = true;
            else if (is_turned_on) is_turned_on = false;
        }
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        glfwSetCursor(window, cursor);
    }
}

void keyboard_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (action == GLFW_PRESS) {
        if (key == GLFW_KEY_UP)
            desired_temperature = std::min(40.0f, desired_temperature + 1.0f);
        if (key == GLFW_KEY_DOWN)
            desired_temperature = std::max(-10.0f, desired_temperature - 1.0f);
        if (key == GLFW_KEY_SPACE) {
            is_full = false;
            water_height = 0.0f;
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

    TextRenderer text_renderer;
    if (!text_renderer.init("Resources/DS-DIGI.TTF", textShader)) {
        std::cerr << "Failed to initialize TextRenderer\n";
        return -1;
    }

    int fbw, fbh;
    glfwGetFramebufferSize(window, &fbw, &fbh);
    float aspect = (float)fbw / (float)fbh;

    Rectangle air_conditioner(-0.378f, 0.6885f, 0.378f, 0.9415f);
    Rectangle flap_light(-0.304f, 0.692f, 0.304f, 0.728f);
    Rectangle flap_dark(-0.304f, 0.692f, 0.304f, 0.728f);
    Circle light(0.355f, 0.71f, 0.010f, aspect);

    float screen_gap = 0.015f;
    Rectangle screen_1(-0.20f - screen_gap, 0.8f, -0.12f - screen_gap, 0.88f);
    Rectangle screen_2(-0.04f, 0.8f, 0.04f, 0.88f);
    Rectangle screen_3(0.12f + screen_gap, 0.8f, 0.20f + screen_gap, 0.88f);
    Rectangle basin(-0.22f, -0.805f, 0.22f, -0.545f);
    TexturedQuad icon(0.14f + screen_gap, 0.81f, 0.18f + screen_gap, 0.87f);
    TexturedQuad signature_texture(-0.99f, -0.99f, -0.65f, -0.87f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.8f, 0.6f, 1.0f);

    preprocessTexture(snowflake, "Resources/snowflake.png");
    preprocessTexture(fire, "Resources/fire.png");
    preprocessTexture(check, "Resources/check.png");
    preprocessTexture(signature, "Resources/signature.png");

    glfwSetWindowUserPointer(window, &light);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetKeyCallback(window, keyboard_callback);

    cursor = loadImageToCursor("Resources/remote_control.png");
    cursor_pressed = loadImageToCursor("Resources/remote_control_pressed.png");
    glfwSetCursor(window, cursor);

    const double target_fps = 75.0;
    const double target_frame_time = 1.0 / target_fps;

    const float water_top_coord = -0.56f;
    const float water_bottom_coord = -0.78f;
    const float water_height_max = water_top_coord - water_bottom_coord;
    const float ABS_WATER_FILL_SPEED = 0.5f;

    while (!glfwWindowShouldClose(window)) {
        double frame_start = glfwGetTime();

        float max_offset = 0.036f;
        if (is_turned_on) {
            if (flap_offset < max_offset) flap_offset += flap_speed;
            if (measured_temperature < desired_temperature) measured_temperature = std::min(desired_temperature, measured_temperature + 0.01f);
            else if (measured_temperature > desired_temperature) measured_temperature = std::max(desired_temperature, measured_temperature - 0.01f);
        }
        else {
            if (flap_offset > 0.0f) flap_offset -= flap_speed;
        }

        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(rectShader);

        int colorLoc = glGetUniformLocation(rectShader, "uColor");
        int uYOffsetLoc = glGetUniformLocation(rectShader, "uYOffset");
        int texLoc = glGetUniformLocation(textureShader, "uTexture");
        int opacityLoc = glGetUniformLocation(textureShader, "uOpacity");

        if (uYOffsetLoc != -1) glUniform1f(uYOffsetLoc, 0.0f);
        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f); air_conditioner.draw();
        glUniform4f(colorLoc, 0.2f, 0.2f, 0.2f, 1.0f); flap_dark.draw();

        if (uYOffsetLoc != -1) glUniform1f(uYOffsetLoc, flap_offset);
        glUniform4f(colorLoc, 0.5f, 0.5f, 0.5f, 1.0f); flap_light.draw();

        if (uYOffsetLoc != -1) glUniform1f(uYOffsetLoc, 0.0f);
        if (is_turned_on) glUniform4f(colorLoc, 1.0f, 0.0f, 0.0f, 1.0f);
        else glUniform4f(colorLoc, 0.2f, 0.2f, 0.2f, 1.0f);
        light.draw();

        glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 1.0f);
        screen_1.draw(); screen_2.draw(); screen_3.draw();

        glUniform4f(colorLoc, 0.8f, 0.8f, 0.8f, 0.3f);
        basin.draw();

        glUniform4f(colorLoc, 0.1f, 0.7f, 0.8f, 1.0f);
        float currentWaterTop = water_bottom_coord + water_height;
        Rectangle dynamicWater(-0.205f, currentWaterTop, 0.205f, water_bottom_coord);
        dynamicWater.draw();

        if (is_turned_on) {
            std::string s1 = std::to_string((int)desired_temperature);
            std::string s2 = std::to_string((int)measured_temperature);

            text_renderer.render(s1, -0.203f, 0.825f, 0.0012f, Color(1, 1, 1));
            text_renderer.render(s2, -0.028f, 0.825f, 0.0012f, Color(1, 1, 1));

            if (measured_temperature < desired_temperature)
                glBindTexture(GL_TEXTURE_2D, fire);
            else if (measured_temperature > desired_temperature)
                glBindTexture(GL_TEXTURE_2D, snowflake);
            else
                glBindTexture(GL_TEXTURE_2D, check);

            glUseProgram(textureShader);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D,
                measured_temperature < desired_temperature ? fire :
                measured_temperature > desired_temperature ? snowflake : check);

            glUniform1i(texLoc, 0);
            glUniform1f(opacityLoc, 1.0f);
            icon.draw();
        }

        glUseProgram(textureShader);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, signature);
        glUniform1f(opacityLoc, 0.5f);
        signature_texture.draw();

        glfwSwapBuffers(window);
        glfwPollEvents();

        double frame_end = glfwGetTime();
        double frame_time = frame_end - frame_start;

        if (is_turned_on && !is_full) {
            water_height += ABS_WATER_FILL_SPEED * frame_time;
            if (water_height >= water_height_max) {
                water_height = water_height_max;
                is_full = true;
                is_turned_on = false;
            }
        }

        double sleep_time = target_frame_time - frame_time;
        if (sleep_time > 0.0) {
            auto sleep_dur = std::chrono::duration<double>(sleep_time);
            std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(sleep_dur));
            while (glfwGetTime() - frame_start < target_frame_time) {}
        }
    }

    text_renderer.cleanup();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
