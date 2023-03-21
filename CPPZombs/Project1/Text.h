#include "Shader.h"

struct Character {
    uint textureID = 0;            // ID handle of the glyph texture
    iVec2 size = vZeroI;          // Size of glyph
    iVec2 bearing = vZeroI;      // Offset from baseline to left/top of glyph
    uint advance = 0;           // Offset to advance to next glyph
};

class Font
{
public:
    std::map<char, Character> characters{};
    uint minimumSize = 0, vao = 0, vbo = 0;
    int mininumVertOffset = 0, maxVertOffset = 0, vertDisp = 0;

    Font() { }

    Font(FT_Byte* data, FT_Long size, uint minimumSize) : minimumSize(minimumSize)
    {
        FT_Library ft;
        if (FT_Init_FreeType(&ft))
        {
            std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
            return;
        }

        FT_Face face;
        if (FT_New_Memory_Face(ft, data, size, 0, &face))
        {
            std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
            return;
        }

        FT_Set_Pixel_Sizes(face, 0, minimumSize);

        if (FT_Load_Char(face, 'X', FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            return;
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // disable byte-alignment restriction

        for (byte c = 0; c < 128; c++)
        {
            // load character glyph 
            if (FT_Load_Char(face, c, FT_LOAD_RENDER))
            {
                std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
                continue;
            }
            // generate texture
            uint texture;
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexImage2D(
                GL_TEXTURE_2D,
                0,
                GL_RED,
                face->glyph->bitmap.width,
                face->glyph->bitmap.rows,
                0,
                GL_RED,
                GL_UNSIGNED_BYTE,
                face->glyph->bitmap.buffer
            );
            // set texture options
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            // now store character for later use
            Character character = {
                texture,
                iVec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
                iVec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
                static_cast<uint>(face->glyph->advance.x)
            };
            characters.insert(std::pair<char, Character>(c, character));

            mininumVertOffset = min(mininumVertOffset, character.bearing.y - character.size.y); // Always negative
            maxVertOffset = max(mininumVertOffset, character.bearing.y); // Always positive
        }
        vertDisp = maxVertOffset - mininumVertOffset; // Very useful.

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        FT_Done_Face(face);
        FT_Done_FreeType(ft);

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        float position[] = {
            0.0f, 0.0f,
            0.0f, 1.0f,
            1.0f, 1.0f,

            0.0f, 0.0f,
            1.0f, 1.0f,
            1.0f, 0.0f
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2, position, GL_DYNAMIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    int TextWidth(string text)
    {
        uint result = 0;

        for (int i = 0; i < text.size() - 1; i++)
            result += characters[text[i]].advance >> 6; // bitshift by 6 to get value in pixels (2^6 = 64)
        result += characters[text[text.size() - 1]].size.x/* >> 6*/;

        return result;
    }

    void Render(string text, Vec2 pos, float scale, RGBA color)
    {
        scale /= minimumSize;
        float xOffset = 0;
        // activate corresponding render state	
        glUseProgram(textShader);
        glUniform4f(glGetUniformLocation(textShader, "textColor"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(vao);

        // iterate through all characters
        for (string::const_iterator c = text.begin(); c != text.end(); c++)
        {
            Character ch = characters[*c];

            float xpos = (pos.x + xOffset + ch.bearing.x * scale) / ScrWidth();
            float ypos = (pos.y - (ch.size.y - ch.bearing.y) * scale) / ScrHeight();

            float w = ch.size.x * scale / ScrWidth();
            float h = ch.size.y * scale / ScrHeight();
            // render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            glUniform2f(glGetUniformLocation(textShader, "position"), xpos, ypos);
            glUniform2f(glGetUniformLocation(textShader, "scale"), w, h);
            // render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            xOffset += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void RenderRotated(string text, Vec2 pos, float rotation, float scale, RGBA color) // FIX THIS TO ROTATE AROUND CENTER
    {
        scale /= minimumSize;
        float xOffset = 0;
        // activate corresponding render state	
        glUseProgram(rotatedTextShader);
        glUniform1f(glGetUniformLocation(rotatedTextShader, "rotation"), rotation);
        glUniform4f(glGetUniformLocation(rotatedTextShader, "textColor"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
        glActiveTexture(GL_TEXTURE0);
        glBindVertexArray(vao);

        // iterate through all characters
        for (string::const_iterator c = text.begin(); c != text.end(); c++)
        {
            Character ch = characters[*c];

            float xpos = (pos.x + xOffset + ch.bearing.x * scale) / ScrWidth();
            float ypos = (pos.y - (ch.size.y - ch.bearing.y) * scale) / ScrHeight();

            float w = ch.size.x * scale / ScrWidth();
            float h = ch.size.y * scale / ScrHeight();
            // render glyph texture over quad
            glBindTexture(GL_TEXTURE_2D, ch.textureID);
            glUniform2f(glGetUniformLocation(rotatedTextShader, "position"), xpos, ypos);
            glUniform2f(glGetUniformLocation(rotatedTextShader, "scale"), w, h);
            // render quad
            glDrawArrays(GL_TRIANGLES, 0, 6);
            // now advance cursors for next glyph (note that advance is number of 1/64 pixels)
            xOffset += (ch.advance >> 6) * scale; // bitshift by 6 to get value in pixels (2^6 = 64)
        }
        glBindVertexArray(0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
};

Font font;