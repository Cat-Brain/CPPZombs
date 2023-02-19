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

	bool showUI = true, paused = false;
	float lastWave = 0.0f, secondsBetweenWaves = 60.0f, brightness = 0.0f;
	bool shouldSpawnBoss = false;
	float timeStartBossPrep = 0.0f;

	Game() : entities(nullptr), player(nullptr) { }

	void Start() override;

	void Update() override;

	void End() override;

	inline Vec2 PlayerPos() override;

	void ApplyLighting();

	void TUpdate();

	void MenuedEntityDied(Entity* entity);

	float BrightnessAtPos(Vec2 pos);
};