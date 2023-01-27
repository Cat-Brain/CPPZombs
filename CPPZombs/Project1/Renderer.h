#include "Shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

class Renderer
{
private:
	Inputs inputs;
	// In the following T stands for true, these are the under-the-hood calls, mainly for rendering.
	bool TStart()
	{
#pragma region Very eary stuff
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		window = glfwCreateWindow(screenWidth * 10, screenHeight * 10, "LearnOpenGL", NULL, NULL);
		if (window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return false;
		}
		glfwMakeContextCurrent(window);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			std::cout << "Failed to initialize GLAD" << std::endl;
			return -1;
		}

		glViewport(0, 0, screenWidth * 10, screenHeight * 10);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
#pragma endregion
		

#pragma region Shader stuff
		defaultShader = CreateShader(defaultVert, defaultFrag);
#pragma endregion

		quad = Mesh({ {-0.5f, -0.5f}, {-0.5f, 0.5f}, {0.5f, 0.5f}, {0.5f, -0.5f} }, {0, 1, 2, 0, 2, 3});

		Start();
		return true;
	}

	void TUpdate()
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		inputs.Update1(window);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(defaultShader);
		glUniform2f(glGetUniformLocation(defaultShader, "scale"), 1, 1);
		glUniform2f(glGetUniformLocation(defaultShader, "position"), -1, -1);
		quad.Draw();
		Update();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	void TEnd()
	{
		End();
		glDeleteProgram(defaultShader);
		glfwTerminate();
	}

public:
	GLFWwindow* window = nullptr;
	float lastTime = 0.0f, dTime = 0.0f;
	bool shouldRun = true;

	Renderer() { }

	void Construct()
	{
		if (!TStart())
			return;
		while (!glfwWindowShouldClose(window))
			TUpdate();
		TEnd();
	}

	virtual void Start() { }
	virtual void Update() { }
	virtual void End() { }
};