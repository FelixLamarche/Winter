#include "mesh.h"

#include <iostream>

#include "glad/glad.h"
#include "shader.h"

Mesh::Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures)
	: vertices(vertices), indices(indices), textures(textures), VAO(UNUSED_VAO), VBO(UNUSED_VAO), EBO(UNUSED_VAO)
{
	setupMesh();
}

Mesh::~Mesh()
{
	if (VAO != UNUSED_VAO) // Only delete if it was initialized
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);
	}
}

Mesh::Mesh(const Mesh& other)
	: vertices(other.vertices), indices(other.indices), textures(other.textures), VAO(0), VBO(0), EBO(0)
{
	setupMesh();
}

Mesh& Mesh::operator=(const Mesh& other)
{
	if (this == &other)
	{
		return *this;
	}

	vertices = other.vertices;
	indices = other.indices;
	textures = other.textures;
	setupMesh();

	return *this;
}

Mesh::Mesh(Mesh&& other) noexcept
	: vertices(std::move(other.vertices)), indices(std::move(other.indices)), textures(std::move(other.textures)),
	 VAO(other.VAO), VBO(other.VBO), EBO(other.EBO)
{
	other.VAO = UNUSED_VAO;
	other.VBO = UNUSED_VAO;
	other.EBO = UNUSED_VAO;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
	if (this == &other)
	{
		return *this;
	}

	vertices = std::move(other.vertices);
	indices = std::move(other.indices);
	textures = std::move(other.textures);
	VAO = other.VAO;
	VBO = other.VBO;
	EBO = other.EBO;
	other.VAO = UNUSED_VAO;
	other.VBO = UNUSED_VAO;
	other.EBO = UNUSED_VAO;

	return *this;
}

void Mesh::draw(Shader& shader) const
{
	unsigned int diffuseNb = 0;
	unsigned int specularNb = 0;
	shader.use();
	for (unsigned int i = 0; i < textures.size(); i++)
	{
		// bind to texture
		glActiveTexture(GL_TEXTURE0 + i);
		// get name in shader according to convention
		std::string number;
		std::string name = textures[i].type;
		if (name == Texture::DIFFUSE_TYPENAME)
			number = std::to_string(diffuseNb++);
		else if (name == Texture::SPECULAR_TYPENAME)
			number = std::to_string(specularNb++);

		shader.setInt(("material." + name + number).c_str(), i);
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);
	// draw mesh
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

void Mesh::AddTexture(const Texture& texture)
{
	textures.push_back(texture);
}

void Mesh::RemoveTexture(const std::string& path)
{
	textures.erase(std::remove_if(textures.begin(), textures.end(), [&path](const Texture& texture) { return texture.path == path; }), textures.end());
}

void Mesh::setupMesh()
{
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

	// vertex positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
	// vertex normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	// vertex texture coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	
	glBindVertexArray(0);
}