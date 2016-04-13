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
private:
	
	std::vector<Mesh> meshes;
	std::string directory;
	
	void loadModel(std::string path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene);
	//For Textures;
	/*std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
		std::string typeName)*/;
};