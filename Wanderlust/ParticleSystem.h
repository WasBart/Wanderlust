#include <glm\glm.hpp>
#include <glew\glew.h>
#include <GLFW\glfw3.h>
#include <memory>
#include "shader.h"

class ParticleSystem
{

public:

	GLuint parSSBO;
	GLuint programHandle;
	GLuint parVAO;
	std::unique_ptr<cgue::Shader> renderShader;

	void initialize(glm::vec3 &playerPos);
	void draw(glm::vec3 &playerPos, glm::mat4x4 &mvp);
	void clean();
};
