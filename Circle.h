#pragma once
#include <GL/glew.h>

class Circle {
public:
    unsigned int VAO, VBO;
    int segments;
    float radius, cx, cy;

    Circle(float x, float y, float r, float aspect, int segs = 50);
    void draw();
};
