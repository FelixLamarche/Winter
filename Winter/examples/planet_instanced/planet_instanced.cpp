// Advanced OpenGL: Instancing
// https://learnopengl.com/Advanced-OpenGL/Instancing
// Quicker version of the planet visualization with instancing
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

	const std::string PATH_EXAMPLE = PathManager::getProjectPath() + "examples/planet_instanced/";

	const std::string PATH_VERTEX_SHADER = PATH_EXAMPLE + "basicVertex.glsl";
    const std::string PATH_FRAGMENT_SHADER = PATH_EXAMPLE + "basicFragment.glsl";
    const std::string PATH_UNLIT_VERTEX_SHADER = PATH_EXAMPLE + "unlitVertex.glsl";
    const std::string PATH_UNLIT_INSTANCED_VERTEX_SHADER = PATH_EXAMPLE + "unlitInstancedVertex.glsl";
    const std::string PATH_UNLIT_FRAGMENT_SHADER = PATH_EXAMPLE + "unlitFragment.glsl";

	const std::string PATH_MODEL_PLANET = PathManager::getModelsPath() + "planet/planet.obj";
	const std::string PATH_MODEL_ROCK = PathManager::getModelsPath() + "rock/rock.obj";

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
    const glm::vec3 cameraPos = glm::vec3(0.0f, 5.0f, 30.0f);
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
    Shader unlitShader(PATH_UNLIT_VERTEX_SHADER, PATH_UNLIT_FRAGMENT_SHADER);
	Shader instancedUnlitShader(PATH_UNLIT_INSTANCED_VERTEX_SHADER, PATH_UNLIT_FRAGMENT_SHADER);

    // TEXTURES
	// ------------------------------------

    // OpenGL expects the 0.0 coordinate on the y-axis to be on the bottom side of the image, 
    // but images usually have 0.0 at the top of the y-axis. 
    // stb_image.h can flip the y-axis during image loading by adding the following statement before loading any image: 

    // VAO
    // ------------------------------------
    float points[] = {
        -0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // top-left
         0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // top-right
         0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // bottom-right
        -0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // bottom-left
    };

    unsigned int VAOpoints, VBOpoints;
    glGenVertexArrays(1, &VAOpoints);
    glGenBuffers(1, &VBOpoints);
	glBindVertexArray(VAOpoints);
	glBindBuffer(GL_ARRAY_BUFFER, VBOpoints);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) 0);
	glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*) (2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

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
	unsigned int uboIndexUnlit = glGetUniformBlockIndex(unlitShader.getID(), "Matrices");
	glUniformBlockBinding(unlitShader.getID(), uboIndexUnlit, 0);
    unsigned int uboIndexInstanced = glGetUniformBlockIndex(instancedUnlitShader.getID(), "Matrices");
    glUniformBlockBinding(instancedUnlitShader.getID(), uboIndexInstanced, 0);

    // Light
	// ------------------------------------
	setShaderLights(shader);

    // Models and Meshes
	// ------------------------------------
    Model planetModel(PATH_MODEL_PLANET);
	Model rockModel(PATH_MODEL_ROCK);

    unsigned int amountRocks = 150000;
    glm::mat4* modelMatrices = new glm::mat4[amountRocks];

    srand(0); // initialize random seed	
    float radius = 200.0;
    float offset = 25.0f;
    for (unsigned int i = 0; i < amountRocks; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)amountRocks * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float y = displacement * 0.4f; // keep height of field smaller compared to width of x and z
        displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, y, z));

        // 2. scale: scale between 0.05 and 0.25f
        float scale = static_cast<float>((rand() % 20)) / 100.0f + 0.05f;
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = static_cast<float>(rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.4f, 0.6f, 0.8f));

        // 4. now add to list of matrices
        modelMatrices[i] = model;
    }

    // VBO
    unsigned int VBORocks;
    glGenBuffers(1, &VBORocks);
    glBindBuffer(GL_ARRAY_BUFFER, VBORocks);
	glBufferData(GL_ARRAY_BUFFER, amountRocks * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    for (unsigned int i = 0; i < rockModel.meshes.size(); i++)
    {
		unsigned int VAO = rockModel.meshes[i].VAO;
		glBindVertexArray(VAO);
		long long vec4Size = static_cast<long long>(sizeof(glm::vec4));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)0);
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(1 * vec4Size));
		glEnableVertexAttribArray(5);
		glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(2 * vec4Size));
		glEnableVertexAttribArray(6);
		glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * vec4Size, (void*)(3 * vec4Size));
		glVertexAttribDivisor(3, 1);
		glVertexAttribDivisor(4, 1);
		glVertexAttribDivisor(5, 1);
		glVertexAttribDivisor(6, 1);
    }
	glBindVertexArray(0);


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

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
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

        glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
        glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(projection));
		glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(view));
        glBindBuffer(GL_UNIFORM_BUFFER, 0);

        // Draw scene
		// ------------------------------------

		// Use shader program

        instancedUnlitShader.use();
		for (const auto& mesh: rockModel.meshes)
		{
			glBindVertexArray(mesh.VAO);
			glDrawElementsInstanced(GL_TRIANGLES, static_cast<GLsizei>(mesh.indices.size()), GL_UNSIGNED_INT, 0, amountRocks);
		}

        glm::mat4 model(1.0f);
		model = glm::scale(model, glm::vec3(2.0f));
        unlitShader.use();
        unlitShader.setMat4("model", value_ptr(model));
        planetModel.draw(unlitShader);

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

        lastFrameTime = curFrameTime;
    }

    // CLEANUP
    // ------------------------------------
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
