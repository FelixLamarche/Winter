#pragma once
#include <string>
#include <vector>

#include "glm/glm.hpp"

#include "vertex.h"
#include "texture.h"

class Shader;

class Mesh {
public:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Texture> textures;

	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures);
	~Mesh();
	Mesh(const Mesh& other);
	Mesh& operator=(const Mesh& other);
	Mesh(Mesh&& other) noexcept;
	Mesh& operator=(Mesh&& other) noexcept;

	void draw(Shader& shader) const;
	void AddTexture(const Texture& texture);
	void RemoveTexture(const std::string& path);
	unsigned int VAO, VBO, EBO;
private:
	static const unsigned int UNUSED_VAO = 0;
	
	void setupMesh();
};