#include "Defence.h"

class Enemy : public DToCol
{
public:
	float timePer, lastMove, points;

	Enemy(float timePer = 0.5f, int points = 1, Color color = Color(olc::WHITE), Color color2 = Color(olc::BLACK), Recipe cost = Recipes::dRecipe, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		DToCol(Vec2(0, 0), color, color2, cost, mass, maxHealth, health, name), timePer(timePer), points(points)
	{ }

	Enemy(Enemy* baseClass, Vec2 pos) :
		Enemy(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		lastMove = (float)rand() / (float)RAND_MAX * timePer + tTime; // Randomly offsetted.
		Start();
	}

	void Update(Screen* screen, vector<Entity*>*entities, int frameCount, Inputs inputs, float dTime) override
	{
		if (tTime - lastMove > timePer)
		{
			lastMove += timePer;
			TUpdate(screen, (Entities*)entities, frameCount, inputs);
		}

		DToCol::Update(screen, entities, frameCount, inputs, dTime);
	}

	bool IsEnemy() override
	{
		return true;
	}

	virtual int GetDamage()
	{
		return 1;
	}

	virtual void TUpdate(Screen * screen, Entities* entities, int frameCount, Inputs inputs)
	{
		Entity* entity;
		if (!TryMove(Squarmalized(playerPos - pos), 1, (vector<Entity*>*)entities, &entity, nullptr) && !entity->IsEnemy())
		{
			entity->DealDamage(GetDamage(), entities, this);
		}
	}

	void OnDeath(vector<Entity*>* entities, Entity* damageDealer) override
	{
		totalGamePoints += points;
		int randomValue = rand() % 2048; // 0-2047
		if (randomValue > 1022) // Half of the time is true I think.
		{
			if (randomValue > 1500) // 1501-2047 ~= 1/4
				((Entities*)entities)->push_back(Collectibles::copper->Clone(ToRandomCSpace(pos)));
			if (randomValue % 16 == 0) // ~1/16 of the time. The following and these can only have one of them happen
				((Entities*)entities)->push_back(cCopperTreeSeed->Clone(ToRandomCSpace(pos)));
			else if (randomValue % 16 == 1)
				((Entities*)entities)->push_back(cIronTreeSeed->Clone(ToRandomCSpace(pos)));
			else if (randomValue % 16 == 2)
				((Entities*)entities)->push_back(cCheeseTreeSeed->Clone(ToRandomCSpace(pos)));
		}
	}
};

Enemy* walker = new Enemy(2.0f / 3.0f, 1, olc::CYAN, olc::BLACK, Recipes::dRecipe, 1, 3, 3, "a Walker");
Enemy* tanker = new Enemy(1.0f, 2, olc::RED, olc::BLACK, Recipes::dRecipe, 5, 12, 12, "a Tanker");
Enemy* speedster = new Enemy(0.5f, 2, olc::YELLOW, olc::BLACK, Recipes::dRecipe, 1, 2, 2, "a Speedster");
Enemy* hyperSpeedster = new Enemy(0.25f, 10, Color(255, 127, 0), olc::BLACK, Recipes::dRecipe, 1, 24, 24, "a Hyper Speedster");