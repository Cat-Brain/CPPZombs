#include "Framebuffers.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, width, height);
	trueScreenWidth = width;
	trueScreenHeight = height;
	screenRatio = (float)trueScreenWidth / (float)trueScreenHeight;
	for (Framebuffer* framebuffer : framebuffers)
		if (framebuffer->shouldScreenRes)
			framebuffer->ResetDim();
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
string difficultyStrs[] = { "Easy", "Medium", "Hard" };

enum class CHARS
{
	SOLDIER, FLICKER, ENGINEER, ORCHID, COUNT
};

// Having an enum of all seeds will come in handy:
enum class SEEDINDICES
{
	COPPER, IRON, RUBY, EMERALD, ROCK, SHADE, BOWLER, VACUUMIUM, SILVER, QUARTZ_S, COAL, BRICK, CHEESE, TOPAZ, SAPPHIRE, LEAD, QUARTZ_V, COUNT
};

string settingsLocation = "Settings.txt";
class Settings
{
public:
	DIFFICULTY difficulty = DIFFICULTY::EASY;
	CHARS character = CHARS::SOLDIER;
	uint chunkRenderDist = 3;
	float sensitivity = 300;
	bool startSeeds[UnEnum(SEEDINDICES::COUNT)] = { false };
	bool colorBand = true, vSync = true, displayFPS = true, maximized = true,
		spawnAllTypesInTier = true, canChangeRow = false; // <- to be removed probably
	vector<std::pair<bool*, string>> dispBoolSettings = { {&colorBand, "color band"}, {&vSync, "vSync"},
		{&displayFPS, "display FPS"},
		{&spawnAllTypesInTier, "spawn all types in tier"}, {&canChangeRow, "can change row"} };
	vector<std::pair<bool*, string>> hidBoolSettings = { {&maximized, "maximized"} };

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
				for (int i = 0; i < difficultyStrs->size(); i++)
					if (contents == "difficulty = " + difficultyStrs[i])
					{
						difficulty = (DIFFICULTY)i;
						continue;
					}
				else if (contents.find("chunk render dist = ") == 0)
				{
					int result = std::stoi(contents.substr(20));
					if (result != -1)
						chunkRenderDist = result;
				}
				else if (contents.find("start seeds = ") == 0)
				{
					string subStr = contents.substr(14);
					int i = 0;
					for (char c : subStr)
					{
						if (c == 't' && i < UnEnum(SEEDINDICES::COUNT))
							startSeeds[i] = true;
						i++;
					}

				}
				else if (contents.find("character = ") == 0)
				{
					for (int i = 0; i < UnEnum(CHARS::COUNT); i++)
						if (contents == "character = " + to_string(i))
							character = CHARS(i);
				}
				for (std::pair<bool*, string> boolSetting : dispBoolSettings)
				{
					if (contents == boolSetting.second + " = false")
					{
						*boolSetting.first = false;
						continue;
					}
					else if (contents == boolSetting.second + " = true")
					{
						*boolSetting.first = true;
						continue;
					}
				}
				for (std::pair<bool*, string> boolSetting : hidBoolSettings)
				{
					if (contents == boolSetting.second + " = false")
					{
						*boolSetting.first = false;
						continue;
					}
					else if (contents == boolSetting.second + " = true")
					{
						*boolSetting.first = true;
						continue;
					}
				}
			}
			file.close();
		}
		else Write();
	}

	void Write()
	{
		string startSeedsStr = "\nstart seeds = ";
		for (bool b : startSeeds)
			startSeedsStr += b ? "t" : "f";

		string contents =
			"\ndifficulty = " + difficultyStrs[difficulty] +
			"\nchunk render dist = " + to_string(chunkRenderDist) +
			startSeedsStr +
			"\ncharacter = " + to_string(UnEnum(character));
		for (std::pair<bool*, string> boolSetting : dispBoolSettings)
			contents += '\n' + boolSetting.second + " = " + ToStringBool(*boolSetting.first);
		for (std::pair<bool*, string> boolSetting : hidBoolSettings)
			contents += '\n' + boolSetting.second + " = " + ToStringBool(*boolSetting.first);
		std::ofstream file;
		file.open(settingsLocation, std::ios::out | std::ios::trunc);
		file << contents;
		file.close();
	}
};

struct Plane
{
	// unit vector
	Vec3 normal = up;
	// distance from origin to the nearest point in the plane
	float distance = 0.f;

	Plane(Vec3 pos = vZero, Vec3 normal = up) :
		normal(normal), distance(glm::dot(pos, normal)) { }
	
	float SignedDistance(const Vec3& point)
	{
		return glm::dot(normal, point) - distance;
	}

	bool InPlane(const Vec3& point)
	{
		return SignedDistance(point) > 0;
	}

	bool AABBOverlaps(Vec3 pos, Vec3 halfDim)
	{
		const float r = halfDim.x * std::abs(normal.x) +
			halfDim.y * std::abs(normal.y) + halfDim.z * std::abs(normal.z);

		return -r <= SignedDistance(pos);
	}
};

struct Frustum
{
	Plane leftFace;
	Plane rightFace;

	Plane topFace;
	Plane bottomFace;

	Plane nearFace;
	Plane farFace;

	bool BoxInFrustum(Vec3 boxPos, Vec3 boxHalfDim)
	{
		return leftFace.AABBOverlaps(boxPos, boxHalfDim) &&
			rightFace.AABBOverlaps(boxPos, boxHalfDim) &&
			topFace.AABBOverlaps(boxPos, boxHalfDim) &&
			bottomFace.AABBOverlaps(boxPos, boxHalfDim) &&
			nearFace.AABBOverlaps(boxPos, boxHalfDim) &&
			farFace.AABBOverlaps(boxPos, boxHalfDim);
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

		uint textureType = std::get<0>(textures);
		for (std::tuple<Texture&, int, int>& textureTuple : std::get<1>(textures))
			std::get<0>(textureTuple) = Texture(std::get<1>(textureTuple), textureType, std::get<2>(textureTuple));

		quad = Mesh({ 0.0f, 0.0f,  0.0f, 1.0f,  1.0f, 1.0f,  1.0f, 0.0f }, {0, 1, 2, 0, 2, 3});
		screenSpaceQuad = Mesh({ -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f }, { 0, 1, 2, 0, 2, 3});
		line = Mesh({ 1.0f, 0.0f, 0.0f, 1.0f }, { 0, 1 }, GL_LINES);
		dot = Mesh({ 0.0f, 0.0f }, { 0 }, GL_POINTS);
		rightTriangle = Mesh({ -1.f, 0.0f,  0.0f, 1.f,  1.f, 0.0f }, { 0, 1, 2 });

		cube = Mesh({-1.f, -1.f, -1.f,  -1.f, -1.f, 1.f,  -1.f, 1.f, -1.f,  -1.f, 1.f, 1.f,
			1.f, -1.f, -1.f,  1.f, -1.f, 1.f,  1.f, 1.f, -1.f,  1.f, 1.f, 1.f}, {1, 3, 7, 1, 7, 5,
			0, 2, 3, 0, 3, 1,  0, 1, 5, 0, 5, 4,  2, 6, 7, 2, 7, 3,  6, 4, 5, 6, 5, 7,  0, 4, 6, 0, 6, 2},
			GL_TRIANGLES, GL_STATIC_DRAW, 3);

		glGenBuffers(1, &instanceVBO);
		glBindVertexArray(cube.vao);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO); // this attribute comes from a different vertex buffer

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(4 * sizeof(float)));
		glVertexAttribDivisor(3, 1); // tell OpenGL this is an instanced vertex attribute.

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
			glfwSetWindowTitle(window, (name + (settings.displayFPS ? " FPS = " + to_string(fpsCount) : "")).c_str());
			fpsCount = 0;
		}
		lastTime = static_cast<float>(glfwGetTime());

		glfwSetInputMode(window, GLFW_CURSOR, cursorUnlockCount == 0 ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

		Update();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	void TEnd()
	{
		while (!End());
		for (std::tuple<std::pair<int, int>, uint*, string>& pairPair : shaders)
			glDeleteProgram(*(std::get<1>(pairPair)));
		for (Framebuffer* framebuffer : framebuffers)
			framebuffer->Destroy();
		for (std::tuple<Texture&, int, int>& textureTuple : std::get<1>(textures))
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
	uint fpsCount = 0;
	string name = "Martionatany";
	Inputs inputs;
	Vec3 screenOffset = vZero;
	float fov = glm::radians(90.f), nearDist = 0.1f, farDist = 100;
	Vec3 camPos = vZero, camForward = vZero, camRight = vZero, camUp = vZero;
	glm::mat4 camera = glm::mat4(1), cameraInv = glm::mat4(1), camRot = glm::mat4(1), perspective = glm::mat4(1);
	int cursorUnlockCount = 1, lastCursorUnlockCount = 1;
	Frustum camFrustum;

	vector<std::pair<glm::vec4, glm::vec4>> toDrawCircles;
	uint instanceVBO = 0;

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
	virtual bool End() { return true; }
	
	void CalcFrustum()
	{
		float halfVSide = farDist * tanf(fov * 0.5f);
		float halfHSide = halfVSide * screenRatio;
		glm::vec3 frontMultFar = farDist * camForward;

		camFrustum.nearFace = { camPos + nearDist * camForward, camForward };
		camFrustum.farFace = { camPos + frontMultFar, -camForward };
		camFrustum.rightFace = { camPos,
								glm::cross(frontMultFar - camRight * halfHSide, camUp) };
		camFrustum.leftFace = { camPos,
								glm::cross(camUp,frontMultFar + camRight * halfHSide) };
		camFrustum.topFace = { camPos,
								glm::cross(camRight, frontMultFar - camUp * halfVSide) };
		camFrustum.bottomFace = { camPos,
									glm::cross(frontMultFar + camUp * halfVSide, camRight) };
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
		//for (int i = 0; i < cube.vertices.size(); i += 3)
		//	if ((camera * glm::vec4(pos + radius * Vec3(cube.vertices[i], cube.vertices[i + 1], cube.vertices[i + 2]), 1.f)).z < 0)
		//	{
		//	}
		// radius *= -1;
		toDrawCircles.push_back({ glm::vec4(pos, radius), glm::vec4(color.r / 255.f, color.g / 255.f, color.b / 255.f, color.a / 255.f) });
	}

	void DrawCircleInstantly(glm::vec4 posScale, RGBA color) // DOESN'T WORK
	{
		glUseProgram(circleShader);
		glUniform4f(glGetUniformLocation(circleShader, "posScale"), posScale.x, posScale.y, posScale.z, posScale.w);
		// The " / 255.0f" is to put the 0-255 range colors into 0-1 range colors.
		glUniform4f(glGetUniformLocation(circleShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);

		cube.Draw();
	}

	void DrawAllCircles()
	{
		glBindVertexArray(cube.vao);

		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 8 * toDrawCircles.size(), toDrawCircles.data(), GL_DYNAMIC_DRAW);
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);
		glUseProgram(circleShader);
		glDrawElementsInstanced(cube.mode, static_cast<GLsizei>(cube.indices.size()), GL_UNSIGNED_INT, 0, static_cast<GLsizei>(toDrawCircles.size()));
		toDrawCircles.clear();
		glEnable(GL_BLEND);
		glEnable(GL_CULL_FACE);
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
		glUniform3f(glGetUniformLocation(shadowShader, "pos"), pos.x, pos.y, pos.z);
		pos = camera * glm::vec4(pos, 1);
		glUniform3f(glGetUniformLocation(shadowShader, "position"), pos.x, pos.y, pos.z);
		glUniform1f(glGetUniformLocation(shadowShader, "range"), range);

		glUniform3f(glGetUniformLocation(shadowShader, "color"), color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);

		glCullFace(GL_FRONT);
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

	inline Vec3 PlayerPos(); // The position of the player
	inline Vec3 BasePos(); // The position of the player
	
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
};

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	((Renderer*)game.get())->inputs.mouseScroll += static_cast<int>(yoffset);
	((Renderer*)game.get())->inputs.mouseScrollF += static_cast<float>(yoffset);
}

void window_maximize_callback(GLFWwindow* window, int maximized)
{
	((Renderer*)game.get())->settings.maximized = maximized;
	((Renderer*)game.get())->settings.Write();

}