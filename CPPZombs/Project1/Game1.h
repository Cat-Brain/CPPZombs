#include "Renderer.h"

class Entity;
class Entities;
class Player;
class Planet;

class Game : public Renderer
{
public:
	unique_ptr<Entities> entities;
	Player* player;
	std::unique_ptr<Planet> planet;

	//JRGB shadowMap[screenWidth * 3][screenHeight * 3];

	bool showUI = true, paused = false;
	float lastWave = 0.0f, secondsBetweenWaves = 60.0f, brightness = 0.0f;
	bool shouldSpawnBoss = false;
	float timeStartBossPrep = 0.0f;

	Game() : entities(nullptr), player(nullptr)
	{
		/*for (int x = 0; x < screenWidth * 3; x++)
			for (int y = 0; y < screenHeight * 3; y++)
			{
				shadowMap[x][y].r = 0u;
				shadowMap[x][y].g = 0u;
				shadowMap[x][y].b = 0u;
			}*/
	}

	void Start() override;

	void Update() override;

	void End() override;

	inline Vec2 PlayerPos() override;

	void ApplyLighting();

	void TUpdate();

	void MenuedEntityDied(Entity* entity);
};