#include <iostream>
#include "Contour.h"

void Contour::initialize(GLuint width, GLuint height)
{
	windowWidth = width;
	windowHeight = height;

	// Initialize laplace shader
	laplaceShader = std::make_unique<cgue::Shader>("../Shader/contour.vert",
		"../Shader/contour.frag");

	// Initialize framebuffers
	GLuint big_depth;
	GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
	glGenFramebuffers(1, &big_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, big_fbo);
	glGenTextures(1, &big_tex);
	glBindTexture(GL_TEXTURE_2D, big_tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glGenRenderbuffers(1, &big_depth);
	glBindRenderbuffer(GL_RENDERBUFFER, big_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, big_depth);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, big_tex, 0);
	glDrawBuffers(1, draw_buffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		exit(EXIT_FAILURE);
	}

	//Calculate offsets
	float xinc = 1.0f / (GLfloat)width;
	float yinc = 1.0f / (GLfloat)height;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			offset[(((i * 5) + j) * 2) + 0] = (-2.0f * xinc) + ((GLfloat)i * xinc);
			offset[(((i * 5) + j) * 2) + 1] = (-2.0f * yinc) + ((GLfloat)j * yinc);
		}
	}
	

	// Create vertex buffer for laplace shader
	glGenVertexArrays(1, &laplace_vao);
	glBindVertexArray(laplace_vao);
	GLuint laplace_vbo;
	GLfloat laplace_trigs[] = {
		-1.0f, 1.0f, 0.0f,
		-1.0f, -1.0f, 0.0f,
		1.0f, 1.0f, 0.0f,
		1.0f, -1.0f, 0.0f
	};
	glGenBuffers(1, &laplace_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, laplace_vbo);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(GLfloat), laplace_trigs, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindVertexArray(0);
}

void Contour::activate()
{
	glBindFramebuffer(GL_FRAMEBUFFER, big_fbo);
	//glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, big_tex, 0);
	//GLenum draw_buffers[1] = { GL_COLOR_ATTACHMENT0 };
	//glDrawBuffers(1, draw_buffers);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glViewport(0, 0, windowWidth, windowHeight);

}

void Contour::deactivate()
{
	glBindFramebuffer(GL_FRAMEBUFFER,0);
	//glViewport(0,0,windowWidth, windowHeight);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, big_tex);
	laplaceShader->useShader();
	glUniform2fv(3, 25, offset);
	glBindVertexArray(laplace_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}