#include "shader.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include "glad/glad.h"

unsigned int Shader::UNUSED_ID = 0;

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath)
	: ID(UNUSED_ID), vertexPath(vertexPath), fragmentPath(fragmentPath), geometryPath(geometryPath)
{
	generateShader(vertexPath, fragmentPath, geometryPath);
}

Shader::~Shader()
{
    if (ID != UNUSED_ID)
    {
	    glDeleteProgram(ID);
    }
}

Shader::Shader(const Shader& other)
	: ID(UNUSED_ID), vertexPath(other.vertexPath), fragmentPath(other.fragmentPath), geometryPath(other.geometryPath)
{
    generateShader(vertexPath, fragmentPath, geometryPath);
}

Shader& Shader::operator=(const Shader& other)
{
    if (this == &other)
    {
        return *this;
    }

    vertexPath = other.vertexPath;
    fragmentPath = other.fragmentPath;
	geometryPath = other.geometryPath;
    generateShader(vertexPath, fragmentPath, geometryPath);
    return *this;
}

Shader::Shader(Shader&& other) noexcept
	: ID(std::move(other.ID)), vertexPath(std::move(other.vertexPath)), fragmentPath(std::move(other.fragmentPath)), 
    geometryPath(std::move(other.geometryPath))
{
	other.ID = UNUSED_ID;
}

Shader& Shader::operator=(Shader&& other) noexcept
{
    if (this == &other)
    {
        return *this;
    }

    ID = std::move(other.ID);
    other.ID = UNUSED_ID;
    vertexPath = std::move(other.vertexPath);
    fragmentPath = std::move(other.fragmentPath);
	geometryPath = std::move(other.geometryPath);
    return *this;
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

void Shader::generateShader(const std::string& vertexPath, const std::string& fragmentPath, const std::string& geometryPath)
{
	unsigned int vertex = compileShaderSource(vertexPath, GL_VERTEX_SHADER);
	unsigned int fragment = compileShaderSource(fragmentPath, GL_FRAGMENT_SHADER);
	unsigned int geometry = 0;
    if (!geometryPath.empty())
    {
		geometry = compileShaderSource(geometryPath, GL_GEOMETRY_SHADER);
    }

    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
	if (!geometryPath.empty())
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
    if (!geometryPath.empty())
    {
		glDeleteShader(geometry);
    }
}

unsigned int Shader::compileShaderSource(const std::string& shaderPath, GLenum shaderType)
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string code;
    std::ifstream shaderFile;
    // ensure ifstream objects can throw exceptions:
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        shaderFile.open(shaderPath);
        std::stringstream vShaderStream;
        vShaderStream << shaderFile.rdbuf();
        shaderFile.close();
        code = vShaderStream.str();
    }
    catch (std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << shaderPath << " " << e.what() << std::endl;
    }

    const char* shaderCode = code.c_str();
    // 2. compile shaders
    unsigned int shaderID;
    int success;
    char infoLog[512];

    shaderID = glCreateShader(shaderType);
    glShaderSource(shaderID, 1, &shaderCode, nullptr);
    glCompileShader(shaderID);
    // print compile errors if any
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(shaderID, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::" << getShaderTypeString(shaderType) << "::COMPILATION_FAILED\n" <<
            shaderPath << "\n" << infoLog << std::endl;
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

