#include "Renderer.h"

class Entity;
class Entities;
class Player;
class Planet;

enum UIMODE
{
	MAINMENU, CHARSELECT, INGAME, PAUSED
};

enum DIFFICULTY
{
	EASY, MEDIUM, HARD
};

string difficultyStrs[] = { "Easy", "Medium", "Hard" };

enum CHARS
{
	COMMANDER, MESSENGER
};

class Game : public Renderer
{
public:
	unique_ptr<Entities> entities;
	CHARS selectedCharacter = CHARS::COMMANDER;
	Player* player;
	std::unique_ptr<Planet> planet;

	bool showUI = true;
	UIMODE uiMode = UIMODE::MAINMENU;
	DIFFICULTY difficulty = DIFFICULTY::MEDIUM;
	float lastWave = 0.0f, secondsBetweenWaves = 60.0f, brightness = 0.0f;
	bool shouldSpawnBoss = false;
	float timeStartBossPrep = 0.0f;
	float screenShake = 0.0f;
	//Vec2 deadPlayerPos = vZero;
	FastNoiseLite screenShkX, screenShkY;

	Game() : entities(nullptr), player(nullptr) { }

	void Start() override;

	void Update() override;

	void End() override;

	inline Vec3 PlayerPos() override;

	void ApplyLighting();

	void TUpdate();

	void MenuedEntityDied(Entity* entity);

	float BrightnessAtPos(iVec2 pos);
};