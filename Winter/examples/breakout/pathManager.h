#pragma once

#include <string>

class PathManager {
public:
	static inline std::string projectPath = "";
	static std::string getProjectPath() { return projectPath; }
	static std::string getResourcesPath() { return getProjectPath() + "resources/"; }
	static std::string getTexturesPath() { return getResourcesPath() + "textures/"; }
	static std::string getShadersPath() { return getProjectPath() + "shaders/"; }
	static std::string getModelsPath() { return getResourcesPath() + "objects/"; }
	static std::string getFontsPath() { return getResourcesPath() + "fonts/"; }
};