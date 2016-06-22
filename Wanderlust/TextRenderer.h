#include <unordered_map>
#include <memory>
#include "shader.h"
#include <GLFW\glfw3.h>
#include <glm\vec2.hpp>
#include <glm\mat4x4.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H 

struct Character
{
	GLuint tex;
	glm::ivec2 size;
	glm::ivec2 bearing;
	GLuint advance;
};

class TextRenderer
{
private:
	GLuint tvao;
	GLuint tvbo;
	std::unique_ptr<cgue::Shader> textProgram;
	std::unordered_map<char, Character> charmap;
	glm::mat4 projection;
public:
	TextRenderer(float width, float height);
	void drawText(std::string text, float relX, float relY, float scale);
};