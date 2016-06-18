#include "ParticleSystem.h"
#include <glew\glew.h>
#include <glm\glm.hpp>
#define NUM_PARTICLES 1024*1024
#define WORK_GROUP_SIZE 128

struct Pstruct
{
	float x, y, z, w; //positions
	float vx, vy, vz, vw; //velocities
};

void ParticleSystem::initialize(glm::vec3 playerPos)
{
	GLuint parSSBO;
	glGenBuffers(1,&parSSBO);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER,parSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER,NUM_PARTICLES * sizeof(struct Pstruct),nullptr,GL_STATIC_DRAW);

	GLint bufMask = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;

	struct Pstruct *par = (struct Pstruct *) glMapBufferRange(GL_SHADER_STORAGE_BUFFER,0,NUM_PARTICLES * sizeof(struct Pstruct),bufMask);
	for (int i = 0; i < NUM_PARTICLES; i++)
	{
		par[i].x = playerPos.x;
		par[i].y = playerPos.y;
		par[i].z = playerPos.z;
		par[i].z = 1.0;

		par[i].vx = 10.0;
		par[i].vy = 0.0;
		par[i].vz = 10.0;
		par[i].vw = 0.0;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

void ParticleSystem::draw(glm::vec3 playerPos)
{

}

void ParticleSystem::clean()
{

}