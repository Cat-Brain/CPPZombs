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
	Inputs inputs;

	Color backgroundBaseColor;
	Color backgroundColorWidth;
	FastNoiseLite backgroundNoise;

	bool showUI = true, paused = false;
	float lastWave = 0.0f, secondsBetweenWaves = 60.0f;



	Game() : entities(nullptr), player(nullptr) { }

	bool OnUserCreate() override;

	bool OnUserUpdate(float deltaTime) override;

	Color GetBackgroundNoise(Vec2f noisePos);

	void Update(float dTime);

	bool OnUserDestroy() override;

	bool OnConsoleCommand(const string& text) override;
};