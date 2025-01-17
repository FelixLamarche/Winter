#pragma once

#include <string>
#include "glm/glm.hpp"
#include "glad/glad.h"

class Shader
{
private:
    static unsigned int UNUSED_ID;
	static std::string getShaderTypeString(GLenum shaderType);
public:
    // constructor reads and builds the shader
    Shader(const std::string& vertexCode, const std::string& fragmentCode, const std::string& geometryCode = "");
    Shader();
	Shader& operator=(const Shader& other) = default;
    Shader(const Shader& other) = default;
    Shader& operator=(Shader&& other) = default;
    Shader(Shader&& other) = default;
    // use/activate the shader
    void use() const;
    unsigned int getID() const { return ID; }
    // utility uniform functions
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, float v0, float v1) const;
    void setVec2(const std::string& name, const glm::vec2& v) const;
    void setVec3(const std::string& name, float v0, float v1, float v2) const;
    void setVec3(const std::string& name, const glm::vec3& v) const;
    void setVec4(const std::string& name, float v0, float v1, float v2, float v3) const;
    void setVec4(const std::string& name, const glm::vec4& v) const;
    void setMat2(const std::string& name, const float* value) const;
    void setMat3(const std::string& name, const float* value) const;
    void setMat4(const std::string& name, const float* value) const;

    void compileShader(const std::string& vertexCode, const std::string& fragmentCode, const std::string& geometryCode = "");
private:
    unsigned int ID;
	
	unsigned int compileShaderSource(const std::string& shaderCode, GLenum shaderType);
};
