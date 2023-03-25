#include "Framebuffers.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	trueScreenWidth = width;
	trueScreenHeight = height;
	screenRatio = (float)trueScreenWidth / (float)trueScreenHeight;
	for (Framebuffer* framebuffer : framebuffers)
		if (framebuffer->shouldScreenRes)
			framebuffer->ResetWidth();
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

#define START_SCR_WIDTH 990
#define START_SCR_HEIGHT 990

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
		trueScreenWidth = START_SCR_WIDTH;
		trueScreenHeight = START_SCR_HEIGHT;
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
		
		GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
		glfwSetCursor(window, cursor);


		for (std::tuple<std::pair<int, int>, uint*, string>& pairPair : shaders)
		{
			Resource vert(std::get<0>(pairPair).first, TEXT_FILE);
			char* vert2 = new char[vert.size + 2];
			for (int i = 0; i < vert.size; i++)
				vert2[i] = ((char*)vert.ptr)[i];
			vert2[vert.size] = '\0';
			Resource frag(std::get<0>(pairPair).second, TEXT_FILE);
			char* frag2 = new char[frag.size + 2];
			for (int i = 0; i < frag.size; i++)
				frag2[i] = ((char*)frag.ptr)[i];
			frag2[frag.size] = '\0';
			*(std::get<1>(pairPair)) = CreateShader(vert2, frag2, std::get<2>(pairPair));
		}

		for (std::tuple<Texture&, int, int, uint>& textureTuple : textures)
			std::get<0>(textureTuple) = Texture(std::get<1>(textureTuple), std::get<2>(textureTuple), std::get<3>(textureTuple));

		quad = Mesh({ 0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f }, {0, 1, 2, 0, 2, 3});
		screenSpaceQuad = Mesh({ -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f }, { 0, 1, 2, 0, 2, 3});
		line = Mesh({ 1.0f, 0.0f, 0.0f, 1.0f }, { 0, 1 }, GL_LINES);
		dot = Mesh({ 0.0f, 0.0f }, { 0 }, GL_POINTS);
		rightTriangle = Mesh({ -0.5f, 0.0f,  0.0f, 0.5f,  0.5f, 0.0f }, { 0, 1, 2 });

		mainScreen = make_unique<DeferredFramebuffer>(trueScreenHeight, GL_RGB, true);
		shadowMap = make_unique<Framebuffer>(trueScreenHeight, GL_RGB16F, true);
		framebuffers = { mainScreen.get(), shadowMap.get() };

		Resource defaultFont = Resource(PIXELOID_SANS, FONT_FILE);
		font = Font(static_cast<FT_Byte*>(defaultFont.ptr), static_cast<FT_Long>(defaultFont.size), 128);

		currentFramebuffer = MAINSCREEN;
		UseFramebuffer();

		glStencilMask(0x00);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

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

		Update();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	void TEnd()
	{
		End();
		for (std::tuple<std::pair<int, int>, uint*, string>& pairPair : shaders)
			glDeleteProgram(*(std::get<1>(pairPair)));
		for (Framebuffer* framebuffer : framebuffers)
			framebuffer->Destroy();
		for (std::tuple<Texture&, int, int, uint>& textureTuple : textures)
			std::get<0>(textureTuple).Destroy();
		for (Mesh* mesh : meshes)
			mesh->Destroy();
		glfwTerminate();
	}

public:
	GLFWwindow* window = nullptr;
	float lastTime = 0.0f, dTime = 0.0f;
	bool shouldRun = true;
	float zoom = 30, minZoom = 10, maxZoom = 40, zoomSpeed = 5;
	uint fpsCount = 0;
	string name = "Martionatany";
	Inputs inputs;
	Vec2 screenOffset = vZero;

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
				dimensions.x * 2 / ScrWidth(), dimensions.y * 2 / ScrHeight());

			glUniform2f(glGetUniformLocation(defaultShader, "position"),
				pos.x / ScrWidth(),
				pos.y / ScrHeight());
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
		DrawFBL(pos - dimensions / 2.f, color, dimensions);
	}

	inline void DrawCircle(Vec2 pos, RGBA color, float radius = 1)
	{
		glUseProgram(circleShader);
		radius /= zoom;
		glUniform2f(glGetUniformLocation(circleShader, "scale"), radius / screenRatio, radius);

		pos -= PlayerPos() + screenOffset;
		pos.x /= screenRatio;
		pos /= zoom;
		glUniform2f(glGetUniformLocation(circleShader, "position"), pos.x, pos.y);
		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(circleShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
		screenSpaceQuad.Draw();
	}

	inline void DrawRightTri(Vec2 pos, Vec2 scale, float rotation, RGBA color)
	{
		glUseProgram(triangleShader);
		glUniform1f(glGetUniformLocation(triangleShader, "rotation"), rotation);

		scale /= zoom;
		glUniform2f(glGetUniformLocation(triangleShader, "scale"), scale.x / screenRatio, scale.y);

		pos -= PlayerPos() + screenOffset;
		pos.x /= screenRatio;
		pos /= zoom;
		glUniform2f(glGetUniformLocation(triangleShader, "position"), pos.x, pos.y);
		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(triangleShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
		rightTriangle.Draw();
	}

	inline void DrawString(string text, Vec2 pos, float scale, RGBA color, iVec2 pixelOffset = vZero) // In normal coordinates.
	{
		font.Render(text, pixelOffset + static_cast<iVec2>(Vec2((pos - PlayerPos()) * 2.f) / Vec2(mainScreen->ScrDim()) * Vec2(ScrDim())), scale, color);
	}

	void DrawTextured(Texture& texture, uint spriteToDraw, iVec2 pos, RGBA color, iVec2 dimensions = vOne)
	{
		glUseProgram(texturedShader);
		if (currentFramebuffer == 0) // We're rendering to the big television in the sky.
		{
			glUniform2f(glGetUniformLocation(texturedShader, "scale"),
				float(dimensions.x * 2) / ScrWidth(), float(dimensions.y * 2) / ScrHeight());

			glUniform2f(glGetUniformLocation(texturedShader, "position"),
				float(pos.x) / ScrWidth(),
				float(pos.y) / ScrHeight());
		}
		else
		{
			// The * 2s are there as the screen goes from -1 to 1 instead of 0 to 1.
			// The "/ ScrWidth() or ScrHeight()" are to put it in pixel dimensions.
			glUniform2f(glGetUniformLocation(texturedShader, "scale"),
				float(dimensions.x * 2) / ScrWidth(), float(dimensions.y * 2) / ScrHeight());

			glUniform2f(glGetUniformLocation(texturedShader, "position"),
				float((pos.x - PlayerPos().x) * 2) / ScrWidth(),
				float((pos.y - PlayerPos().y) * 2) / ScrHeight());
		}

		float spriteWidth = 1.0f / texture.spriteCount;
		glUniform2f(glGetUniformLocation(texturedShader, "uvData"),
			spriteWidth, spriteWidth * spriteToDraw);
		texture.Activate(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(texturedShader, "tex"), 0);
		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(texturedShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
		quad.Draw();
	}

	void DrawLine(Vec2 a, Vec2 b, RGBA color)
	{
		glUseProgram(lineShader);
		a -= PlayerPos() + screenOffset;
		a.x /= screenRatio;
		a /= zoom;

		b -= PlayerPos() + screenOffset;
		b.x /= screenRatio;
		b /= zoom;
		glUniform2f(glGetUniformLocation(lineShader, "a"), a.x, a.y);
		glUniform2f(glGetUniformLocation(lineShader, "b"), b.x, b.y);

		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(lineShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
		line.Draw();
	}

	inline void DrawLineThick(Vec2 a, Vec2 b, RGBA color, float thickness)
	{
		glLineWidth(thickness / zoom * trueScreenHeight);
		DrawLine(a, b, color);
	}

	void DrawLight(Vec2 pos, float range, JRGB color) // You have to set a lot of variables manually before calling this!!!
	{
		glUniform1f(glGetUniformLocation(shadowShader, "range"), range);

		Vec2 scrPos = pos - PlayerPos() - screenOffset;
		glUniform2f(glGetUniformLocation(shadowShader, "scale"),
			range / (zoom * 0.5f * screenRatio), range / (zoom * 0.5f));

		glUniform2f(glGetUniformLocation(shadowShader, "position"),
			(scrPos.x - range) / (zoom * screenRatio),
			(scrPos.y - range) / zoom);

		glUniform2f(glGetUniformLocation(shadowShader, "center"), scrPos.x, scrPos.y);

		glUniform2f(glGetUniformLocation(shadowShader, "bottomLeft"),
			scrPos.x - range, scrPos.y - range);

		glUniform2f(glGetUniformLocation(shadowShader, "topRight"),
			scrPos.x + range, scrPos.y + range);

		glUniform3f(glGetUniformLocation(shadowShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);

		quad.Draw();
	}

	void DrawFramebufferOnto(uint newFramebuffer)
	{
		glUseProgram(framebufferShader);
		glBindTexture(GL_TEXTURE_2D, framebuffers[static_cast<size_t>(currentFramebuffer) - 1]->textureColorbuffer);	// use the color attachment texture as the texture of the quad plane
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

	bool IsFullscreen()
	{
		return glfwGetWindowMonitor(window) != nullptr;
	}

	void Fullscreen()
	{
		if (IsFullscreen())
		{
			glfwSetWindowMonitor(window, NULL, 100, 100, START_SCR_WIDTH, START_SCR_HEIGHT, 0);
			return;
		}
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
	}

	float DistToCorner()
	{
		return sqrtf(maxZoom * maxZoom * (screenRatio * screenRatio + 1));
	}
};

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	((Renderer*)game.get())->inputs.mouseScroll += static_cast<int>(yoffset);
}