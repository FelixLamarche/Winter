// Advanced Lighting: Parallax Mapping
// https://learnopengl.com/Advanced-Lighting/Parallax-Mapping
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
    glm::vec3( 2.5f,  3.0f,  4.0f),
    glm::vec3( 2.5f,  3.0f, -4.0f),
    glm::vec3(-2.5f,  3.0f,  4.0f),
    glm::vec3(-2.5f,  3.0f, -4.0f)
};

// MISC

void mouseCallback(GLFWwindow* window, double xPos, double yPos);
void scrollCallback(GLFWwindow* window, double xOffset, double yOffset);
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

Mesh createQuad();
void setShaderLights(Shader& shader);
void renderQuad();

int main()
{
    // Init Path
    PathManager::projectPath = std::filesystem::current_path().string() + "/";

    // DATA
    // ------------------------------------
    const int32_t WINDOW_WIDTH = 800;
    const int32_t WINDOW_HEIGHT = 600;
    const std::string WINDOW_TITLE = "LearnOpenGL";

    const std::string PATH_EXAMPLE = PathManager::getProjectPath() + "examples/parallax_normal_map/";

	const std::string PATH_VERTEX_SHADER = PATH_EXAMPLE + "basic.vert";
    const std::string PATH_FRAGMENT_SHADER = PATH_EXAMPLE + "basic.frag";
	const std::string PATH_LIGHT_CUBE_VERTEX_SHADER = PATH_EXAMPLE + "lightCube.vert";
	const std::string PATH_LIGHT_CUBE_FRAGMENT_SHADER = PATH_EXAMPLE + "lightCube.frag";
    const std::string PATH_SCREEN_VERTEX_SHADER = PATH_EXAMPLE + "screen.vert";
    const std::string PATH_SCREEN_FRAGMENT_SHADER = PATH_EXAMPLE + "screen.frag";
	const std::string PATH_PARALLAX_MAP_VERTEX_SHADER = PATH_EXAMPLE + "parallaxMap.vert";
	const std::string PATH_PARALLAX_MAP_FRAGMENT_SHADER = PATH_EXAMPLE + "parallaxMap.frag";

    const std::string PATH_TEXTURE_CONTAINER = PathManager::getTexturesPath() + "container.jpg";
    const std::string PATH_TEXTURE_CONTAINER2 = PathManager::getTexturesPath() + "container2.png";
    const std::string PATH_TEXTURE_CONTAINER2_SPECULAR = PathManager::getTexturesPath() + "container2_specular.png";
	const std::string PATH_TEXTURE_WOOD = PathManager::getTexturesPath() + "wood.png";
	const std::string PATH_TEXTURE_BRICK_WALL = PathManager::getTexturesPath() + "brickwall.jpg";
	const std::string PATH_TEXTURE_BRICK_WALL_NORMAL = PathManager::getTexturesPath() + "brickwall_normal.jpg";
    const std::string PATH_TEXTURE_BRICK_WALL2 = PathManager::getTexturesPath() + "bricks2.jpg";
    const std::string PATH_TEXTURE_BRICK_WALL2_NORMAL = PathManager::getTexturesPath() + "bricks2_normal.jpg";
    const std::string PATH_TEXTURE_BRICK_WALL2_DISPL = PathManager::getTexturesPath() + "bricks2_disp.jpg";
	const std::string PATH_TEXTURE_TOY_BOX = PathManager::getTexturesPath() + "toy_box_diffuse.png";
	const std::string PATH_TEXTURE_TOY_BOX_DISPL = PathManager::getTexturesPath() + "toy_box_disp.png";
	const std::string PATH_TEXTURE_TOY_BOX_NORMAL = PathManager::getTexturesPath() + "toy_box_normal.png";

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
    const float cameraNearPlane = 0.05f;
    const float cameraFarPlane = 250.0f;

	Camera camera(cameraPos, cameraFront, cameraUp, cameraRollYawPitch, cameraFOV, cameraNearPlane, cameraFarPlane);
	pCamera = &camera;

    // SHADERS
    // ------------------------------------
    Shader shader(PATH_VERTEX_SHADER, PATH_FRAGMENT_SHADER);
    Shader screenShader(PATH_SCREEN_VERTEX_SHADER, PATH_SCREEN_FRAGMENT_SHADER);
	Shader lightCubeShader(PATH_LIGHT_CUBE_VERTEX_SHADER, PATH_LIGHT_CUBE_FRAGMENT_SHADER);
	Shader parallaxMapShader(PATH_PARALLAX_MAP_VERTEX_SHADER, PATH_PARALLAX_MAP_FRAGMENT_SHADER);

    // TEXTURES
	// ------------------------------------

    // OpenGL expects the 0.0 coordinate on the y-axis to be on the bottom side of the image, 
    // but images usually have 0.0 at the top of the y-axis. 
    // stb_image.h can flip the y-axis during image loading by adding the following statement before loading any image: 
	unsigned int containerTexture = Texture::loadTexture(PATH_TEXTURE_CONTAINER);
	unsigned int container2Texture = Texture::loadTexture(PATH_TEXTURE_CONTAINER2);
    unsigned int container2Specular = Texture::loadTexture(PATH_TEXTURE_CONTAINER2_SPECULAR, false);
	unsigned int woodTexture = Texture::loadTexture(PATH_TEXTURE_WOOD);
	unsigned int woodTextureSpec = Texture::loadTexture(PATH_TEXTURE_WOOD, false);
    unsigned int brickWallTexture = Texture::loadTexture(PATH_TEXTURE_BRICK_WALL);
    unsigned int brickWallTextureSpec = Texture::loadTexture(PATH_TEXTURE_BRICK_WALL, false);
	unsigned int brickWallNormalTexture = Texture::loadTexture(PATH_TEXTURE_BRICK_WALL_NORMAL, false);
	unsigned int bricks2Texture = Texture::loadTexture(PATH_TEXTURE_BRICK_WALL2);
	unsigned int bricks2NormalTexture = Texture::loadTexture(PATH_TEXTURE_BRICK_WALL2_NORMAL, false);
	unsigned int bricks2DisplTexture = Texture::loadTexture(PATH_TEXTURE_BRICK_WALL2_DISPL, false);
	unsigned int toyBoxTexture = Texture::loadTexture(PATH_TEXTURE_TOY_BOX);
	unsigned int toyBoxDisplTexture = Texture::loadTexture(PATH_TEXTURE_TOY_BOX_DISPL, false);
	unsigned int toyBoxNormalTexture = Texture::loadTexture(PATH_TEXTURE_TOY_BOX_NORMAL, false);

    // FrameBuffer

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

    // Light
	// ------------------------------------
	setShaderLights(shader);

    // Models and Meshes
	// ------------------------------------
    Mesh quad = createQuad();

	Mesh woodQuad = quad;
	woodQuad.AddTexture(Texture(woodTexture, Texture::DIFFUSE_TYPENAME, PATH_TEXTURE_WOOD));
	woodQuad.AddTexture(Texture(woodTextureSpec, Texture::SPECULAR_TYPENAME, PATH_TEXTURE_WOOD));

    Mesh brickWallQuad = createQuad();
    brickWallQuad.AddTexture(Texture(brickWallTexture, Texture::DIFFUSE_TYPENAME, PATH_TEXTURE_BRICK_WALL));
    brickWallQuad.AddTexture(Texture(brickWallTextureSpec, Texture::SPECULAR_TYPENAME, PATH_TEXTURE_BRICK_WALL));
	brickWallQuad.AddTexture(Texture(brickWallNormalTexture, Texture::NORMAL_TYPENAME, PATH_TEXTURE_BRICK_WALL_NORMAL));

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
		glm::mat4 model = glm::mat4(1.0f);
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
		pointLightPositions[0] = glm::vec3(2.5f * cos(curFrameTime), 2.5f * sin(curFrameTime), 2.0f);
		shader.setVec3("pointLights[0].position", pointLightPositions[0]);

		shader.setVec2("texScale", glm::vec2(1.0f));
        shader.setVec3("viewPos", camera.GetPosition());
        shader.setVec3("spotLight.position", camera.GetPosition());
        shader.setVec3("spotLight.direction", glm::vec3(camera.GetFront()));
        float shininessMat = 32.0f;
        shader.setFloat("material.shininess", shininessMat);

        // Render Floor
        float floorScale = 8.0f;
        shader.use();
		model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -1.0f, 0.0f));
		model = glm::rotate(model, glm::radians(90.0f), glm::vec3(-1.0f, 0.0f, 0.0f));
		model = glm::scale(model, glm::vec3(floorScale));
		shader.setMat4("model", value_ptr(model));
        shader.setVec2("texScale", glm::vec2(floorScale));
        shader.setFloat("material.shininess", 16.0f);
		woodQuad.draw(shader);

        // Render light cubes
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

        // Render parallax mapped bricks
		parallaxMapShader.use();
		parallaxMapShader.setMat4("projection", value_ptr(projection));
		parallaxMapShader.setMat4("view", value_ptr(view));
        model = glm::mat4(1.0f);
		parallaxMapShader.setMat4("model", value_ptr(model));

		parallaxMapShader.setVec3("viewPos", viewPos);
		parallaxMapShader.setVec3("lightPos", pointLightPositions[0]);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, bricks2Texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, bricks2NormalTexture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, bricks2DisplTexture);
		parallaxMapShader.setInt("diffuseMap", 0);
		parallaxMapShader.setInt("normalMap", 1);
		parallaxMapShader.setInt("depthMap", 2);
		parallaxMapShader.setFloat("heightScale", 0.15f);

		renderQuad();


        // Render toy box
        parallaxMapShader.use();
        parallaxMapShader.setMat4("projection", value_ptr(projection));
        parallaxMapShader.setMat4("view", value_ptr(view));
        model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        parallaxMapShader.setMat4("model", value_ptr(model));

        parallaxMapShader.setVec3("viewPos", viewPos);
        parallaxMapShader.setVec3("lightPos", pointLightPositions[0]);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, toyBoxTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, toyBoxNormalTexture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, toyBoxDisplTexture);
        parallaxMapShader.setInt("diffuseMap", 0);
        parallaxMapShader.setInt("normalMap", 1);
        parallaxMapShader.setInt("depthMap", 2);
        parallaxMapShader.setFloat("heightScale", 0.2f);

        renderQuad();

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        lastFrameTime = curFrameTime;
    }

    // CLEANUP
    // ------------------------------------
    glDeleteTextures(1, &containerTexture);
    glDeleteTextures(1, &container2Texture);
    glDeleteTextures(1, &container2Specular);
	glDeleteTextures(1, &woodTexture);
	glDeleteTextures(1, &woodTextureSpec);

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