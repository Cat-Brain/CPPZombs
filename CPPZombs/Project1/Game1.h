#include "Renderer.h"

class Entity; class Entities; class Player;
class Base; class Planet;
enum class SEEDINDICES;

#pragma region Psuedo-Virtuals
typedef function<void(Entity* entity)> Update, VUpdate, DUpdate, UIUpdate;
typedef function<void(Entity* entity, Entity* damageDealer)> OnDeath;
typedef function<bool(Entity* entity)> TUpdate;
typedef function<bool(Entity* from, Entity* to)> EntityMaskFun;
typedef function<float(Entity* from, Entity* to)> EntityExtremetyFun;

enum class STARTCALLBACK
{
	NONE, TUTORIAL1, TUTORIAL2, TUTORIAL3, TUTORIAL4
};
vector<function<void()>> startCallbacks;

enum class UPDATEMODE
{
	MAINMENU, CHAR_SELECT, SEED_SELECT, TUTORIAL_SELECT, IN_GAME, PAUSED, DEAD
};
vector<function<void()>> updateModes;

enum class PREUPDATE
{
	DEFAULT, TUTORIAL1, TUTORIAL2, TUTORIAL3, TUTORIAL4
};
vector<function<void()>> preUpdates;

enum class POSTUPDATE
{
	DEFAULT, TUTORIAL1, TUTORIAL2, TUTORIAL3
};
vector<function<void()>> postUpdates;
#pragma endregion

vector<string> tutorialStrs = { "Basic Combat", "Basic Farming and Building", "Inventory management" };

enum class LOGMODE
{
	// Log book
	MAIN, CONTROLS, PLANTS, TOWERS,
	// Settings
	SETTINGS
};
class LogBook
{
protected:
	bool isLogBook = false;
	int individualIndex = -1, wrapIndex = 0;
	float scroll = 0;
	LOGMODE logMode = LOGMODE::MAIN;
public:
	bool isOpen = false;

	void OpenLogBook()
	{
		isOpen = true;
		logMode = LOGMODE::MAIN;
	}

	void OpenSettings()
	{
		isOpen = true;
		logMode = LOGMODE::SETTINGS;
	}

	void DUpdate();
};

class Game : public Renderer
{
public:
	unique_ptr<Entities> entities;
	Player* player;
	Entity* playerE; // Same as player but as an enity*
	Base* base;
	unique_ptr<Planet> planet;
	unique_ptr<LogBook> logBook;



	bool showUI = true;

	STARTCALLBACK startCallback = STARTCALLBACK::NONE;
	UPDATEMODE updateMode = UPDATEMODE::MAINMENU;
	PREUPDATE preUpdate = PREUPDATE::DEFAULT;
	POSTUPDATE postUpdate = POSTUPDATE::DEFAULT;

	byte tutorialState = 0;

	float brightness = 0.0f;
	bool shouldSpawnBoss = false;
	float timeStartBossPrep = 0.0f;
	float screenShake = 0.0f;
	float scroll = 0;
	Vec3 lastPlayerPos = vZero;
	FastNoiseLite screenShkX, screenShkY, screenShkZ;
	float endTimer = -1;
	int endWidth = -1, endHeight = -1;
	float growthSpeedMul = 1;

	vector<std::pair<string, RGBA>> specialData;

	DIFFICULTY difficulty = DIFFICULTY::EASY;
	bool startSeeds[UnEnum(SEEDINDICES::COUNT)] = { false };

	Game() : entities(nullptr), player(nullptr), base(nullptr), logBook(make_unique<LogBook>()) { }

	void EndRun()
	{
		updateMode = UPDATEMODE::DEAD;
		if (startCallback == STARTCALLBACK::NONE && preUpdate == PREUPDATE::DEFAULT && postUpdate == POSTUPDATE::DEFAULT)
		{
			settings.highScores[difficulty] = max(settings.highScores[difficulty], totalGamePoints);
			settings.Write();
		}
	}

	void Start() override;

	void Update() override;

	bool End() override;

	void ApplyLighting();

	void MenuedEntityDied(Entity* entity);

	float BrightnessAtPos(iVec2 pos);
};

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