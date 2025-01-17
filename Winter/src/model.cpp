#include "model.h"

#include <algorithm>
#include <string>
#include <vector>
#include <iostream>

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "mesh.h"

std::vector<Texture> Model::texturesLoaded;

Model::Model(const std::string& path)
{
	loadModel(path);
}

void Model::draw(Shader& shader) const
{
	for (const auto& mesh : meshes)
	{
		mesh.draw(shader);
	}
}

void Model::loadModel(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
		return;
	}
	directory = path.substr(0, path.find_last_of('/'));

	processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
	// process all the node's meshes (if any)
	if (meshes.size() == 0)
	{
		meshes.reserve(scene->mNumMeshes);
	}
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes.push_back(processMesh(mesh, scene));
	}
	// then do the same for each of its children
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) const
{
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	vertices.reserve(mesh->mNumVertices);
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		// process vertex positions, normals and texture coordinates
		const glm::vec3 pos(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
		const glm::vec3 normal(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);
		glm::vec2 texCoords = glm::vec2(0.0f, 0.0f);
		const bool doesMeshContainTexCoords = mesh->mTextureCoords[0];
		if (doesMeshContainTexCoords)
		{
			texCoords = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
		}
		vertices.emplace_back(pos, normal, texCoords);
	}

	// process indices
	indices.reserve(static_cast<size_t>(mesh->mNumFaces) * 3);
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		const aiFace face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// process material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
		const std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, Texture::DIFFUSE_TYPENAME);
		const std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, Texture::SPECULAR_TYPENAME);

		textures.reserve(diffuseMaps.size() + specularMaps.size());
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName) const
{
	std::vector<Texture> textures;
	textures.reserve(mat->GetTextureCount(type));
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString strPath;
		mat->GetTexture(type, i, &strPath);
		const std::string path = directory + '/' + strPath.C_Str();
		
		const auto it = std::find_if(texturesLoaded.begin(), texturesLoaded.end(), 
			[&path](Texture t) { return t.path == path; }
		);
		const bool isAlreadyLoaded = it != texturesLoaded.end();

		if (isAlreadyLoaded)
		{
			textures.push_back(*it);
		}
		else
		{
			bool loadAsSRGB = false;
			if (typeName == Texture::DIFFUSE_TYPENAME)
			{
				loadAsSRGB = true;
			}
			const unsigned int id = Texture::loadTexture(path, loadAsSRGB);
			textures.emplace_back(id, typeName, path);
			texturesLoaded.emplace_back(id, typeName, path);
		}
	}
	return textures;
}