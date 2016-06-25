#include <glew\glew.h>
#include <string>
#include <FreeImage/FreeImage.h>
#include "Model.h"
#include "shader.h"
#include <glm\glm.hpp>
#include <glm/gtc/type_ptr.hpp>

Model::Model(std::string path, std::vector<GLuint> *textures)
{
	this->textures = textures;
	this->loadModel(path);
};

/*
Drawing all Meshes from this Model
*/
void Model::draw(){
	update();
	for (GLuint i = 0; i < this->meshes.size(); i++){
		this->meshes[i].viewMatrix = viewMatrix;
		this->meshes[i].draw();
	}
}

void Model::updateChild(glm::mat4 modelIn){
	child->meshes[0].viewMatrix = viewMatrix;
	child->meshes[0].update(modelIn);
	child->meshes[0].draw();
}

/*
Updating all Meshes from this Model
*/
void Model::update(){
	glm::mat4 model;
	model = glm::translate(model, position);
	model = glm::rotate(model,angle,glm::vec3(0,1,0));
	model = glm::scale(model, scale);
	outModel = model;
	for (GLuint i = 0; i < this->meshes.size(); i++){
		this->meshes[i].update(model);
	}
}



void Model::loadModel(std::string path)
{
	Assimp::Importer import;
	const aiScene* scene = import.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << import.GetErrorString() << std::endl;
		return;
	}
	this->directory = path.substr(0, path.find_last_of('/'));

	this->processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	
	for (GLuint i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		this->meshes.push_back(this->processMesh(mesh, scene));
	}
	this->center = (minVector + maxVector) / 2.0f;

	for (GLuint i = 0; i < node->mNumChildren; i++)
	{
		this->processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<GLuint> textures;

	bool firstRound = true;
	for (GLuint i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		//Vertex pos
		glm::vec3 vector;
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;

		if (firstRound)
		{
			minVector.x = vector.x;
			minVector.y = vector.y;
			minVector.z = vector.z;

			maxVector.x = vector.x;
			maxVector.y = vector.y;
			maxVector.z = vector.z;

			firstRound = false;
		}
		else
		{
			if (vector.x < minVector.x)
			{
				minVector.x = vector.x;
			}

			if (vector.y < minVector.y)
			{
				minVector.y = vector.y;
			}

			if (vector.z < minVector.z)
			{
				minVector.z = vector.z;
			}

			if (vector.x > maxVector.x)
			{
				maxVector.x = vector.x;
			}

			if (vector.y > maxVector.y)
			{
				maxVector.y = vector.y;
			}

			if (vector.z > maxVector.z)
			{
				maxVector.z = vector.z;
			}

		}

		vertex.Position = vector;
		//Normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;

		if (mesh->mTextureCoords[0]) // No textureCoordinates found
		{
			glm::vec2 vec;
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);

		vertices.push_back(vertex);
	}
	// Process indices
	for (GLuint i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (GLuint j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
		// Process material
		if (mesh->mMaterialIndex >= 0)
		{
			//For Texturing:
			if (mesh->mMaterialIndex >= 0)
			{
				aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
				textures = loadMaterialTextures(material);
			}
		}

	return Mesh(vertices, indices, textures);
}
//For Texturing:
std::vector<GLuint> Model::loadMaterialTextures(aiMaterial* mat)
{
	std::vector<GLuint> textures;
	for (GLuint i = 0; i < mat->GetTextureCount(aiTextureType_DIFFUSE); i++)
	{
		aiString str;
		mat->GetTexture(aiTextureType_DIFFUSE, i, &str);
		textures.push_back(loadTexture(str.C_Str()));
	}
	
	return textures;


}

GLuint Model::loadTexture(std::string filePath)
{
	GLuint textureHandle = 0;
	filePath.insert(0, std::string("../Textures/"));
	//std::cout << filePath << std::endl;
	auto bitMap = FreeImage_Load(FIF_PNG, filePath.c_str());

	if (bitMap == nullptr)
	{
		return textureHandle;
	}

	glGenTextures(1, &textureHandle);
	glBindTexture(GL_TEXTURE_2D, textureHandle);

	//if no alpha bits < 32 (24)
	int a = 0;
	if ((a=FreeImage_GetBPP(bitMap)) < 32){
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FreeImage_GetWidth(bitMap), FreeImage_GetHeight(bitMap), 0, GL_BGR, GL_UNSIGNED_BYTE, FreeImage_GetBits(bitMap));
	}
	//else tex has alpha
	else{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, FreeImage_GetWidth(bitMap), FreeImage_GetHeight(bitMap), 0, GL_BGRA, GL_UNSIGNED_BYTE, FreeImage_GetBits(bitMap));
	}
	FreeImage_Unload(bitMap);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glGenerateMipmap(GL_TEXTURE_2D);
	textures->push_back(textureHandle);
	glBindTexture(GL_TEXTURE_2D, 0);
	return textureHandle;
}