#include "Renderer.h"

class Entity; class Entities; class Player;
class Base; class Planet;

enum UIMODE
{
	MAINMENU, SETTINGS, CHARSELECT, INGAME, PAUSED
};

string difficultyStrs[] = { "Easy", "Medium", "Hard" };


enum class LOGMODE
{
	MAIN, CONTROLS, PLANTS, TOWERS
};
class LogBook
{
public:
	bool isOpen = false;
	int individualIndex = -1, wrapIndex = 0;
	float scroll = 0;
	LOGMODE logMode = LOGMODE::MAIN;

	void DUpdate();
};

class Game : public Renderer
{
public:
	unique_ptr<Entities> entities;
	Player* player;
	Base* base;
	unique_ptr<Planet> planet;
	unique_ptr<LogBook> logBook;

	bool showUI = true;
	UIMODE uiMode = UIMODE::MAINMENU;
	float lastWave = 0.0f, secondsBetweenWaves = 60.0f, brightness = 0.0f;
	bool shouldSpawnBoss = false;
	float timeStartBossPrep = 0.0f;
	float screenShake = 0.0f;
	Vec3 lastPlayerPos = vZero;
	FastNoiseLite screenShkX, screenShkY, screenShkZ;

	Game() : entities(nullptr), player(nullptr), base(nullptr), logBook(make_unique<LogBook>()) { }

	void Start() override;

	void Update() override;

	void End() override;

	void ApplyLighting();

	void TUpdate();

	void MenuedEntityDied(Entity* entity);

	float BrightnessAtPos(iVec2 pos);
};

Vec3 Renderer::PlayerPos()
{
	return ((Game*)this)->lastPlayerPos;
}

bool CheckInputSquareHover(Vec2 minPos, float scale, string text) // Returns if square is being hovered over.
{
	return game->inputs.screenMousePosition.x > minPos.x && game->inputs.screenMousePosition.x < minPos.x + font.TextWidth(text) * scale / font.minimumSize &&
		game->inputs.screenMousePosition.y > minPos.y + scale * font.mininumVertOffset / font.minimumSize &&
		game->inputs.screenMousePosition.y < minPos.y + scale * font.maxVertOffset / font.minimumSize;
}

bool InputHoverSquare(Vec2 minPos, float scale, string text, RGBA color1 = RGBA(255, 255, 255), RGBA color2 = RGBA(127, 127, 127)) // Renders and returns if this was clicked on(not hovered over).
{
	if (CheckInputSquareHover(minPos, scale, text))
	{
		font.Render(text, minPos * 2.f - Vec2(ScrDim()), scale * 2, color2);
		return game->inputs.keys[KeyCode::PRIMARY].pressed;
	}
	else
		font.Render(text, { minPos * 2.f - Vec2(ScrDim()) }, scale * 2, color1);
	return false;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (game->inputs.bindingButton != -1 && action == GLFW_PRESS)
		game->inputs.SetKey(key);
}