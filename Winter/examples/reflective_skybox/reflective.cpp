// Advanced OpenGL: Cubemaps
// https://learnopengl.com/Advanced-OpenGL/
#include <array>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <map>
#include <numeric>
#include <string>
#include <vector>

#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "stb_image.h"

#include "camera.h"
#include "model.h"
#include "pathManager.h"
#include "shader.h"
#include "texture.h"

// Time
float deltaTime = 0.0f;
float lastFrame = 0.0f;

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
glm::vec3 lightSpecular(1.0f, 1.0f, 1.0f);

glm::vec3 lightScale(0.33f);
glm::vec3 pointLightPositions[] = {
    glm::vec3(0.7f,  0.2f,  2.0f),
    glm::vec3(2.3f, -3.3f, -4.0f),
    glm::vec3(-4.0f,  2.0f, -12.0f),
    glm::vec3(0.0f,  0.0f, -3.0f)
};

// MISC

void mouseCallback(GLFWwindow* window, double xPos, double yPos);
void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

Mesh createQuad();
void setShaderLights(Shader& shader);

int main()
{
    // Init Path
    PathManager::projectPath = std::filesystem::current_path().string() + "/";

    // DATA
    // ------------------------------------
    const int32_t WINDOW_WIDTH = 800;
    const int32_t WINDOW_HEIGHT = 600;
    const std::string WINDOW_TITLE = "LearnOpenGL";

	const std::string PATH_EXAMPLE = PathManager::getProjectPath() + "examples/reflective_skybox/";

	const std::string PATH_VERTEX_SHADER = PATH_EXAMPLE + "basicVertex.glsl";
    const std::string PATH_FRAGMENT_SHADER = PATH_EXAMPLE + "basicFragment.glsl";
    const std::string PATH_LIGHT_VERTEX_SHADER = PATH_EXAMPLE + "lightCubeVertex.glsl";
    const std::string PATH_LIGHT_FRAGMENT_SHADER = PATH_EXAMPLE + "lightCubeFragment.glsl";
	const std::string PATH_SKYBOX_VERTEX_SHADER = PATH_EXAMPLE + "cubemapVertex.glsl";
    const std::string PATH_SKYBOX_FRAGMENT_SHADER = PATH_EXAMPLE + "cubemapFragment.glsl";
    const std::string PATH_REFLECTIVE_VERTEX_SHADER = PATH_EXAMPLE + "reflectiveSkyboxVertex.glsl";
    const std::string PATH_REFLECTIVE_FRAGMENT_SHADER = PATH_EXAMPLE + "reflectiveSkyboxFragment.glsl";

    const std::string PATH_TEXTURE_CONTAINER = PathManager::getTexturesPath() + "container.jpg";
    const std::string PATH_TEXTURE_CONTAINER2 = PathManager::getTexturesPath() + "container2.png";
    const std::string PATH_TEXTURE_CONTAINER2_SPECULAR = PathManager::getTexturesPath() + "container2_specular.png";
	
    const std::string PATH_MODEL_CUBE = PathManager::getModelsPath() + "cube/cube.obj";

    const std::vector<std::string> cubemapFaces = {
         PathManager::getTexturesPath() + "skybox/right.jpg",
         PathManager::getTexturesPath() + "skybox/left.jpg",
         PathManager::getTexturesPath() + "skybox/top.jpg",
         PathManager::getTexturesPath() + "skybox/bottom.jpg",
         PathManager::getTexturesPath() + "skybox/front.jpg",
         PathManager::getTexturesPath() + "skybox/back.jpg"
    };

    // INIT GLFW
    // ------------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

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
    const float cameraNearPlane = 0.1f;
    const float cameraFarPlane = 100.0f;

	Camera camera(cameraPos, cameraFront, cameraUp, cameraRollYawPitch, cameraFOV, cameraNearPlane, cameraFarPlane);
	pCamera = &camera;

    // SHADERS
    // ------------------------------------
    Shader shader(PATH_VERTEX_SHADER, PATH_FRAGMENT_SHADER);
    Shader lightCubeShader(PATH_LIGHT_VERTEX_SHADER, PATH_LIGHT_FRAGMENT_SHADER);
    Shader skyboxShader(PATH_SKYBOX_VERTEX_SHADER, PATH_SKYBOX_FRAGMENT_SHADER);
    Shader reflectiveShader(PATH_REFLECTIVE_VERTEX_SHADER, PATH_REFLECTIVE_FRAGMENT_SHADER);

    // TEXTURES
	// ------------------------------------

    // OpenGL expects the 0.0 coordinate on the y-axis to be on the bottom side of the image, 
    // but images usually have 0.0 at the top of the y-axis. 
    // stb_image.h can flip the y-axis during image loading by adding the following statement before loading any image: 
	unsigned int container2Texture = Texture::loadTexture(PATH_TEXTURE_CONTAINER2);
    unsigned int container2Specular = Texture::loadTexture(PATH_TEXTURE_CONTAINER2_SPECULAR);

    // Cubemap
	unsigned int skyboxTexture = Texture::loadCubemap(cubemapFaces);

    // VBO, VAO
    // ------------------------------------

    // FBO
	// ------------------------------------
    unsigned int fbo;
	glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // Attachment Color Buffer
    unsigned int textureFBO;
    glGenTextures(1, &textureFBO);
    glBindTexture(GL_TEXTURE_2D, textureFBO);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureFBO, 0);

	// Attaching both a depth and stencil buffer to the FBO with a Renderbuffer Object
    unsigned int rbo;
    glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, WINDOW_WIDTH, WINDOW_HEIGHT);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
		std::cout << "Failed to create Framebuffer" << std::endl;
        return -1;
	}
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Light
	// ------------------------------------

	setShaderLights(shader);
    // Models
	// ------------------------------------
    Model cubeModel(PATH_MODEL_CUBE);

    glm::vec3 cubePositions[] = {
        glm::vec3(6.0f,  3.0f,  7.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    // Render Loop
    // ------------------------------------
	lastFrame = static_cast<float>(glfwGetTime());

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    while (!glfwWindowShouldClose(window))
    {
		const float curFrame = static_cast<float>(glfwGetTime());
		deltaTime = curFrame - lastFrame;
		lastFrame = curFrame;
        // input
        processInput(window);

        // rendering commands

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glEnable(GL_FRAMEBUFFER_SRGB);

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        // matrixes
        glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = camera.getProjectionMatrix(WINDOW_WIDTH, WINDOW_HEIGHT);

        // Draw scene
		// ------------------------------------

		// Use shader program
        shader.use();
		shader.setMat4("view", value_ptr(view));
		shader.setMat4("projection", value_ptr(projection));
		shader.setVec3("viewPos", camera.GetPosition());
        shader.setVec3("spotLight.position", camera.GetPosition());
        shader.setVec3("spotLight.direction", glm::vec3(camera.GetFront()));
        // Material
        float shininessMat = 51.2f;
        shader.setInt("material.texture_diffuse0", 0);
        shader.setInt("material.texture_specular0", 1);
        shader.setFloat("material.shininess", shininessMat);

        // Draw cubes
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, container2Texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, container2Specular);
        for (unsigned int i = 0; i < 10; i++)
        {
            float angle = 20.0f * i + 20.0f * static_cast<float>(glfwGetTime());
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            shader.setMat4("model", value_ptr(model));
			cubeModel.draw(shader);
        }

		// Draw lightcubes
		lightCubeShader.use();
        lightCubeShader.setMat4("view", value_ptr(view));
		lightCubeShader.setMat4("projection", value_ptr(projection));
		lightCubeShader.setVec3("color", lightSpecular);
        for (int i = 0; i < sizeof(pointLightPositions) / sizeof(glm::vec3); i++)
        {
            glm::mat4 model(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, lightScale);
		    lightCubeShader.setMat4("model", value_ptr(model));

			cubeModel.draw(lightCubeShader);
        }

        // Reflective cube
		reflectiveShader.use();
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
		reflectiveShader.setMat4("model", value_ptr(model));
        reflectiveShader.setMat4("view", value_ptr(view));
        reflectiveShader.setMat4("projection", value_ptr(projection));
        reflectiveShader.setVec3("viewPos", camera.GetPosition());
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
		reflectiveShader.setInt("skybox", 0);
		cubeModel.draw(reflectiveShader);

        // skybox
        skyboxShader.use();
        glm::mat4 viewNoTranslation = glm::mat4(glm::mat3(view));
        skyboxShader.setMat4("view", value_ptr(viewNoTranslation));
        skyboxShader.setMat4("projection", value_ptr(projection));

		glDepthFunc(GL_LEQUAL);
        glCullFace(GL_FRONT);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        cubeModel.draw(skyboxShader);
        glDepthMask(GL_TRUE);
        glCullFace(GL_BACK);

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // CLEANUP
    // ------------------------------------
    glDeleteTextures(1, &container2Texture);
    glDeleteTextures(1, &container2Specular);
    glDeleteFramebuffers(1, &fbo);

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
        shader.setVec3(pointLightName + ".ambient", lightAmbient);
        shader.setVec3(pointLightName + ".diffuse", lightDiffuse);
        shader.setVec3(pointLightName + ".specular", lightSpecular);
        shader.setFloat(pointLightName + ".constant", 1.0f);
        shader.setFloat(pointLightName + ".linear", 0.09f);
        shader.setFloat(pointLightName + ".quadratic", 0.032f);
    }
}

Mesh createQuad()
{
    const std::vector<Vertex> verts = {
        Vertex(glm::vec3(-1.0f,  1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 1.0f)), // top left
        Vertex(glm::vec3(1.0f,  1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 1.0f)), // top right
        Vertex(glm::vec3(-1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(0.0f, 0.0f)), // bottom left
        Vertex(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec2(1.0f, 0.0f)) // bottom right
    };

    const std::vector<unsigned int> indices = {  // note that we start from 0!
        0, 2, 1,   // first triangle
        1, 2, 3    // second triangle
    };

    return Mesh(verts, indices, std::vector<Texture>());
}
