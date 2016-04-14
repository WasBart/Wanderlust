#pragma once
// Std. Includes
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glew/glew.h> // Contains all the necessery OpenGL includes
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "shader.h"



struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

	class Mesh {
	public:
		/*  Mesh Data  */
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		std::vector<GLuint> textures;
		/*  Functions  */
		Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<GLuint> textures);
		void draw(cgue::Shader* shader);
	private:
		/*  Render data  */
		GLuint VAO, VBO, EBO;
		/*  Functions    */
		void setupMesh();
	};
