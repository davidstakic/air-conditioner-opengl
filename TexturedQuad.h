#pragma once
#include <GL/glew.h>

class TexturedQuad {
public:
    unsigned int VAO, VBO;
    float x1, y1, x2, y2;

    TexturedQuad(float x1, float y1, float x2, float y2) {
        this->x1 = x1; this->y1 = y1; this->x2 = x2; this->y2 = y2;

        float vertices[] = {
            x1, y1, 0.0f, 0.0f,
            x1, y2, 0.0f, 1.0f,
            x2, y2, 1.0f, 1.0f,
            x2, y1, 1.0f, 0.0f
        };

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
    }

    void draw() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }
};
