#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Header/Util.h"
#include "../Rectangle.h";
#include "../Circle.h"

#include <chrono>
#include <thread>

// Main fajl funkcija sa osnovnim komponentama OpenGL programa
// Projekat je dozvoljeno pisati počevši od ovog kostura
// Toplo se preporučuje razdvajanje koda po fajlovima (i eventualno potfolderima) !!!
// Srećan rad!
int main()
{
    if (!glfwInit()) return endProgram("GLFW inicijalizacija nije uspela.");

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWmonitor* primary = glfwGetPrimaryMonitor();
    if (!primary) return endProgram("Ne moze da se dobije primary monitor.");
    const GLFWvidmode* mode = glfwGetVideoMode(primary);
    if (!mode) return endProgram("Ne moze da se dobije video mode monitora.");

    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "Air Conditioner", primary, NULL);
    if (window == NULL) return endProgram("Prozor nije uspeo da se kreira.");
    glfwMakeContextCurrent(window);

    glfwSwapInterval(0);

    if (glewInit() != GLEW_OK) return endProgram("GLEW nije uspeo da se inicijalizuje.");

    unsigned int rectShader = createShader("rect.vert", "rect.frag");

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    float aspect = (float)width / (float)height;

    Rectangle klima(-0.378f, 0.6885f, 0.378f, 0.9415f);
    Rectangle precaga(-0.304f, 0.692f, 0.304f, 0.728f);
    Circle lampica(0.355f, 0.71f, 0.010f, aspect);
    float ekranGap = 0.015f;
    Rectangle ekran1(-0.20f - ekranGap, 0.8f, -0.12f - ekranGap, 0.88f);
    Rectangle ekran2(-0.04f, 0.8f, 0.04f, 0.88f);
    Rectangle ekran3(0.12f + ekranGap, 0.8f, 0.20f + ekranGap, 0.88f);
    Rectangle lavor(-0.22f, -0.805f, 0.22f, -0.545f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glClearColor(0.2f, 0.8f, 0.6f, 1.0f);

    const double targetFPS = 75.0;
    const double targetFrameTime = 1.0 / targetFPS;

    while (!glfwWindowShouldClose(window))
    {
        double frameStart = glfwGetTime();

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(rectShader);
        int colorLoc = glGetUniformLocation(rectShader, "uColor");

        glUniform4f(colorLoc, 1.0f, 1.0f, 1.0f, 1.0f);
        klima.draw();

        glUniform4f(colorLoc, 0.4f, 0.4f, 0.4f, 1.0f);
        precaga.draw();

        glUniform4f(colorLoc, 0.2f, 0.2f, 0.2f, 1.0f);
        lampica.draw();

        glUniform4f(colorLoc, 0.0f, 0.0f, 0.0f, 1.0f);
        ekran1.draw();
        ekran2.draw();
        ekran3.draw();

        glUniform4f(colorLoc, 0.8f, 0.8f, 0.8f, 0.3f);
        lavor.draw();

        glBindVertexArray(0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        double frameEnd = glfwGetTime();
        double frameTime = frameEnd - frameStart;
        double sleepTime = targetFrameTime - frameTime;

        if (sleepTime > 0.0) {
            auto sleepDur = std::chrono::duration<double>(sleepTime);
            std::this_thread::sleep_for(std::chrono::duration_cast<std::chrono::milliseconds>(sleepDur));
            while (glfwGetTime() - frameStart < targetFrameTime) {}
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
