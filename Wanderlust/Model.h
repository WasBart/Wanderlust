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
	
	Model(std::string path);
	void draw(cgue::Shader* shader);
	GLuint Model::loadTexture(std::string filePath);
	void update();
	glm::vec3 position;
	GLfloat angle;

private:
	
	std::vector<Mesh> meshes;
	std::string directory;
	
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	
	//For Textures;
	std::vector<GLuint> loadMaterialTextures(aiMaterial* mat);
};