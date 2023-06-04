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
void window_maximize_callback(GLFWwindow* window, int maximized);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

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
	uint chunkRenderDist = 5;
	float sensitivity = 5;
	bool maximized = true;

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
				else if (contents == "maximized = false")
					maximized = false;
				else if (contents.find("chunk render dist = ") == 0)
				{
					int result = std::stoi(contents.substr(20));
					if (result != -1)
						chunkRenderDist = result;
				}
				else if (contents.find("character = ") == 0)
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
			"\nmaximized = " + string(maximized ? "true" : "false") +
			"\nchunk render dist = " + to_string(chunkRenderDist) +
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
		settings.TryOpen(); // Used in window creation stuff.
		
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
		glfwSetWindowMaximizeCallback(window, window_maximize_callback);
		glfwSetKeyCallback(window, key_callback);

		if (settings.maximized)
			glfwMaximizeWindow(window);

		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CW);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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
		rightTriangle = Mesh({ -1.f, 0.0f,  0.0f, 1.f,  1.f, 0.0f }, { 0, 1, 2 });

		cube = Mesh({-1.f, -1.f, -1.f,  -1.f, -1.f, 1.f,  -1.f, 1.f, -1.f,  -1.f, 1.f, 1.f,
			1.f, -1.f, -1.f,  1.f, -1.f, 1.f,  1.f, 1.f, -1.f,  1.f, 1.f, 1.f}, {1, 3, 7, 1, 7, 5,
			0, 2, 3, 0, 3, 1,  0, 1, 5, 0, 5, 4,  2, 6, 7, 2, 7, 3,  6, 4, 5, 6, 5, 7,  0, 4, 6, 0, 6, 2},
			GL_TRIANGLES, GL_STATIC_DRAW, 3);

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

		if (cursorUnlockCount == 0)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

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
	float zoom = 30, minZoom = 10, maxZoom = 30, zoomSpeed = 5;
	uint fpsCount = 0;
	string name = "Martionatany";
	Inputs inputs;
	Vec3 screenOffset = Vec3(0);
	glm::mat4 camera = glm::mat4(1), cameraInv = glm::mat4(1), perspective = glm::mat4(1);
	int cursorUnlockCount = 1;

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

	bool AABBInCam(Vec3 minPos, Vec3 maxPos)
	{
		return true;
	}

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
		glUniform4f(glGetUniformLocation(circleShader, "posScale"), pos.x, pos.y, pos.z, radius);
		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(circleShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);

		for (int i = 0; i < cube.vertices.size(); i += 3)
			if ((camera * glm::vec4(pos + radius * Vec3(cube.vertices[i], cube.vertices[i + 1], cube.vertices[i + 2]), 1.f)).z < 0)
			{
				glCullFace(GL_FRONT);
				break;
			}

		cube.Draw();
		glCullFace(GL_BACK);
	}

	inline void DrawCone(Vec3 a, Vec3 b, RGBA color, float thicknessA, float thicknessB = 0)
	{
		glUseProgram(coneShader);
		glUniform3f(glGetUniformLocation(coneShader, "a"), a.x, a.y, a.z);
		glUniform3f(glGetUniformLocation(coneShader, "b"), b.x + 0.01f, b.y + 0.01f, b.z + 0.01f);
		glUniform1f(glGetUniformLocation(coneShader, "thickness"), max(thicknessA, thicknessB));
		glUniform1f(glGetUniformLocation(coneShader, "thicknessA"), thicknessA);
		glUniform1f(glGetUniformLocation(coneShader, "thicknessB"), thicknessB);

		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(coneShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
		glDisable(GL_CULL_FACE);
		cube.Draw();
		glEnable(GL_CULL_FACE);
	}

	inline void DrawString(string text, Vec2 pos, float scale, RGBA color, iVec2 pixelOffset = vZero) // In normal coordinates.
	{
		font.Render(text, pixelOffset + static_cast<iVec2>(Vec2((pos - Vec2(PlayerPos())) * 2.f) / Vec2(mainScreen->ScrDim()) * Vec2(ScrDim())), scale, color);
	}

	void DrawTextured(Texture& texture, uint spriteToDraw, Vec2 pivot, Vec2 pos, RGBA color, Vec2 dimensions)
	{
		glUseProgram(texturedShader);
		glUniform2f(glGetUniformLocation(texturedShader, "scale"),
			dimensions.x / screenRatio, dimensions.y);

		glUniform2f(glGetUniformLocation(texturedShader, "position"), pivot.x + pos.x / screenRatio, pivot.y + pos.y);

		float spriteWidth = 1.0f / texture.spriteCount;
		glUniform2f(glGetUniformLocation(texturedShader, "uvData"),
			spriteWidth, spriteWidth * spriteToDraw);
		texture.Activate(GL_TEXTURE0);
		glUniform1i(glGetUniformLocation(texturedShader, "tex"), 0);
		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(texturedShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
		quad.Draw();
	}

	inline void DrawCylinder(Vec3 a, Vec3 b, RGBA color, float thickness)
	{
		glUseProgram(cylinderShader);
		glUniform3f(glGetUniformLocation(cylinderShader, "a"), a.x, a.y, a.z);
		glUniform3f(glGetUniformLocation(cylinderShader, "b"), b.x + 0.01f, b.y + 0.01f, b.z + 0.01f);
		glUniform1f(glGetUniformLocation(cylinderShader, "thickness"), thickness);

		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(cylinderShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
		glDisable(GL_CULL_FACE);
		cube.Draw();
		glEnable(GL_CULL_FACE);
	}

	inline void DrawCapsule(Vec3 a, Vec3 b, RGBA color, float thickness)
	{
		glUseProgram(capsuleShader);
		glUniform3f(glGetUniformLocation(capsuleShader, "a"), a.x, a.y, a.z);
		glUniform3f(glGetUniformLocation(capsuleShader, "b"), b.x + 0.01f, b.y + 0.01f, b.z + 0.01f);
		glUniform1f(glGetUniformLocation(capsuleShader, "thickness"), thickness);

		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(capsuleShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
		glDisable(GL_CULL_FACE);
		cube.Draw();
		glEnable(GL_CULL_FACE);
	}

	void DrawLight(Vec3 pos, float range, JRGB color) // You have to set a lot of variables manually before calling this!!!
	{
		glUniform3f(glGetUniformLocation(shadowShader, "position"), pos.x, pos.y, pos.z);
		glUniform1f(glGetUniformLocation(shadowShader, "range"), range);

		glUniform3f(glGetUniformLocation(shadowShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);

		for (int i = 0; i < cube.vertices.size(); i += 3)
			if ((camera * glm::vec4(pos + range * Vec3(cube.vertices[i], cube.vertices[i + 1], cube.vertices[i + 2]), 1.f)).z < 0)
			{
				glCullFace(GL_FRONT);
				break;
			}

		cube.Draw();
		glCullFace(GL_BACK);
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

void window_maximize_callback(GLFWwindow* window, int maximized)
{
	((Renderer*)game.get())->settings.maximized = maximized;
	((Renderer*)game.get())->settings.Write();

}