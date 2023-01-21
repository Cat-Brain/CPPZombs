#include "Include.h"

class Entity;
class Entities;
class Player;

class Game : public olc::PixelGameEngine
{
public:
	Entities* entities;
	Player* player;
	olc::Sprite lowResScreen, midResScreen;
	RGB shadowMap[screenWidth][screenHeight];
	Inputs inputs;

	Color backgroundBaseColor;
	Color backgroundColorWidth;
	FastNoiseLite backgroundNoise;

	bool showUI = true, paused = false;
	float lastWave = 0.0f, secondsBetweenWaves = 60.0f;
	float dTime = 0.0f;
	float brightness = 0.0f;

	Game() : entities(nullptr), player(nullptr)
	{
		for (int x = 0; x < screenWidth; x++)
			for (int y = 0; y < screenHeight; y++)
			{
				shadowMap[x][y][0] = 0u;
				shadowMap[x][y][1] = 0u;
				shadowMap[x][y][2] = 0u;
			}
	}

	bool OnUserCreate() override;

	bool OnUserUpdate(float deltaTime) override;

	Color GetBackgroundNoise(Vec2f noisePos);

	void ApplyLighting();

	void Update();

	void MenuedEntityDied(Entity* entity);

	bool OnUserDestroy() override;

	bool OnConsoleCommand(const string& text) override;
};