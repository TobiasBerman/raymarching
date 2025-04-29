#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "shader.hpp"

using namespace std;

float deltaTime = 0.0f; // Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

// For mouse movement
float lastX = 1920.0 / 2.0;
float lastY = 1080.0 / 2.0;
float yaw = 0.0;
float pitch = 0.0;
bool firstMouse = true;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    
    lastX = width / 2.0f;
    lastY = height / 2.0f;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
      
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    
    yaw -= xoffset * 0.001;
    pitch -= yoffset * 0.001;
}

void processInput(GLFWwindow *window, Shader &shader, float &xPos, float &yPos, float &zPos) {
    float speed = 0.6;
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        zPos += speed*deltaTime * cos(yaw);
        xPos += speed*deltaTime * -sin(yaw);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        zPos -= speed*deltaTime * cos(yaw);
        xPos -= speed*deltaTime * -sin(yaw);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        zPos -= speed*deltaTime * sin(yaw);
        xPos -= speed*deltaTime * cos(yaw);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        zPos += speed*deltaTime * sin(yaw);
        xPos += speed*deltaTime * cos(yaw);
    }
    if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        yPos += speed*deltaTime;
    if(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        yPos -= speed*deltaTime;
}

int main()
{
    // Initializes GLFW
    if (!glfwInit())
        return -1;
    
    // Tells GLFW we want to use OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
    // Tells GLFW we want to use the core-profile, and makes it forward but not backward compatible
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    
    // Creates the window and makes its context (state) current
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "Test Window", nullptr, nullptr);
    if (!window)
    {
        cout << "Failed to create GLFW window" << endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    
    // Synchronizes FPS to VSYNC (monitor's refresh rate)
    glfwSwapInterval(1);
    
    // Initializes GLEW
    if (glewInit() != GLEW_OK)
    {
        cout << "Failed to initialize GLEW" << endl;
        return -1;
    }
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // disables cursor
    
    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    
    // Tells OpenGL to render onto this size screen
    glViewport(0, 0, framebufferWidth, framebufferHeight);
    
    Shader myShader("shader.vs", "shader.fs");
    
    float square[] = {
        -1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f
    };
    
    // Create buffer and bind it to GL_ARRAY_BUFFER (the buffer type of a vertex buffer object (VBO)) spot
    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(square), square, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0); // Set only for vertices, not indices
    glEnableVertexAttribArray(0);
    
    float xPos = 0.0f;
    float yPos = 0.0f;
    float zPos = -3.0f;
    // Creates render loop (1 iteration = 1 frame)
    while(!glfwWindowShouldClose(window))
    {
        // Delta time update
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        processInput(window, myShader, xPos, yPos, zPos);
        
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        myShader.use();
        
        int framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
        myShader.setFloat("xSize", framebufferWidth);
        myShader.setFloat("ySize", framebufferHeight);
        myShader.setVec3("camPos", xPos, yPos, zPos);
        myShader.setFloat("yaw", yaw);
        myShader.setFloat("pitch", pitch);
        myShader.setFloat("time", glfwGetTime());
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        // Swaps back and front buffers, so final image for this frame is displayed all at once, once it is all drawn
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    
    glfwTerminate();
    return 0;
}
