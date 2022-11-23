#include "Defence.h"

class Enemy : public DToCol
{
public:
	int tickPer, moveOffset;

	Enemy(int tickPer = 2, Color color = Color(olc::WHITE), Color color2 = Color(olc::BLACK), Recipe cost = Recipes::dRecipe, int mass = 1, int maxHealth = 1, int health = 1) :
		DToCol(Vec2(0, 0), color, color2, cost, mass, maxHealth, health), tickPer(tickPer)
	{ }

	Enemy(Enemy* baseClass, Vec2 pos) :
		Enemy(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		moveOffset = PsuedoRandom() % tickPer;
		Start();
	}

	void Update(Screen* screen, vector<Entity*>*entities, int frameCount, Inputs inputs) override
	{
		if ((frameCount + moveOffset) % tickPer == 0)
			TUpdate(screen, (Entities*)entities, frameCount, inputs);

		DToCol::Update(screen, entities, frameCount, inputs);
	}

	bool IsEnemy() override
	{
		return true;
	}

	virtual int GetDamage()
	{
		return 1;
	}

	virtual int GetPoints()
	{
		return 1;
	}

	virtual void TUpdate(Screen * screen, Entities* entities, int frameCount, Inputs inputs)
	{
		Entity* entity;
		if (!TryMove(Squarmalized(playerPos - pos), 1, (vector<Entity*>*)entities, &entity) && !entity->IsEnemy())
		{
			entity->DealDamage(GetDamage(), entities);
		}
	}

	void OnDeath(vector<Entity*>* entities) override
	{
		totalGamePoints += GetPoints();
		int randomValue = rand() % 2048; // 0-2047
		if (randomValue > 1022) // Half of the time is true I think.
		{
			if (randomValue > 1500) // 1501-2047 ~= 1/4
				((Entities*)entities)->push_back(Collectibles::copper->Clone(ToRandomCSpace(pos)));
			if (randomValue % 16 == 0) // ~1/16 of the time.
				((Entities*)entities)->push_back(cCopperTreeSeed->Clone(ToRandomCSpace(pos)));
			else if (randomValue % 16 == 1) // Never does this and copper tree seed.
				((Entities*)entities)->push_back(cIronTreeSeed->Clone(ToRandomCSpace(pos)));
		}
	}
};

Enemy* walker = new Enemy(24, olc::CYAN, olc::BLACK, Recipes::dRecipe, 1, 3, 3);
Enemy* tanker = new Enemy(36, olc::DARK_CYAN, olc::BLACK, Recipes::dRecipe, 5, 12, 12);
Enemy* speedster = new Enemy(12, olc::DARK_YELLOW, olc::BLACK, Recipes::dRecipe, 1, 2, 2);