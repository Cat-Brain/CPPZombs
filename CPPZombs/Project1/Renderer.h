#include "Framebuffers.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	trueScreenWidth = width;
	trueScreenHeight = height;
	screenRatio = (float)trueScreenWidth / (float)trueScreenHeight;
	for (Framebuffer* framebuffer : framebuffers)
		framebuffer->ResetWidth();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

class Renderer
{
private:
	// In the following T stands for true, these are the under-the-hood calls, mainly for rendering.
	bool TStart()
	{
#pragma region Very eary stuff
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		trueScreenWidth = 990;
		trueScreenHeight = 990;
		screenRatio = (float)trueScreenWidth / (float)trueScreenHeight;
		window = glfwCreateWindow(trueScreenWidth, trueScreenHeight, name.c_str(), NULL, NULL);
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
			return false;
		}

		glViewport(0, 0, trueScreenWidth, trueScreenHeight);
		glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
		glfwSetScrollCallback(window, scroll_callback);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#pragma endregion
		

		for (std::pair<std::pair<const char*, const char*>, uint*>& pairPair : shaders)
			*pairPair.second = CreateShader(pairPair.first.first, pairPair.first.second);

		quad = Mesh({ 0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f }, {0, 1, 2, 0, 2, 3});
		screenSpaceQuad = Mesh({ -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f }, { 0, 1, 2, 0, 2, 3});
		line = Mesh({ 1.0f, 0.0f, 0.0f, 1.0f }, { 0, 1 }, GL_LINES);
		dot = Mesh({ 0.0f, 0.0f }, { 0 }, GL_POINTS);

		midRes = Framebuffer(80);
		subScat = Framebuffer(MAP_WIDTH_TRUE, MAP_WIDTH_TRUE);
		shadowMap = Framebuffer(MAP_WIDTH_TRUE, MAP_WIDTH_TRUE);
		currentFramebuffer = 1;
		Resource defaultFont = Resource(PIXELOID_SANS, FONT_FILE);
		font = Font(static_cast<FT_Byte*>(defaultFont.ptr), defaultFont.size, 128);
		UseFramebuffer();

		Start();
		return true;
	}

	void TUpdate()
	{
		glfwSwapInterval(1);
		dTime = static_cast<float>(glfwGetTime() - lastTime);
		fpsCount++;
		if (int(lastTime) != int(glfwGetTime()))
		{
			glfwSetWindowTitle(window, (name + " FPS = " + to_string(fpsCount)).c_str());
			fpsCount = 0;
		}
		lastTime = static_cast<float>(glfwGetTime());
		tTime += dTime;

		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		inputs.Update(window);
		Update();
		// Prepare current framebuffer to be used in rendering of the frame.
		currentFramebuffer = 1;
		UseFramebuffer();

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
	uint fpsCount = 0;
	string name = "Martionatany";
	Inputs inputs;

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

	// FBL stands for From Bottom Left.
	void DrawFBL(Vec2 pos, RGBA color, Vec2 dimensions = vOne)
	{
		glUseProgram(defaultShader);
		if (currentFramebuffer == 0) // We're rendering to the big television in the sky.
		{
			glUniform2f(glGetUniformLocation(defaultShader, "scale"),
				float(dimensions.x * 2) / ScrWidth(), float(dimensions.y * 2) / ScrHeight());

			glUniform2f(glGetUniformLocation(defaultShader, "position"),
				float(pos.x) / ScrWidth(),
				float(pos.y) / ScrHeight());
		}
		else
		{
			// The * 2s are there as the screen goes from -1 to 1 instead of 0 to 1.
			// The "/ ScrWidth() or ScrHeight()" are to put it in pixel dimensions.
			glUniform2f(glGetUniformLocation(defaultShader, "scale"),
				float(dimensions.x * 2) / ScrWidth(), float(dimensions.y * 2) / ScrHeight());

			glUniform2f(glGetUniformLocation(defaultShader, "position"),
				float((pos.x - PlayerPos().x) * 2) / ScrWidth(),
				float((pos.y - PlayerPos().y) * 2) / ScrHeight());
		}

		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(defaultShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
		quad.Draw();
	}

	inline void Draw(Vec2 pos, RGBA color, Vec2 dimensions = vOne)
	{
		DrawFBL(pos - dimensions / 2, color, dimensions);
	}

	inline void DrawString(string text, Vec2 pos, float scale, RGBA color, Vec2 pixelOffset = vZero) // In normal coordinates.
	{
		font.Render(text, pixelOffset + Vec2f((pos - PlayerPos()) * 2) / midRes.TScrDim() * ScrDim(), scale, color);
	}

	void DrawLine(Vec2f a, Vec2f b, RGBA color)
	{
		glUseProgram(lineShader);
		// The * 2s are there as the screen goes from -1 to 1 instead of 0 to 1.
		// The "/ ScrWidth() or ScrHeight()" are to put it in pixel dimensions.
		glUniform2f(glGetUniformLocation(lineShader, "a"),
			(a.x - PlayerPos().x) * 2 / ScrWidth(),
			(a.y - PlayerPos().y) * 2 / ScrHeight());
		glUniform2f(glGetUniformLocation(lineShader, "b"),
			(b.x - PlayerPos().x) * 2 / ScrWidth(),
			(b.y - PlayerPos().y) * 2 / ScrHeight());

		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(lineShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
		line.Draw();
	}

	void DrawFramebufferOnto(uint newFramebuffer)
	{
		glUseProgram(framebufferShader);
		glBindTexture(GL_TEXTURE_2D, framebuffers[currentFramebuffer - 1]->textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
		glUniform1i(glGetUniformLocation(framebufferShader, "screenTexture"), currentFramebuffer - 1);
		glUniform1f(glGetUniformLocation(framebufferShader, "currentScrRat"), (float)ScrWidth() / (float)ScrHeight());

		currentFramebuffer = newFramebuffer;
		UseFramebuffer();
		if (newFramebuffer != 0)
			glUniform1f(glGetUniformLocation(framebufferShader, "newScrRat"), (float)ScrWidth() / (float)ScrHeight());
		else
			glUniform1f(glGetUniformLocation(framebufferShader, "newScrRat"), screenRatio);

		screenSpaceQuad.Draw();
		glUseProgram(defaultShader);
	}

	inline virtual Vec2 PlayerPos() // The position of the player in normal coordinates.
	{
		return vZero;
	}
};

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	((Renderer*)game.get())->inputs.mouseScroll += yoffset;
}