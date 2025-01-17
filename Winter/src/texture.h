#pragma once
#include <string>
#include <vector>

#include "glad/glad.h"

struct Texture {
	unsigned int id;
	std::string type;
	std::string path;

	static const std::string DIFFUSE_TYPENAME;
	static const std::string SPECULAR_TYPENAME;
	static const std::string NORMAL_TYPENAME;
    static unsigned int loadTexture(const std::string& path, bool loadSRGB = true, GLenum wrap = GL_REPEAT);
	static unsigned int loadCubemap(const std::vector<std::string>& faces);
	static unsigned int loadHDR(const std::string& path);
};
