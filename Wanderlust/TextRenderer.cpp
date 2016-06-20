#include <iostream>
#include <glew\glew.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "TextRenderer.h"

TextRenderer::TextRenderer()
{
	// Enable freetype
	FT_Library ft;
	FT_Face face;
	if (FT_Init_FreeType(&ft))
	{
		std::cout << "FreeType initialization error\n";
		exit(EXIT_FAILURE);
	}
	if (FT_New_Face(ft, "..\\fonts\\yanone.ttf", 0, &face))
	{
		std::cout << "Failed to load font\n";
		exit(EXIT_FAILURE);
	}
	FT_Set_Pixel_Sizes(face, 0, 48);

	// Store all ascii characters
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	for (unsigned char c = 0; c < 128; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) continue;

		GLuint glyph;
		glGenTextures(1, &glyph);
		glBindTexture(GL_TEXTURE_2D, glyph);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, face->glyph->bitmap.width,face->glyph->bitmap.rows,
			0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Now store character for later use
		Character character = {
			glyph,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		charmap.insert(std::pair<char, Character>(c, character));
	}

	// Cleanup
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	// Load shaders
	textProgram =std::make_unique<cgue::Shader>("../Shader/text.vert", "../Shader/text.frag");

	// Initialize vao and vbo for text rendering
	glGenVertexArrays(1, &tvao);
	glGenBuffers(1, &tvbo);
	glBindVertexArray(tvao);
	glBindBuffer(GL_ARRAY_BUFFER, tvbo);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	projection = glm::ortho(0.0f, 800.0f, 0.0f, 600.0f);
}

void TextRenderer::drawText(std::string text, float relX, float relY, float scale)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	textProgram->useShader();
	glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(projection));
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(tvao);
	for (unsigned int i = 0; i < text.length(); i++)
	{
		Character c = charmap[text[i]];
		GLfloat xpos = relX + c.bearing.x * scale;
		GLfloat ypos = relY - (c.size.y - c.bearing.y) * scale;
		GLfloat w = c.size.x * scale;
		GLfloat h = c.size.y * scale;
		GLfloat vertices[6][4] = {
			{ xpos, ypos + h, 0.0, 0.0 },
			{ xpos, ypos, 0.0, 1.0 },
			{ xpos + w, ypos, 1.0, 1.0 },
			{ xpos, ypos + h, 0.0, 0.0 },
			{ xpos + w, ypos, 1.0, 1.0 },
			{ xpos + w, ypos + h, 1.0, 0.0 }
		};

		glBindTexture(GL_TEXTURE_2D, c.tex);
		glBindBuffer(GL_ARRAY_BUFFER, tvbo);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		relX += (c.advance >> 6) * scale;
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glDisable(GL_BLEND);
}