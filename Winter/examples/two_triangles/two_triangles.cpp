// Getting Started: Hello Triangle
// https://learnopengl.com/Getting-started/Hello-Triangle
#include <iostream>
#include <string>
#include <vector>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

int main()
{
    // DATA
    // ------------------------------------
    const std::string vertexShaderSource = "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
        "}\0";
    const char* vertexShaderSourceCStr = vertexShaderSource.c_str();

    const std::string fragmentShaderSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
        "}\0";
    const char* fragmentShaderSourceCStr = fragmentShaderSource.c_str();
    const std::string fragmentShaderRecolorSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);\n"
        "}\0";
    const char* fragmentShaderRecolorSourceCStr = fragmentShaderRecolorSource.c_str();

    const int32_t WINDOW_WIDTH = 800;
    const int32_t WINDOW_HEIGHT = 600;
    const std::string WINDOW_TITLE = "LearnOpenGL";

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

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE.c_str(), NULL, NULL);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Init GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);


    // SHADERS
    // ------------------------------------
    int  success;
    char infoLog[512];
    // Vertex Shader
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSourceCStr, nullptr);
    glCompileShader(vertexShader);

    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Fragment Shader
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSourceCStr, nullptr);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int fragmentShaderRecolor;
    fragmentShaderRecolor = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShaderRecolor, 1, &fragmentShaderRecolorSourceCStr, nullptr);
    glCompileShader(fragmentShaderRecolor);

    glGetShaderiv(fragmentShaderRecolor, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderRecolor, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // Shader program object
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    // Shader program object
    unsigned int shaderProgramRecolor;
    shaderProgramRecolor = glCreateProgram();
    glAttachShader(shaderProgramRecolor, vertexShader);
    glAttachShader(shaderProgramRecolor, fragmentShaderRecolor);
    glLinkProgram(shaderProgramRecolor);
    glGetProgramiv(shaderProgramRecolor, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgramRecolor, sizeof(infoLog), nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderPrograms[2] = { shaderProgram, shaderProgramRecolor };

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    glDeleteShader(fragmentShaderRecolor);

    // VBO, VAO
    // ------------------------------------
    constexpr unsigned int nbVAOs = 2;
    constexpr float triangle1Vertices[] = {
        0.5f,  0.5f, 0.0f,  // top right
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f,  0.5f, 0.0f   // top left
    };
    constexpr float triangle2Vertices[] = {
        -0.5f,  0.5f, 0.0f,   // top left 
        0.5f, -0.5f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f  // bottom left
    };
    std::vector<const float*> trianglesVertices = { triangle1Vertices, triangle2Vertices };
    std::vector<unsigned int> trianglesVerticesSizes = { sizeof(triangle1Vertices), sizeof(triangle2Vertices) };
    unsigned int VAOs[nbVAOs];
    unsigned int VBOs[nbVAOs];
    glGenVertexArrays(nbVAOs, VAOs);
    glGenBuffers(nbVAOs, VBOs);

    for (int i = 0; i < nbVAOs; i++)
    {
        glBindVertexArray(VAOs[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBOs[i]);
        auto verts = trianglesVertices[i];
        auto vertsSize = trianglesVerticesSizes[i];
        glBufferData(GL_ARRAY_BUFFER, vertsSize, verts, GL_STATIC_DRAW);
        int attribLocation = 0;
        glVertexAttribPointer(attribLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(attribLocation);
    }

    // Render Loop
    // ------------------------------------
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // rendering commands
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw VAO (object)
        for (int i = 0; i < nbVAOs; i++)
        {
            glUseProgram(shaderPrograms[i]);
            glBindVertexArray(VAOs[i]);
            unsigned int nbTriangles = trianglesVerticesSizes[i] / 3 / sizeof(float);
            glDrawArrays(GL_TRIANGLES, 0, nbTriangles);
        }

        // check and call events and swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // CLEANUP
    // ------------------------------------
    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
    glDeleteProgram(shaderProgram);
    glDeleteProgram(shaderProgramRecolor);
    glfwTerminate();

    return 0;
}