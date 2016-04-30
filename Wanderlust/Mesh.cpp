#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>

#include <glew/glew.h> 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Mesh.h"

#include <glm/gtc/type_ptr.hpp>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<GLuint> indices, std::vector<GLuint> textures){

	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;

	this->setupMesh();
};
/*
Drawing Mesh
*/
void Mesh::draw(cgue::Shader* shader){
	// Binding textures

	if (textures.size() > 0){
		glActiveTexture(GL_TEXTURE0);
		//glUniform1i(glGetUniformLocation(shader->programHandle, "TODO"), i);
		glBindTexture(GL_TEXTURE_2D, this->textures[0]);
	}
	// Drawing mesh
	glUniformMatrix4fv(4, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(5, 1, GL_FALSE, glm::value_ptr(viewMatrix));
	
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);

	//Unbinding
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Mesh::update(glm::mat4 operation){
	modelMatrix = operation;
}
/*
Setting up VAO, VBO, and EBO
*/
void Mesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// Loading data into vertex buffers
	glGenBuffers(1, &VBO); //positionBuffer in Cube
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);


	glGenBuffers(1, &EBO); //indexBuffer in Cube
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	
	// Vertex Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
	
	// Vertex Texture Coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

