#include <iostream>

// todo : this is ugly
#define _WIN

// GLEW
#define GLEW_STATIC
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>
#include <assert.h>
#include <Shader.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <Mesh.h>

#include "minimalOpenGL.h"

#include "Camera.h"
#include "FileIO.h"

#define PI (3.1415927f)

using std::cout;
using std::endl;

struct TimeTracker {
	GLfloat deltaTime = 0.0f;
	GLfloat lastFrameTime = 0.0f;
} timeTracker;

struct InputTracker {
	bool keys[1024];
	GLfloat g_lastX = 400, g_lastY = 300;
	bool g_firstMouse = true;
} inputTracker;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

// Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void doMovement();

int main() {
    std::cout << "Starting GLFW context, OpenGL 3.3" << std::endl;

	GLFWwindow *window = nullptr;
	uint32_t windowWidth = 1280, windowHeight = 720;
    if (!GL::initWindow(windowWidth, windowHeight, window)) return -1;
    if (!GL::initGLEW()) return -1;
	GL::initViewport(window);

	/////////////////////////////////////////////////////////////////
	// Set the required callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	/////////////////////////////////////////////////////////////////
	// Load vertex array buffers
	Mesh mesh = GL::createTriangleMesh();

	glm::vec3 trianglePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
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

	/////////////////////////////////////////////////////////////////////
	// Create the main shader
	Shader shader((FileIO::getCurrentDirectory() + "/shaders/points.vs").c_str(), (FileIO::getCurrentDirectory() + "/shaders/points.fs").c_str());

	/////////////////////////////////////////////////////////////////////
	// OpenGL settings
    glClearColor(0.f, 0.f, 0.f, 1.0f);
    glEnable(GL_PROGRAM_POINT_SIZE);

	/////////////////////////////////////////////////////////////////////
	// Camera constants
	const float nearPlaneZ = -0.1f;
	const float farPlaneZ = -100.0f;
	const float verticalFieldOfView = 45.0f * PI / 180.0f;

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
		assert(glGetError() == GL_NONE);

		GLfloat currentFrameTime = (GLfloat)glfwGetTime();
		timeTracker.deltaTime = currentFrameTime - timeTracker.lastFrameTime;
		timeTracker.lastFrameTime = currentFrameTime;

		// Check if any events have been activated (key pressed, mouse moved etc.) and call corresponding response functions
		glfwPollEvents();
		doMovement();

		/////////////////////////////////////////////////////////////////////
		// Update transforms
		glm::mat4 projection = glm::perspective(verticalFieldOfView, (float)windowWidth / (float)windowHeight, -nearPlaneZ, -farPlaneZ);

		// Clear the colorbuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		const glm::mat4 view = camera.GetViewMatrix();

		shader.use();

		/////////////////////////////////////////////////////////////////////
		// Assign uniforms (View, projection)
		GLint viewLoc = glGetUniformLocation(shader.program, "view");
		GLint projectionLoc = glGetUniformLocation(shader.program, "projection");
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

		/////////////////////////////////////////////////////////////////////
		// Draw triangles
		glm::mat4 model;
		model = glm::rotate(glm::mat4(), glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		GLint modelLoc = glGetUniformLocation(shader.program, "model");
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

		mesh.Draw(shader);

        // Swap the screen buffers
        glfwSwapBuffers(window);
    }

    // Terminate GLFW, clearing any resources allocated by GLFW.
    glfwTerminate();

    return 0;
}

// Is called whenever a key is pressed/released via GLFW
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    std::cout << key << std::endl;
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    if (action == GLFW_PRESS)
		inputTracker.keys[key] = true;
    else if (action == GLFW_RELEASE)
		inputTracker.keys[key] = false;
}

void doMovement()
{
    if(inputTracker.keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, timeTracker.deltaTime);
    if(inputTracker.keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, timeTracker.deltaTime);
    if(inputTracker.keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, timeTracker.deltaTime);
    if(inputTracker.keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, timeTracker.deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if(inputTracker.g_firstMouse)
    {
		inputTracker.g_lastX = (GLfloat)xpos;
		inputTracker.g_lastY = (GLfloat)ypos;
		inputTracker.g_firstMouse = false;
    }

    GLfloat xoffset = (GLfloat)xpos - inputTracker.g_lastX;
    GLfloat yoffset = inputTracker.g_lastY - (GLfloat)ypos;
	inputTracker.g_lastX = (GLfloat)xpos;
	inputTracker.g_lastY = (GLfloat)ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll((GLfloat)yoffset);
}