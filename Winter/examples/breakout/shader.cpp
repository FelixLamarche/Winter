#include "shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "glad/glad.h"

unsigned int Shader::UNUSED_ID = 0;

Shader::Shader(const std::string& vertexCode, const std::string& fragmentCode, const std::string& geometryCode)
	: ID(UNUSED_ID)
{
	compileShader(vertexCode, fragmentCode, geometryCode);
}

Shader::Shader()
	: ID(UNUSED_ID)
{
}

void Shader::use() const
{
    glUseProgram(ID);
}

void Shader::setBool(const std::string& name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec2(const std::string& name, float v0, float v1) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), v0, v1);
}

void Shader::setVec2(const std::string& name, const glm::vec2& v) const
{
    glUniform2f(glGetUniformLocation(ID, name.c_str()), v.x, v.y);
}

void Shader::setVec3(const std::string& name, float v0, float v1, float v2) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), v0, v1, v2);
}

void Shader::setVec3(const std::string& name, const glm::vec3& v) const
{
    glUniform3f(glGetUniformLocation(ID, name.c_str()), v.x, v.y, v.z);
}

void Shader::setVec4(const std::string& name, float v0, float v1, float v2, float v3) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), v0, v1, v2, v3);
}

void Shader::setVec4(const std::string& name, const glm::vec4& v) const
{
    glUniform4f(glGetUniformLocation(ID, name.c_str()), v.x, v.y, v.z, v.w);
}
void Shader::setMat2(const std::string& name, const float* value) const
{
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value);
}

void Shader::setMat3(const std::string& name, const float* value) const
{
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value);
}

void Shader::setMat4(const std::string& name, const float* value) const
{
	glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, value);
}

void Shader::compileShader(const std::string& vertexCode, const std::string& fragmentCode, const std::string& geometryCode)
{
	unsigned int vertex = compileShaderSource(vertexCode, GL_VERTEX_SHADER);
	unsigned int fragment = compileShaderSource(fragmentCode, GL_FRAGMENT_SHADER);
	unsigned int geometry = 0;
    if (!geometryCode.empty())
    {
		geometry = compileShaderSource(geometryCode, GL_GEOMETRY_SHADER);
    }

    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
	if (!geometryCode.empty())
	{
		glAttachShader(ID, geometry);
	}
    glLinkProgram(ID);

    // print linking errors if any
    int success;
    char infoLog[512];
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(ID, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    if (!geometryCode.empty())
    {
		glDeleteShader(geometry);
    }
}

unsigned int Shader::compileShaderSource(const std::string& shaderCode, GLenum shaderType)
{
    const char* shaderCodeCStr = shaderCode.c_str();
    // 2. compile shaders
    unsigned int shaderID;
    int success;
    char infoLog[512];

    shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &shaderCodeCStr, nullptr);
    glCompileShader(shaderID);
    // print compile errors if any
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderID, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::" << getShaderTypeString(shaderType) << "::COMPILATION_FAILED\n" <<
            shaderCode << "\n" << infoLog << std::endl;
    };

	return shaderID;
}

std::string Shader::getShaderTypeString(GLenum shaderType)
{
    switch (shaderType)
    {
        case GL_VERTEX_SHADER:
            return "VERTEX";
        case GL_FRAGMENT_SHADER:
            return "FRAGMENT";
        case GL_GEOMETRY_SHADER:
            return "GEOMETRY";
        case GL_TESS_CONTROL_SHADER:
            return "TESS_CONTROL";
        case GL_TESS_EVALUATION_SHADER:
            return "TESS_EVALUATION";
        case GL_COMPUTE_SHADER:
            return "COMPUTE";
        default:
            return "UNKNOWN";
    }
}

