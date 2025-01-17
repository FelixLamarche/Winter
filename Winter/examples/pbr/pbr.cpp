// PBR (Physically Based Rendering): Diffuse Irradiance
// https://learnopengl.com/PBR/IBL/Diffuse-irradiance
#include <array>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
#include <random>
#include <sstream>
#include <string>
#include <vector>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "stb_image.h"

#include "camera.h"
#include "fpsCounter.h"
#include "model.h"
#include "pathManager.h"
#include "shader.h"
#include "texture.h"

// Time
float deltaTime = 0.0f;
float lastFrameTime = 0.0f;

// Input
bool firstMouseInput = true;
float lastMouseX = 0.0f;
float lastMouseY = 0.0f;

// Camera
Camera* pCamera = nullptr;
const float cameraSensitivity = 0.05f;
const float cameraMoveSpeed = 2.5f;

// Light
glm::vec3 lightAmbient(0.1f, 0.1f, 0.1f);
glm::vec3 lightDiffuse(0.9f, 0.9f, 0.9f);
glm::vec3 lightSpecular(10.0f, 10.0f, 10.0f);

glm::vec3 lightScale(0.33f);
glm::vec3 pointLightPositions[] = {
    glm::vec3( 1.5f,  1.0f,  3.0f),
    glm::vec3( 2.5f,  1.0f, -3.0f),
    glm::vec3(-1.5f,  1.0f,  3.0f),
    glm::vec3(-2.5f,  1.0f, -3.0f)
};

glm::vec3 lightColors[] = {
	glm::vec3(1.0f, 0.0f, 0.0f),
	glm::vec3(0.0f, 1.0f, 0.0f),
	glm::vec3(0.0f, 0.0f, 1.0f),
	glm::vec3(1.0f, 1.0f, 1.0f)
};

// MISC

void mouseCallback(GLFWwindow* window, double xPos, double yPos);
void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

Mesh createQuad();
void setShaderLights(Shader& shader);
void renderQuad();
void renderSphere();

int main()
{
    // Init Path
    PathManager::projectPath = std::filesystem::current_path().string() + "/";

    // DATA
    // ------------------------------------
    const int32_t WINDOW_WIDTH = 800;
    const int32_t WINDOW_HEIGHT = 600;
    const std::string WINDOW_TITLE = "LearnOpenGL";

    const std::string PATH_EXAMPLE = PathManager::getProjectPath() + "examples/pbr/";

	const std::string PATH_PBR_VERTEX_SHADER = PATH_EXAMPLE + "pbr.vert";
	const std::string PATH_PBR_FRAGMENT_SHADER = PATH_EXAMPLE + "pbr.frag";
    
	const std::string PATH_TEXTURE_RUSTED_IRON_ALBEDO = PathManager::getTexturesPath() + "pbr/rusted_iron/albedo.png";
	const std::string PATH_TEXTURE_RUSTED_IRON_NORMAL = PathManager::getTexturesPath() + "pbr/rusted_iron/normal.png";
	const std::string PATH_TEXTURE_RUSTED_IRON_METALLIC = PathManager::getTexturesPath() + "pbr/rusted_iron/metallic.png";
	const std::string PATH_TEXTURE_RUSTED_IRON_ROUGHNESS = PathManager::getTexturesPath() + "pbr/rusted_iron/roughness.png";
	const std::string PATH_TEXTURE_RUSTED_IRON_AO = PathManager::getTexturesPath() + "pbr/rusted_iron/ao.png";

    // INIT GLFW
    // ------------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    // For easy MSAA Don't forget to call: glEnable(GL_MULTISAMPLE);
	// ------------------------------------
	glfwWindowHint(GLFW_SAMPLES, 4);
	// ------------------------------------
#ifdef __APPLE__
    // MAC only line to enable forward compatibility
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE.c_str(), nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

    // CAMERA
	// ------------------------------------
    const glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    const glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    const glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
    const float cameraYaw = -90.0f;
    const float cameraRoll = 0.0f;
    const float cameraPitch = 0.0f;
    const glm::vec3 cameraRollYawPitch(cameraRoll, cameraYaw, cameraPitch);
    const float cameraFOV = 45.0f;
    const float cameraNearPlane = 0.05f;
    const float cameraFarPlane = 250.0f;

	Camera camera(cameraPos, cameraFront, cameraUp, cameraRollYawPitch, cameraFOV, cameraNearPlane, cameraFarPlane);
	pCamera = &camera;

    // SHADERS
    // ------------------------------------
	Shader pbrShader(PATH_PBR_VERTEX_SHADER, PATH_PBR_FRAGMENT_SHADER);

    // TEXTURES
	// ------------------------------------
	unsigned int rustedIronAlbedoMap = Texture::loadTexture(PATH_TEXTURE_RUSTED_IRON_ALBEDO.c_str(), true);
	unsigned int rustedIronNormalMap = Texture::loadTexture(PATH_TEXTURE_RUSTED_IRON_NORMAL.c_str(), false);
	unsigned int rustedIronMetallicMap = Texture::loadTexture(PATH_TEXTURE_RUSTED_IRON_METALLIC.c_str(), false);
	unsigned int rustedIronRoughnessMap = Texture::loadTexture(PATH_TEXTURE_RUSTED_IRON_ROUGHNESS.c_str(), false);
	unsigned int rustedIronAOMap = Texture::loadTexture(PATH_TEXTURE_RUSTED_IRON_AO.c_str(), false);


    // OpenGL expects the 0.0 coordinate on the y-axis to be on the bottom side of the image, 
    // but images usually have 0.0 at the top of the y-axis. 
    // stb_image.h can flip the y-axis during image loading by adding the following statement before loading any image: 


    // Uniform Buffers
	// ------------------------------------
    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

    unsigned int uboIndexPBR = glGetUniformBlockIndex(pbrShader.getID(), "Matrices");
    glUniformBlockBinding(pbrShader.getID(), uboIndexPBR, 0);

    // Light
	// ------------------------------------
    const unsigned int NR_LIGHTS = 4;
    glm::vec3 lightPositions[] = {
        glm::vec3(-10.0f,  10.0f, 10.0f),
        glm::vec3(10.0f,  10.0f, 10.0f),
        glm::vec3(-10.0f, -10.0f, 10.0f),
        glm::vec3(10.0f, -10.0f, 10.0f),
    };
    glm::vec3 lightColors[] = {
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f),
        glm::vec3(300.0f, 300.0f, 300.0f)
    };
    // Models and Meshes
	// ------------------------------------
    Mesh quad = createQuad();

    // Render Loop
    // ------------------------------------
	lastFrameTime = static_cast<float>(glfwGetTime());
    FPSCounter fpsCounter(1.0f);
    unsigned int frameCount = 0;

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    while (!glfwWindowShouldClose(window))
    {
        frameCount++;
		const float curFrameTime = static_cast<float>(glfwGetTime());
		deltaTime = curFrameTime - lastFrameTime;
		fpsCounter.update(curFrameTime);
		if (frameCount % 60 == 0)
		{
			fpsCounter.showFPS();
		}
        // input
        processInput(window);

        // rendering commands
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glEnable(GL_MULTISAMPLE);
        glEnable(GL_FRAMEBUFFER_SRGB);

        // matrixes
		glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = camera.getProjectionMatrix(WINDOW_WIDTH, WINDOW_HEIGHT);
		glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        glm::vec3 viewPos = camera.GetPosition();


        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Draw scene
		// ------------------------------------
		// Use shader program
        pbrShader.use();
		pbrShader.setVec2("texScale", glm::vec2(1.0f));
		pbrShader.setVec3("camPos", viewPos);
		pbrShader.setMat4("model", value_ptr(model));

        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, rustedIronAlbedoMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, rustedIronNormalMap);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, rustedIronMetallicMap);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, rustedIronRoughnessMap);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, rustedIronAOMap);
		pbrShader.setInt("albedoMap", 0);
		pbrShader.setInt("normalMap", 1);
		pbrShader.setInt("metallicMap", 2);
		pbrShader.setInt("roughnessMap", 3);
		pbrShader.setInt("aoMap", 4);

		pbrShader.setFloat("ao", 1.0f);
		pbrShader.setFloat("metallic", 0.0f);
		pbrShader.setFloat("roughness", 0.5f);
		pbrShader.setVec3("albedo", glm::vec3(0.5f, 0.0f, 0.0f));

        for (unsigned int i = 0; i < NR_LIGHTS; i++)
        {
			std::string lightName = "lightPositions[" + std::to_string(i) + "]";
			pbrShader.setVec3(lightName, lightPositions[i]);
			lightName = "lightColors[" + std::to_string(i) + "]";
			pbrShader.setVec3(lightName, lightColors[i]);
        }

        pbrShader.use();
		int nbRows = 7;
		int nbColumns = 7;
		float spacing = 2.5;
        for (int row = 0; row < nbRows; ++row)
        {
            pbrShader.setFloat("metallic", (float)row / (float)nbRows);
            for (int col = 0; col < nbColumns; ++col)
            {
                // we clamp the roughness to 0.05 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
                // on direct lighting.
                pbrShader.setFloat("roughness", glm::clamp((float)col / (float)nbColumns, 0.05f, 1.0f));
                pbrShader.setFloat("ao", 1.0f);
                pbrShader.setFloat("metallic", (float)row / (float)nbRows);
                pbrShader.setVec3("albedo", glm::vec3(0.5f, 0.0f, 0.0f));

                model = glm::mat4(1.0f);
                model = glm::translate(model, glm::vec3((col - (nbColumns / 2)) * spacing, (row - (nbRows / 2)) * spacing, 0.0f));
                pbrShader.setMat4("model", value_ptr(model));
                pbrShader.setMat3("normalMatrix", value_ptr(glm::transpose(glm::inverse(glm::mat3(model)))));
                renderSphere();
            }
        }
		model = glm::mat4(1.0f);
		normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
		pbrShader.setMat4("model", value_ptr(model));
		pbrShader.setMat4("normalMatrix", value_ptr(normalMatrix));
        renderSphere();

        // Render lights
        for (unsigned int i = 0; i < sizeof(lightPositions) / sizeof(lightPositions[0]); ++i)
        {
            glm::vec3 newPos = lightPositions[i];
            pbrShader.setVec3("lightPositions[" + std::to_string(i) + "]", newPos);
            pbrShader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);

            model = glm::mat4(1.0f);
            model = glm::translate(model, newPos);
            model = glm::scale(model, glm::vec3(0.5f));
            pbrShader.setMat4("model", value_ptr(model));
            pbrShader.setMat3("normalMatrix", value_ptr(glm::transpose(glm::inverse(glm::mat3(model)))));
			pbrShader.setVec3("albedo", lightColors[i]);
            renderSphere();
        }

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        lastFrameTime = curFrameTime;
    }

    // CLEANUP
    // ------------------------------------

    glfwTerminate();

    return 0;
}

void processInput(GLFWwindow* window)
{
    const float cameraSpeed = cameraMoveSpeed * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
		const glm::vec3 movement = cameraSpeed * pCamera->GetFront();
		pCamera->SetPosition(pCamera->GetPosition() + movement);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        const glm::vec3 movement = -cameraSpeed * pCamera->GetFront();
        pCamera->SetPosition(pCamera->GetPosition() + movement);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
		const glm::vec3 movement = -cameraSpeed * glm::normalize(glm::cross(pCamera->GetFront(), pCamera->GetUp()));
        pCamera->SetPosition(pCamera->GetPosition() + movement);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        const glm::vec3 movement = cameraSpeed * glm::normalize(glm::cross(pCamera->GetFront(), pCamera->GetUp()));
        pCamera->SetPosition(pCamera->GetPosition() + movement);
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        const glm::vec3 movement = cameraSpeed * pCamera->GetUp();
        pCamera->SetPosition(pCamera->GetPosition() + movement);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
    {
        const glm::vec3 movement = -cameraSpeed * pCamera->GetUp();
        pCamera->SetPosition(pCamera->GetPosition() + movement);
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void mouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouseInput)
    {
		lastMouseX = static_cast<float>(xPos);
		lastMouseY = static_cast<float>(yPos);
		firstMouseInput = false;
    }

    float xOffset = static_cast<float>(xPos) - lastMouseX;
    float yOffset = static_cast<float>(yPos) - lastMouseY;
	xOffset *= cameraSensitivity;
	yOffset *= cameraSensitivity;
	lastMouseX = static_cast<float>(xPos);
    lastMouseY = static_cast<float>(yPos);

    // Camera Logic
	const float maxPitch = 89.0f;
	float cameraYaw = pCamera->GetYaw() + xOffset;
	float cameraPitch = pCamera->GetPitch() - yOffset;
	cameraPitch = std::min(cameraPitch, maxPitch);
	cameraPitch = std::max(cameraPitch, -maxPitch);
	
	pCamera->SetYaw(cameraYaw);
	pCamera->SetPitch(cameraPitch);
}

void scrollCallback(GLFWwindow* window, double xOffset, double yOffset)
{
    const float fov = pCamera->GetFOV() - static_cast<float>(yOffset);
	pCamera->SetFOV(fov);
}

void setShaderLights(Shader& shader)
{
    shader.use();

    shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
    shader.setVec3("dirLight.ambient", lightAmbient);
    shader.setVec3("dirLight.diffuse", lightDiffuse);
    shader.setVec3("dirLight.specular", lightSpecular);

    shader.setVec3("spotLight.ambient", lightAmbient);
    shader.setVec3("spotLight.diffuse", lightDiffuse);
    shader.setVec3("spotLight.specular", lightSpecular);
    shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
    shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(14.0f)));
    shader.setFloat("spotLight.constant", 1.0f);
    shader.setFloat("spotLight.linear", 0.09f);
    shader.setFloat("spotLight.quadratic", 0.032f);

    for (int i = 0; i < sizeof(pointLightPositions) / sizeof(glm::vec3); i++)
    {
        std::string pointLightName = "pointLights[" + std::to_string(i) + "]";
        shader.setVec3(pointLightName + ".position", pointLightPositions[i]);
        shader.setVec3(pointLightName + ".ambient", lightAmbient * lightColors[i]);
        shader.setVec3(pointLightName + ".diffuse", lightDiffuse * lightColors[i]);
        shader.setVec3(pointLightName + ".specular", lightSpecular * lightColors[i]);
        shader.setFloat(pointLightName + ".constant", 1.0f);
        shader.setFloat(pointLightName + ".linear", 0.09f);
        shader.setFloat(pointLightName + ".quadratic", 0.032f);
    }
}

Mesh createQuad()
{
    const std::vector<Vertex> verts = {
        Vertex(glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 1.0f)), // top left
        Vertex(glm::vec3(1.0f,  1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 1.0f)), // top right
        Vertex(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(0.0f, 0.0f)), // bottom left
        Vertex(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec2(1.0f, 0.0f)) // bottom right
    };

    const std::vector<unsigned int> indices = {  // note that we start from 0!
        0, 2, 1,   // first triangle
        1, 2, 3    // second triangle
    };

    return Mesh(verts, indices, std::vector<Texture>());
}


unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        // positions
        glm::vec3 pos1(-1.0f, 1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3(1.0f, -1.0f, 0.0f);
        glm::vec3 pos4(1.0f, 1.0f, 0.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


        float quadVertices[] = {
            // positions            // normal         // texcoords  // tangent                          // bitangent
            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

            pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
            pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}


// renders (and builds at first invocation) a sphere
// -------------------------------------------------
unsigned int sphereVAO = 0;
unsigned int indexCount;
void renderSphere()
{
    if (sphereVAO == 0)
    {
        glGenVertexArrays(1, &sphereVAO);

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        std::vector<glm::vec3> positions;
        std::vector<glm::vec2> uv;
        std::vector<glm::vec3> normals;
        std::vector<unsigned int> indices;

        const unsigned int X_SEGMENTS = 64;
        const unsigned int Y_SEGMENTS = 64;
        const float PI = 3.14159265359f;
        for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
        {
            for (unsigned int y = 0; y <= Y_SEGMENTS; ++y)
            {
                float xSegment = (float)x / (float)X_SEGMENTS;
                float ySegment = (float)y / (float)Y_SEGMENTS;
                float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
                float yPos = std::cos(ySegment * PI);
                float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

                positions.push_back(glm::vec3(xPos, yPos, zPos));
                uv.push_back(glm::vec2(xSegment, ySegment));
                normals.push_back(glm::vec3(xPos, yPos, zPos));
            }
        }

        bool oddRow = false;
        for (unsigned int y = 0; y < Y_SEGMENTS; ++y)
        {
            if (!oddRow) // even rows: y == 0, y == 2; and so on
            {
                for (unsigned int x = 0; x <= X_SEGMENTS; ++x)
                {
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                }
            }
            else
            {
                for (int x = X_SEGMENTS; x >= 0; --x)
                {
                    indices.push_back((y + 1) * (X_SEGMENTS + 1) + x);
                    indices.push_back(y * (X_SEGMENTS + 1) + x);
                }
            }
            oddRow = !oddRow;
        }
        indexCount = static_cast<unsigned int>(indices.size());

        std::vector<float> data;
        for (unsigned int i = 0; i < positions.size(); ++i)
        {
            data.push_back(positions[i].x);
            data.push_back(positions[i].y);
            data.push_back(positions[i].z);
            if (normals.size() > 0)
            {
                data.push_back(normals[i].x);
                data.push_back(normals[i].y);
                data.push_back(normals[i].z);
            }
            if (uv.size() > 0)
            {
                data.push_back(uv[i].x);
                data.push_back(uv[i].y);
            }
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);
        unsigned int stride = (3 + 2 + 3) * sizeof(float);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    }

    glBindVertexArray(sphereVAO);
    glDrawElements(GL_TRIANGLE_STRIP, indexCount, GL_UNSIGNED_INT, 0);
}
