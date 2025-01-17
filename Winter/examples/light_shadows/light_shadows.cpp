// Advanced Lighting: Shadows
// https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping
// https://learnopengl.com/Advanced-Lighting/Shadows/Point-Shadows
#include <array>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <numeric>
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
glm::vec3 lightSpecular(1.0f, 1.0f, 1.0f);

glm::vec3 lightScale(0.33f);
glm::vec3 pointLightPositions[] = {
    glm::vec3( 1.5f,  1.0f,  2.0f),
    glm::vec3( 200.5f,  3.0f, -4.0f),
    glm::vec3(-200.5f,  3.0f,  4.0f),
    glm::vec3(-200.5f,  3.0f, -4.0f)
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
	
    const std::string PATH_EXAMPLE = PathManager::getProjectPath() + "examples/light_shadows/";

    const std::string PATH_VERTEX_SHADER = PATH_EXAMPLE + "basic.vert";
    const std::string PATH_FRAGMENT_SHADER = PATH_EXAMPLE + "basic.frag";
    const std::string PATH_LIGHT_VERTEX_SHADER = PATH_EXAMPLE + "lightCube.vert";
    const std::string PATH_LIGHT_FRAGMENT_SHADER = PATH_EXAMPLE + "lightCube.frag";
    const std::string PATH_SCREEN_VERTEX_SHADER = PATH_EXAMPLE + "screen.vert";
    const std::string PATH_SCREEN_FRAGMENT_SHADER = PATH_EXAMPLE + "screen.frag";
	const std::string PATH_SIMPLE_DEPTH_VERTEX_SHADER = PATH_EXAMPLE + "simpleDepth.vert";
	const std::string PATH_EMPTY_FRAGMENT_SHADER = PATH_EXAMPLE + "empty.frag";
	const std::string PATH_CUBEMAP_SHADOW_VERTEX_SHADER = PATH_EXAMPLE + "cubemapShadow.vert";
	const std::string PATH_CUBEMAP_SHADOW_FRAGMENT_SHADER = PATH_EXAMPLE + "cubemapShadow.frag";
	const std::string PATH_CUBEMAP_SHADOW_GEOMETRY_SHADER = PATH_EXAMPLE + "cubemapShadow.geom";

    const std::string PATH_TEXTURE_CONTAINER2 = PathManager::getTexturesPath() + "container2.png";
    const std::string PATH_TEXTURE_CONTAINER2_SPECULAR = PathManager::getTexturesPath() + "container2_specular.png";
	const std::string PATH_TEXTURE_WOOD = PathManager::getTexturesPath() + "wood.png";

	const std::string PATH_MODEL_CUBE = PathManager::getModelsPath() + "cube/cube.obj";

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
    const float cameraNearPlane = 0.1f;
    const float cameraFarPlane = 250.0f;

	Camera camera(cameraPos, cameraFront, cameraUp, cameraRollYawPitch, cameraFOV, cameraNearPlane, cameraFarPlane);
	pCamera = &camera;

    // SHADERS
    // ------------------------------------
    Shader shader(PATH_VERTEX_SHADER, PATH_FRAGMENT_SHADER);
    Shader lightCubeShader(PATH_LIGHT_VERTEX_SHADER, PATH_LIGHT_FRAGMENT_SHADER);
    Shader screenShader(PATH_SCREEN_VERTEX_SHADER, PATH_SCREEN_FRAGMENT_SHADER);

	Shader simpleDepthShader = Shader(PATH_SIMPLE_DEPTH_VERTEX_SHADER, PATH_EMPTY_FRAGMENT_SHADER);
	Shader cubemapShadowShader = Shader(PATH_CUBEMAP_SHADOW_VERTEX_SHADER, 
        PATH_CUBEMAP_SHADOW_FRAGMENT_SHADER, PATH_CUBEMAP_SHADOW_GEOMETRY_SHADER);

	screenShader.use();
	screenShader.setVec3("color", glm::vec3(1.0f, 0.66f, 0.0f));

    // TEXTURES
	// ------------------------------------

    // OpenGL expects the 0.0 coordinate on the y-axis to be on the bottom side of the image, 
    // but images usually have 0.0 at the top of the y-axis. 
    // stb_image.h can flip the y-axis during image loading by adding the following statement before loading any image: 
	unsigned int container2Texture = Texture::loadTexture(PATH_TEXTURE_CONTAINER2);
    unsigned int container2Specular = Texture::loadTexture(PATH_TEXTURE_CONTAINER2_SPECULAR, false);
	unsigned int woodTexture = Texture::loadTexture(PATH_TEXTURE_WOOD);
	unsigned int woodTextureSpec = Texture::loadTexture(PATH_TEXTURE_WOOD, false);


    // FrameBuffer

    unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);

	const unsigned int SHADOW_WIDTH = 1024;
	const unsigned int SHADOW_HEIGHT = 1024;

	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(glm::vec3(1.0f)));

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    unsigned int depthCubeMapFBO;
    glGenFramebuffers(1, &depthCubeMapFBO);

    unsigned int depthCubemap;
	glGenTextures(1, &depthCubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
    for (unsigned int i = 0; i < 6; ++i)
    {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    }
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubemap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Uniform Buffers
	// ------------------------------------
    unsigned int uboMatrices;
    glGenBuffers(1, &uboMatrices);
    glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(glm::mat4));

	unsigned int uboIndexBasic = glGetUniformBlockIndex(shader.getID(), "Matrices");
	glUniformBlockBinding(shader.getID(), uboIndexBasic, 0);
    unsigned int uboIndexLight = glGetUniformBlockIndex(lightCubeShader.getID(), "Matrices");
    glUniformBlockBinding(lightCubeShader.getID(), uboIndexLight, 0);

    // Light
	// ------------------------------------
	setShaderLights(shader);

    // Models and Meshes
	// ------------------------------------
    Mesh quad = createQuad();

	Mesh woodQuad = quad;
	woodQuad.AddTexture(Texture(woodTexture, Texture::DIFFUSE_TYPENAME, PATH_TEXTURE_WOOD));
	woodQuad.AddTexture(Texture(woodTextureSpec, Texture::SPECULAR_TYPENAME, PATH_TEXTURE_WOOD));

    Model cubeModel(PATH_MODEL_CUBE);

    for (auto& mesh : cubeModel.meshes)
    {
        mesh.AddTexture(Texture(container2Texture, Texture::DIFFUSE_TYPENAME, PATH_TEXTURE_CONTAINER2));
        mesh.AddTexture(Texture(container2Specular, Texture::SPECULAR_TYPENAME, PATH_TEXTURE_CONTAINER2_SPECULAR));
    }

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
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glEnable(GL_MULTISAMPLE);
        glEnable(GL_FRAMEBUFFER_SRGB);

        // matrixes
        glm::mat4 view = camera.getViewMatrix();
		glm::mat4 projection = camera.getProjectionMatrix(WINDOW_WIDTH, WINDOW_HEIGHT);
        glm::vec3 viewPos = camera.GetPosition();

        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Draw scene
		// ------------------------------------
		// Use shader program
        shader.use();
		shader.setVec2("texScale", glm::vec2(1.0f));
        shader.setVec3("viewPos", camera.GetPosition());
        shader.setVec3("spotLight.position", camera.GetPosition());
        shader.setVec3("spotLight.direction", glm::vec3(camera.GetFront()));
        float shininessMat = 32.0f;
        shader.setFloat("material.shininess", shininessMat);

		// Render to FBO for shadow mapping
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

        // To prevent peter panning (this makes sure that shadows are not 'detached' from the objects)
        glCullFace(GL_FRONT);
        // Set projection matrix for directional light POV
        float nearPlane = 1.0f;
        float farPlane = 7.5f;
		glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);
        glm::mat4 lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
		glm::mat4 lightMatrix = lightProjection * lightView;
        // RENDER
        simpleDepthShader.use();
		simpleDepthShader.setMat4("lightSpaceMatrix", value_ptr(lightMatrix));

        glm::mat4 model(1.0f);
        simpleDepthShader.use();
        simpleDepthShader.setMat4("model", value_ptr(model));
        cubeModel.draw(simpleDepthShader);

        model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, -0.5f, 0.0f));
        simpleDepthShader.use();
        simpleDepthShader.setMat4("model", value_ptr(model));
        cubeModel.draw(simpleDepthShader);

        simpleDepthShader.use();
        simpleDepthShader.setVec3("color", lightSpecular);
        for (unsigned int i = 0; i < sizeof(pointLightPositions) / sizeof(glm::vec3); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, lightScale);
            simpleDepthShader.setMat4("model", value_ptr(model));
            cubeModel.draw(simpleDepthShader);
        }

		// Reset culling as quads only have a front face
        glCullFace(GL_BACK);
        // Floor
        float floorScale = 8.0f;
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(floorScale));
        simpleDepthShader.setMat4("model", value_ptr(model));
        simpleDepthShader.setVec2("texScale", glm::vec2(floorScale));
        simpleDepthShader.setFloat("material.shininess", 16.0f);
        woodQuad.draw(simpleDepthShader);

        glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render to FBO cubemap shadows for point lights
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthCubeMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);

        // To prevent peter panning (this makes sure that shadows are not 'detached' from the objects)
        glCullFace(GL_FRONT);
		float aspectPoint = static_cast<float>(SHADOW_WIDTH) / static_cast<float>(SHADOW_HEIGHT);
		float nearPoint = 1.0f;
		float farPoint = 25.0f;
		glm::mat4 shadowProj = glm::perspective(glm::radians(90.0f), aspectPoint, nearPoint, farPoint);
		std::vector<glm::mat4> shadowTransforms;
        shadowTransforms.resize(6);
		shadowTransforms[0] = shadowProj * glm::lookAt(pointLightPositions[0], 
            pointLightPositions[0] + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		shadowTransforms[1] = shadowProj * glm::lookAt(pointLightPositions[0],
			pointLightPositions[0] + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		shadowTransforms[2] = shadowProj * glm::lookAt(pointLightPositions[0],
			pointLightPositions[0] + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		shadowTransforms[3] = shadowProj * glm::lookAt(pointLightPositions[0],
			pointLightPositions[0] + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
		shadowTransforms[4] = shadowProj * glm::lookAt(pointLightPositions[0],
			pointLightPositions[0] + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, -1.0f, 0.0f));
		shadowTransforms[5] = shadowProj * glm::lookAt(pointLightPositions[0],
			pointLightPositions[0] + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, -1.0f, 0.0f));

        // RENDER
        cubemapShadowShader.use();
		cubemapShadowShader.setVec3("lightPos", pointLightPositions[0]);
		cubemapShadowShader.setFloat("far_plane", farPoint);
		cubemapShadowShader.setMat4("shadowMatrices[0]", value_ptr(shadowTransforms[0]));
		cubemapShadowShader.setMat4("shadowMatrices[1]", value_ptr(shadowTransforms[1]));
        cubemapShadowShader.setMat4("shadowMatrices[2]", value_ptr(shadowTransforms[2]));
        cubemapShadowShader.setMat4("shadowMatrices[3]", value_ptr(shadowTransforms[3]));
        cubemapShadowShader.setMat4("shadowMatrices[4]", value_ptr(shadowTransforms[4]));
        cubemapShadowShader.setMat4("shadowMatrices[5]", value_ptr(shadowTransforms[5]));

        model = glm::mat4(1.0f);
        cubemapShadowShader.use();
        cubemapShadowShader.setMat4("model", value_ptr(model));
        cubeModel.draw(cubemapShadowShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, -0.5f, 0.0f));
        cubemapShadowShader.use();
        cubemapShadowShader.setMat4("model", value_ptr(model));
        cubeModel.draw(cubemapShadowShader);

        cubemapShadowShader.use();
        cubemapShadowShader.setVec3("color", lightSpecular);
        for (unsigned int i = 0; i < sizeof(pointLightPositions) / sizeof(glm::vec3); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, lightScale);
            cubemapShadowShader.setMat4("model", value_ptr(model));
            cubeModel.draw(cubemapShadowShader);
        }

        // Reset culling as quads only have a front face
        glCullFace(GL_BACK);
        // Floor
        floorScale = 8.0f;
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(floorScale));
        cubemapShadowShader.setMat4("model", value_ptr(model));
        cubemapShadowShader.setVec2("texScale", glm::vec2(floorScale));
        cubemapShadowShader.setFloat("material.shininess", 16.0f);
        woodQuad.draw(cubemapShadowShader);

        glCullFace(GL_BACK);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Render scene
		glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

        shader.use();
		shader.setMat4("lightSpaceMatrix", value_ptr(lightMatrix));
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, depthMap);
        shader.setInt("shadowMap", 2);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubemap);
		shader.setInt("depthCubemap", 3);
		shader.setFloat("far_planeCube", farPoint);

        model = glm::mat4(1.0f);
        shader.setMat4("model", value_ptr(model));
        cubeModel.draw(shader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(2.0f, -0.5f, 0.0f));
        shader.setMat4("model", value_ptr(model));
        cubeModel.draw(shader);

        // Floor
        floorScale = 8.0f;
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(floorScale));
		shader.setMat4("model", value_ptr(model));
        shader.setVec2("texScale", glm::vec2(floorScale));
        shader.setFloat("material.shininess", 16.0f);
		woodQuad.draw(shader);

		lightCubeShader.use();
		lightCubeShader.setVec3("color", lightSpecular);
		for (unsigned int i = 0; i < sizeof(pointLightPositions) / sizeof(glm::vec3); i++)
		{
			model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, lightScale);
            lightCubeShader.setMat4("model", value_ptr(model));
			cubeModel.draw(lightCubeShader);
		}

        // Render to screen
        screenShader.use();
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);
		screenShader.setInt("screenTexture", 0);
        //quad.draw(screenShader);

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        lastFrameTime = curFrameTime;
    }

    // CLEANUP
    // ------------------------------------
    glDeleteTextures(1, &container2Texture);
    glDeleteTextures(1, &container2Specular);

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
        shader.setFloat(pointLightName + ".linear", 0.02f);
        shader.setFloat(pointLightName + ".quadratic", 0.002f);
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

