#pragma once
#include <GL/glew.h>
#include <vector>
#include <cmath>

class Circle {
public:
    unsigned int VAO, VBO;
    int segments;
    float radius, cx, cy;

    Circle(float x, float y, float r, float aspect, int segs = 50) : cx(x), cy(y), radius(r), segments(segs) {
        std::vector<float> vertices;
        vertices.push_back(cx);
        vertices.push_back(cy);

        for (int i = 0; i <= segments; i++) {
            float angle = i * 2.0f * 3.14159f / segments;
            vertices.push_back(cx + radius * cos(angle));
            vertices.push_back(cy + radius * sin(angle) * aspect);
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    void draw() {
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);
        glBindVertexArray(0);
    }
};

