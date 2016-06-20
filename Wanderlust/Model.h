#include <glew\glew.h>
#include <vector>
#include <string>
#include "shader.h"
#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

class Model
{
public:
	Model(std::string path, std::vector<GLuint> *textures);
	void draw();
	GLuint Model::loadTexture(std::string filePath);
	void update();
	glm::vec3 position = glm::vec3(0,0,0);
	glm::vec3 scale = glm::vec3(1, 1, 1);
	GLfloat angle = 0;
	glm::mat4 viewMatrix;
	glm::vec3 center;
	glm::vec3 minVector;
	glm::vec3 maxVector;

private:
	
	std::vector<Mesh> meshes;
	std::string directory;
	std::vector<GLuint> *textures;
	
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);

	
	
	//For Textures;
	std::vector<GLuint> loadMaterialTextures(aiMaterial* mat);
};