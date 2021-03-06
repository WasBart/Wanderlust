#pragma once

//A Model contains several Meshes

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glew/glew.h> 
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
		std::vector<Vertex> vertices;
		std::vector<GLuint> indices;
		std::vector<GLuint> textures;
	
		Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<GLuint> textures);
		void draw();
		void update(glm::mat4 operation);
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		
		
	private:
		
		GLuint VAO, VBO, EBO;
		void setupMesh();
	};
