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

enum DIFFICULTY
{
	EASY, MEDIUM, HARD
};

enum class CHARS
{
	SOLDIER, FLICKER, ENGINEER, ORCHID, COUNT
};

string settingsLocation = "Settings.txt";
class Settings
{
public:
	bool colorBand = true, vSync = true;
	DIFFICULTY difficulty = DIFFICULTY::MEDIUM;
	CHARS character = CHARS::SOLDIER;

	void TryOpen()
	{
		std::ifstream file;
		file.open(settingsLocation, std::ios::in);
		if (file.is_open())
		{
			while (file)
			{
				string contents;
				std::getline(file, contents);
				std::cout << contents << '\n';
				if (contents == "color band = false")
					colorBand = false;
				else if (contents == "vSync = false")
					vSync = false;
				else if (contents == "difficulty = easy")
					difficulty = DIFFICULTY::EASY;
				else if (contents == "difficulty = hard")
					difficulty = DIFFICULTY::HARD;
				else
					for (int i = 0; i < UnEnum(CHARS::COUNT); i++)
						if (contents == "character = " + to_string(i))
							character = CHARS(i);
			}
			file.close();
		}
		else Write();
	}

	void Write()
	{
		string contents = "color band = " + string(colorBand ? "true" : "false") + "\nvSync = " + string(vSync ? "true" : "false") +
			"\ndifficulty = " + string(difficulty == DIFFICULTY::EASY ? "easy" : difficulty == DIFFICULTY::MEDIUM ? "medium" : "hard") +
			"\ncharacter = " + to_string(UnEnum(character));
		std::ofstream file;
		file.open(settingsLocation, std::ios::out | std::ios::trunc);
		file << contents;
		file.close();
	}
};

class Renderer
{
private:
	// In the following T stands for true, these are the under-the-hood calls, mainly for rendering.
	bool TStart()
	{
#pragma region Very eary stuff
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
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
		glEnable(GL_DEPTH_TEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
#pragma endregion
		
		settings.TryOpen();

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
		rightTriangle = Mesh({ -1.f, 0.0f,  0.0f, 1.f,  1.f, 0.0f }, { 0, 1, 2 });

		cube = Mesh({-1.f, -1.f, -1.f,  -1.f, -1.f, 1.f,  -1.f, 1.f, -1.f,  -1.f, 1.f, 1.f,
			1.f, -1.f, -1.f,  1.f, -1.f, 1.f,  1.f, 1.f, -1.f,  1.f, 1.f, 1.f}, {1, 3, 7, 1, 7, 5,
			0, 1, 3, 0, 3, 2,  0, 1, 5, 0, 5, 4,  2, 3, 7, 2, 7, 6,  6, 7, 5, 6, 5, 4}, GL_TRIANGLES, GL_STATIC_DRAW, 3);

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
		glfwSwapInterval(int(settings.vSync));
		dTime = static_cast<float>(glfwGetTime() - lastTime);
		fpsCount++;
		if (int(lastTime) != int(glfwGetTime()))
		{
			glfwSetWindowTitle(window, (name + " FPS = " + to_string(fpsCount)).c_str());
			fpsCount = 0;
		}
		lastTime = static_cast<float>(glfwGetTime());

		perspective = glm::perspective(glm::radians(90.f), screenRatio, 0.1f, 100.f);

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
	Settings settings;
	float zoom = 30, minZoom = 10, maxZoom = 40, zoomSpeed = 5;
	uint fpsCount = 0;
	string name = "Martionatany";
	Inputs inputs;
	Vec3 screenOffset = Vec3(0);
	glm::mat4 perspective = glm::mat4(1);

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

	inline void DrawCircle(Vec3 pos, RGBA color, float radius = 1)
	{
		glUseProgram(circleShader);
		glUniformMatrix4fv(glGetUniformLocation(circleShader, "perspective"), 1, GL_FALSE, glm::value_ptr(perspective));
		pos -= PlayerPos() + screenOffset;
		glUniform4f(glGetUniformLocation(circleShader, "posScale"), pos.x, pos.y, pos.z, radius);

		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(circleShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
		cube.Draw();
	}

	inline void DrawRightTri(Vec3 pos, Vec2 scale, float rotation, RGBA color)
	{
		glUseProgram(triangleShader);
		glUniformMatrix4fv(glGetUniformLocation(triangleShader, "perspective"), 1, GL_FALSE, glm::value_ptr(perspective));

		glUniform1f(glGetUniformLocation(triangleShader, "rotation"), rotation);
		glUniform2f(glGetUniformLocation(triangleShader, "scale"), scale.x, scale.y);

		pos -= PlayerPos() + screenOffset;
		glUniform3f(glGetUniformLocation(triangleShader, "position"), pos.x, pos.y, pos.z);
		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(triangleShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
		rightTriangle.Draw();
	}

	inline void DrawString(string text, Vec2 pos, float scale, RGBA color, iVec2 pixelOffset = vZero) // In normal coordinates.
	{
		font.Render(text, pixelOffset + static_cast<iVec2>(Vec2((pos - Vec2(PlayerPos())) * 2.f) / Vec2(mainScreen->ScrDim()) * Vec2(ScrDim())), scale, color);
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

	inline void DrawLine(Vec3 a, Vec3 b, RGBA color, float thickness)
	{
		glUseProgram(lineShader);
		glUniformMatrix4fv(glGetUniformLocation(lineShader, "perspective"), 1, GL_FALSE, glm::value_ptr(perspective));

		a -= PlayerPos() + screenOffset;
		b -= PlayerPos() + screenOffset;
		glUniform3f(glGetUniformLocation(lineShader, "a"), a.x, a.y, a.z);
		glUniform3f(glGetUniformLocation(lineShader, "b"), b.x, b.y, b.z);
		glUniform1f(glGetUniformLocation(lineShader, "thickness"), thickness);

		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(lineShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
		quad.Draw();
	}

	void DrawLight(Vec3 pos, float range, JRGB color) // You have to set a lot of variables manually before calling this!!!
	{
		pos -= PlayerPos() + screenOffset;

		glUniform3f(glGetUniformLocation(shadowShader, "position"), pos.x, pos.y, pos.z);
		glUniform1f(glGetUniformLocation(shadowShader, "range"), range);

		glUniform3f(glGetUniformLocation(shadowShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);

		screenSpaceQuad.Draw();
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

	inline Vec3 PlayerPos(); // The position of the player in normal coordinates.

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

	float CurrentDistToCorner()
	{
		return sqrtf(zoom * zoom * (screenRatio * screenRatio + 1));
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