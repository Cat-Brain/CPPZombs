#include "Framebuffers.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	trueScreenWidth = width;
	trueScreenHeight = height;
	printf("?");
	screenRatio = (float)trueScreenWidth / (float)trueScreenHeight;
	for (Framebuffer* framebuffer : framebuffers)
		framebuffer->ResetWidth(int(ceilf(framebuffer->height * screenRatio)));
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
		trueScreenWidth = 800;
		trueScreenHeight = 600;
		screenRatio = (float)trueScreenWidth / (float)trueScreenHeight;
		window = glfwCreateWindow(trueScreenWidth, trueScreenHeight, "Martionatany", NULL, NULL);
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

		glViewport(0, 0, trueScreenWidth, trueScreenHeight);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
#pragma endregion
		

#pragma region Shader stuff
		defaultShader = CreateShader(defaultVert, defaultFrag);
		framebufferShader = CreateShader(framebufferVert, framebufferFrag);
#pragma endregion

		quad = Mesh({ 0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f }, {0, 1, 2, 0, 2, 3});
		screenSpaceQuad = Mesh({ -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f }, { 0, 1, 2, 0, 2, 3 });

		lowRes = Framebuffer(5);
		midRes = Framebuffer(lowRes.height * 2);
		highRes = Framebuffer(midRes.height * 2);

		Start();
		return true;
	}

	void TUpdate()
	{
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		inputs.Update1(window);
		UseFramebuffer(framebuffers[int(glfwGetTime()) % framebuffers.size()]);
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		glUseProgram(defaultShader);
		glUniform2f(glGetUniformLocation(defaultShader, "scale"), 2, 2);
		glUniform2f(glGetUniformLocation(defaultShader, "position"), -1.0f, -1.0f);
		quad.Draw();
		Update();
		// now bind back to default framebuffer and draw a quad plane with the attached framebuffer color texture
		UseFramebuffer(nullptr);

		glUseProgram(framebufferShader);
		glBindTexture(GL_TEXTURE_2D, framebuffers[int(glfwGetTime()) % framebuffers.size()]->textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
		glUniform1i(glGetUniformLocation(framebufferShader, "screenTexture"), int(glfwGetTime()) % framebuffers.size());

		screenSpaceQuad.Draw();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	void TEnd()
	{
		End();
		for (std::pair<std::pair<const char*, const char*>, uint*>& pairPair : shaders)
			glDeleteProgram(*pairPair.second);
		for (Mesh* mesh : meshes)
			mesh->Destroy();
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