#pragma once

#include <string>

// Requires projectPath to be set before the paths are usable
class PathManager {
public:
	// Has to be set before using the paths
	static inline std::string projectPath = "";
	static std::string getProjectPath() { return projectPath; }
	static std::string getResourcesPath() { return getProjectPath() + "resources/"; }
	static std::string getTexturesPath() { return getResourcesPath() + "textures/"; }
	static std::string getShadersPath() { return getProjectPath() + "shaders/"; }
	static std::string getModelsPath() { return getResourcesPath() + "objects/"; }
	static std::string getFontsPath() { return getResourcesPath() + "fonts/"; }
};