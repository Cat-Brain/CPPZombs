#include "Include.h"

class Entity;
class Entities;
class Player;
class Planet;

class Game : public olc::PixelGameEngine
{
public:
	unique_ptr<Entities> entities;
	shared_ptr<Player> player;
	olc::Sprite lowResScreen, midResScreen;
	JRGB shadowMap[screenWidth * 3][screenHeight * 3];
	Inputs inputs;

	Color backgroundBaseColor;
	Color backgroundColorWidth;
	FastNoiseLite backgroundNoise;

	bool showUI = true, paused = false;
	float lastWave = 0.0f, secondsBetweenWaves = 60.0f;
	float dTime = 0.0f;
	float brightness = 0.0f;
	bool shouldSpawnBoss;
	float timeStartBossPrep = 0.0f;

	std::unique_ptr<Planet> planet;

	Game() : entities(nullptr), player(nullptr)
	{
		for (int x = 0; x < screenWidth * 3; x++)
			for (int y = 0; y < screenHeight * 3; y++)
			{
				shadowMap[x][y].r = 0u;
				shadowMap[x][y].g = 0u;
				shadowMap[x][y].b = 0u;
			}
	}

	bool OnUserCreate() override;

	bool OnUserUpdate(float deltaTime) override;

	bool OnUserDestroy() override;

	bool OnConsoleCommand(const string& text) override;

	void ApplyLighting();

	void Update();

	void MenuedEntityDied(Entity* entity);
};