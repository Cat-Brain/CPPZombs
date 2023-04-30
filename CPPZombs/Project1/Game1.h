#include "Renderer.h"

class Entity;
class Entities;
class Player;
class Planet;

enum UIMODE
{
	MAINMENU, SETTINGS, CHARSELECT, INGAME, PAUSED
};

string difficultyStrs[] = { "Easy", "Medium", "Hard" };

class Game : public Renderer
{
public:
	unique_ptr<Entities> entities;
	Player* player;
	std::unique_ptr<Planet> planet;

	bool showUI = true;
	UIMODE uiMode = UIMODE::MAINMENU;
	float lastWave = 0.0f, secondsBetweenWaves = 60.0f, brightness = 0.0f;
	bool shouldSpawnBoss = false;
	float timeStartBossPrep = 0.0f;
	float screenShake = 0.0f;
	Vec3 lastPlayerPos = vZero;
	FastNoiseLite screenShkX, screenShkY;

	Game() : entities(nullptr), player(nullptr) { }

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