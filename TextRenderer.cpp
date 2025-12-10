#include "TextRenderer.h"
#include <iostream>

bool TextRenderer::init(const std::string& fontPath, unsigned int shader) {
    shaderID = shader;

    glUseProgram(shaderID);
    GLint projLoc = glGetUniformLocation(shaderID, "projection");
    if (projLoc != -1) {
        float identityProj[16] = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
        };
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, identityProj);
    }

    GLint texLoc = glGetUniformLocation(shaderID, "text");
    if (texLoc != -1)
        glUniform1i(texLoc, 0);

    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cerr << "Failed to init FreeType\n";
        return false;
    }

    FT_Face face;
    if (FT_New_Face(ft, fontPath.c_str(), 0, &face)) {
        std::cerr << "Failed to load font\n";
        return false;
    }

    FT_Set_Pixel_Sizes(face, 0, 48);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;

        unsigned int tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0, GL_RED, GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        Character ch = {
            tex,
            (int)face->glyph->bitmap.width,
            (int)face->glyph->bitmap.rows,
            face->glyph->bitmap_left,
            face->glyph->bitmap_top,
            (unsigned int)face->glyph->advance.x
        };
        characters.insert({ c, ch });
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

    FT_Done_Face(face);
    FT_Done_FreeType(ft);

    return true;
}

void TextRenderer::render(const std::string& text, float x, float y, float scale, const Color& color) {
    glUseProgram(shaderID);
    GLint colorLoc = glGetUniformLocation(shaderID, "textColor");
    if (colorLoc != -1) glUniform3f(colorLoc, color.r, color.g, color.b);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);

    for (auto c : text) {
        const Character& ch = characters[c];
        float xpos = x + ch.BearingX * scale;
        float ypos = y - (ch.SizeY - ch.BearingY) * scale;
        float w = ch.SizeX * scale;
        float h = ch.SizeY * scale;

        float verts[6][4] = {
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos, ypos, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos, ypos + h, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 1.0f},
            {xpos + w, ypos + h, 1.0f, 0.0f}
        };

        glBindTexture(GL_TEXTURE_2D, ch.TextureID);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        x += (ch.Advance >> 6) * scale;
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::cleanup() {
    for (auto& kv : characters)
        glDeleteTextures(1, &kv.second.TextureID);
    characters.clear();
}
