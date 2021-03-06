#include "ParticleSystem.h"
#include <glew\glew.h>
#include <glm\glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <iostream>
#include <memory>
#include <cstdlib>
#include <ctime>
#define NUM_PARTICLES 16*5
#define WORK_GROUP_SIZE 16

struct Pstruct
{
	float x, y, z, w; //positions
	float vx, vy, vz, vw; //velocities
	float lx, ly, lz, lw; //lifespans
};

void ParticleSystem::initialize(glm::vec3 &playerPos)
{
	srand(time(0));
	for (int i = 0; i < 5; i++)
	{
		wgPos[i].x = 3.0 + i / 10.0;
		wgPos[i].y = 1.5;
		wgPos[i].z = 0.0 + i / 10.0;
	}

	parSSBO;
	glGenBuffers(1,&parSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER,parSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER,NUM_PARTICLES * sizeof(struct Pstruct),nullptr,GL_STATIC_DRAW);

	GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	//std::cout << playerPos.x << " " << playerPos.y << " " << playerPos.z << "\n";

	struct Pstruct *par = (struct Pstruct *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER,0,NUM_PARTICLES * sizeof(struct Pstruct),bufMask);
	
	for (int i = 0; i < NUM_PARTICLES; i++)
	{
		/*par[i].x = playerPos.x;
		par[i].y = playerPos.y;
		par[i].z = playerPos.z;

		par[i].vx = 0.00;
		par[i].vy = 0.1;
		par[i].vz = 0.00;

		par[i].lx = 1000.0;*/

		par[i].x = wgPos[(int)(i / (float)WORK_GROUP_SIZE)].x;
		par[i].y = wgPos[(int)(i / (float)WORK_GROUP_SIZE)].y;
		par[i].z = wgPos[(int)(i / (float)WORK_GROUP_SIZE)].z;

		par[i].vx = (rand() / (float)RAND_MAX * 1.0) * (rand() % 2 == 0 ? -1.0 : 1.0);
		par[i].vy = (rand() / (float)RAND_MAX * 1.0) * (rand() % 2 == 0 ? -1.0 : 1.0);
		par[i].vz = (rand() / (float)RAND_MAX * 1.0) * (rand() % 2 == 0 ? -1.0 : 1.0);

		//std::cout << "x-velocity: " << par[i].vx << " y-velocity: " << par[i].vy << " z-velocity: " << par[i].vz << std::endl;

		par[i].lx = 10.0;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


	programHandle = glCreateProgram();

	if (programHandle == 0){
		std::cerr << "failed to create shader programm" << std::endl;
		system("PAUSE");
		exit(EXIT_FAILURE);
	}

	std::ifstream shaderFile("../Shader/particle.comp");

	if (shaderFile.good()){
		auto code = std::string(std::istreambuf_iterator<char>(shaderFile),
			std::istreambuf_iterator<char>());

		shaderFile.close();

		auto parHandle = glCreateShader(GL_COMPUTE_SHADER);
		if (parHandle == 0)
		{
			std::cerr << "Failed to create shader" << std::endl;
			system("Pause");
			exit(EXIT_FAILURE);
		}

		auto code_ptr = code.c_str();
		glShaderSource(parHandle, 1, &code_ptr, nullptr);
		glCompileShader(parHandle);

		GLint succeeded;
		glGetShaderiv(parHandle, GL_COMPILE_STATUS, &succeeded);
		if (!succeeded || !glIsShader(parHandle)){
			GLint logSize;
			glGetShaderiv(parHandle, GL_INFO_LOG_LENGTH, &logSize);

			auto msg = std::make_unique<char[]>(logSize);
			glGetShaderInfoLog(parHandle, logSize, nullptr, msg.get());

			std::cerr << "Failed to compile shader"
				<< std::endl << msg.get() << std::endl;

			system("PAUSE");

			msg.reset(nullptr);
			exit(EXIT_FAILURE);
		}

		glAttachShader(programHandle, parHandle);

		glLinkProgram(programHandle);

		glGetProgramiv(programHandle, GL_LINK_STATUS, &succeeded);
		if (!succeeded){
			GLint logSize;

			glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logSize);

			auto msg = std::make_unique<char[]>(logSize);
			glGetProgramInfoLog(programHandle, logSize, nullptr, msg.get());

			std::cerr << "Failed to link program"
				<< std::endl << msg.get() << std::endl;
			system("PAUSE");

			msg.reset(nullptr);
			exit(EXIT_FAILURE);
		}

		// Initialize buffers and vertex array object for dual use with vertex/fragment shader
		glEnable(GL_PROGRAM_POINT_SIZE);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, parSSBO);
		glGenVertexArrays(1, &parVAO);
		glBindVertexArray(parVAO);
		glBindBuffer(GL_ARRAY_BUFFER, parSSBO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Pstruct), 0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(Pstruct), (GLvoid*)offsetof(Pstruct, lx));
		glBindVertexArray(0);

		// Load shader programs for rendering
		renderShader = std::make_unique<cgue::Shader>("../Shader/particle.vert",
			"../Shader/particle.frag");
		timer = 0;
		oldTime = glfwGetTime();
	}
	else{
		std::cerr << "Failed to open shader file " << "Shader/particle.comp" << std::endl;
		system("Pause");
		exit(EXIT_FAILURE);
	}
}

void ParticleSystem::compute()
{
	glUseProgram(programHandle);
	timer = (glfwGetTime() - oldTime) * 0.0001;
	glUniform1f(4, timer);
	glUniform3fv(5, 5, (float*) wgPos);
	glDispatchCompute(NUM_PARTICLES / WORK_GROUP_SIZE, 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void ParticleSystem::draw(glm::mat4x4 &mvp)
{
	renderShader->useShader();
	glUniformMatrix4fv(3, 1, GL_FALSE, glm::value_ptr(mvp));
	glBindVertexArray(parVAO);
	glDrawArrays(GL_POINTS, 0, NUM_PARTICLES);


	/*struct Pstruct *par = (struct Pstruct *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, NUM_PARTICLES * sizeof(struct Pstruct), GL_MAP_READ_BIT);
	std::cout << par[0].y << "\n";
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);*/
}

void ParticleSystem::clean()
{

}