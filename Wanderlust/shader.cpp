#include "shader.h"

#include <iostream>
#include <fstream>
#include <memory>

using namespace cgue;
using namespace std;

Shader::Shader(const std::string& vertexShader,
			   const std::string& fragmentShader)
	:programHandle(0)
	, vertexHandle(0)
	, fragmentHandle(0)
{
	programHandle = glCreateProgram();

	if (programHandle == 0){
		cerr << "failed to create shader programm" << endl;
		system("PAUSE");
		exit(EXIT_FAILURE);
	}

	loadShader(vertexShader, GL_VERTEX_SHADER, vertexHandle);
	loadShader(fragmentShader, GL_FRAGMENT_SHADER, fragmentHandle);

	link();
}
Shader::~Shader()
{
	glDeleteProgram(programHandle);
	glDeleteShader(vertexHandle);
	glDeleteShader(fragmentHandle);
}

void Shader::useShader() const
{
	glUseProgram(programHandle);
}

void Shader::loadShader(const std::string& shader,
	GLenum shaderType, GLuint& handle)
{
	std::ifstream shaderFile(shader);

	if (shaderFile.good()){
		auto code = std::string(std::istreambuf_iterator<char>(shaderFile),
			std::istreambuf_iterator<char>());

		shaderFile.close();

		handle = glCreateShader(shaderType);
		if (handle == 0)
		{
			std::cerr << "Failed to create shader" << std::endl;
			system("Pause");
			exit(EXIT_FAILURE);
		}

		auto code_ptr = code.c_str();
		glShaderSource(handle, 1, &code_ptr, nullptr);
		glCompileShader(handle);

		GLint succeeded;
		glGetShaderiv(handle, GL_COMPILE_STATUS, &succeeded);
		if (!succeeded || !glIsShader(handle)){
			GLint logSize;
			glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logSize);

			auto msg = std::make_unique<char[]>(logSize);
			glGetShaderInfoLog(handle, logSize, nullptr, msg.get());

			cerr << "Failed to compile shader"
				<< std::endl << msg.get() << std::endl;

			system("PAUSE");

			msg.reset(nullptr);
			exit(EXIT_FAILURE);
		}
	}
	else{
		cerr << "Failed to open shader file " << shader << endl;
		system("Pause");
		exit(EXIT_FAILURE);
	}
}
void Shader::link()
{
	glAttachShader(programHandle, vertexHandle);
	glAttachShader(programHandle, fragmentHandle);

	glLinkProgram(programHandle);

	GLint succeeded;
	glGetProgramiv(programHandle, GL_LINK_STATUS, &succeeded);
	if (!succeeded){
		GLint logSize;

		glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logSize);

		auto msg = std::make_unique<char[]>(logSize);
		glGetProgramInfoLog(programHandle, logSize, nullptr, msg.get());

		cerr << "Failed to link program"
			<< std::endl << msg.get() << std::endl;
		system("PAUSE");

		msg.reset(nullptr);
		exit(EXIT_FAILURE);
	}
}