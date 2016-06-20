#include <glew\glew.h>
#include <GLFW\glfw3.h>
#include <memory>
#include "shader.h"

class Contour 
{
public:

	GLuint big_tex;
	GLuint big_fbo;
	GLuint laplace_vao;
	GLuint windowWidth;
	GLuint windowHeight;
	GLfloat offset[50];
	std::unique_ptr<cgue::Shader> laplaceShader;

	void initialize(GLuint width, GLuint height);
	void activate();
	void deactivate();
};