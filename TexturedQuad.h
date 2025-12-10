#pragma once
#include <GL/glew.h>

class TexturedQuad {
public:
    unsigned int VAO, VBO;
    float x1, y1, x2, y2;

    TexturedQuad(float x1, float y1, float x2, float y2);
    void draw();
};
