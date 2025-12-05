#pragma once
#include <GL/glew.h>

class Rectangle {
public:
    unsigned int VAO, VBO;

    Rectangle(float x1, float y1, float x2, float y2);
    void draw();
};
