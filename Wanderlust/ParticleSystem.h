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
	glm::vec3 wgPos[5];
	std::unique_ptr<cgue::Shader> renderShader;
	float timer;
	float oldTime;

	void initialize(glm::vec3 &playerPos);
	void compute();
	void draw(glm::mat4x4 &mvp);
	void clean();
};
