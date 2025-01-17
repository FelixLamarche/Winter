#pragma once
#include <memory>
#include <string>
#include <vector>

#include "assimp/material.h"

#include "mesh.h"
#include "texture.h"

class Shader;
struct aiNode;
struct aiScene;
struct aiMesh;

class Model {
public:
	Model(const std::string& path);
	void draw(Shader& shader) const;
	std::vector<Mesh> meshes;
private:
	static std::vector<Texture> texturesLoaded;
	std::string directory;

	void loadModel(const std::string& path);
	void processNode(aiNode* node, const aiScene* scene);
	Mesh processMesh(aiMesh* mesh, const aiScene* scene) const;
	std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName) const;
};