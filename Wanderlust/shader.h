#pragma once

#include <string>
#include <glew\glew.h>
#include <GLFW\glfw3.h>

namespace cgue
{
	class Shader
	{
	public:
		Shader(const std::string& vertexShader,
			const std::string& fragmentShader);
		~Shader();

		void useShader()const;

		GLuint programHandle;
	private:
		void loadShader(const std::string& shader,
			GLenum shaderType, GLuint& handle);
		void link();

		GLuint vertexHandle;
		GLuint fragmentHandle;
	};
}