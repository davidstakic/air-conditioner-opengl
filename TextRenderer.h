#pragma once
#include <map>
#include <string>
#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H

struct Color {
    float r, g, b;
    Color(float R = 1, float G = 1, float B = 1) : r(R), g(G), b(B) {}
};

class TextRenderer {
public:
    bool init(const std::string& fontPath, unsigned int shader);
    void render(const std::string& text, float x, float y, float scale, const Color& color);
    void cleanup();

private:
    struct Character {
        unsigned int TextureID;
        int SizeX, SizeY;
        int BearingX, BearingY;
        unsigned int Advance;
    };

    std::map<char, Character> characters;
    unsigned int VAO = 0, VBO = 0;
    unsigned int shaderID = 0;
};
