#include "Projectile.h"

class Enemy : public DToCol
{
public:
	int tickPer;

	Enemy(int tickPer = 2, Color color = Color(olc::WHITE), Color color2 = Color(olc::BLACK), int mass = 1, int maxHealth = 1, int health = 1) :
		DToCol(Vec2(0, 0), color, color2, mass, maxHealth, health), tickPer(tickPer)
	{ }

	Enemy(Enemy* baseClass, Vec2 pos) :
		Enemy(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	void Update(Screen* screen, vector<Entity*>*entities, int frameCount, Inputs inputs) override
	{
		if (frameCount % tickPer == 0)
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
		int index;
		if (!TryMove(Squarmalized(playerPos - pos), 1, *(vector<Entity*>*)entities, &index) && std::find(entities->enemies.begin(), entities->enemies.end(), (*entities)[index]) == entities->enemies.end())
		{
			(*entities)[index]->DealDamage(GetDamage(), entities);
		}
	}

	void OnDeath(vector<Entity*>* entities) override
	{
		totalGamePoints += GetPoints();
		int randomValue = rand() % 4; // 0, 1, 2, 3
		if (randomValue > 1) // 2 or 3
		{
			if (randomValue == 2) // 2
				((Entities*)entities)->push_back(new MiniEntity(basicBullet, pos));
			else // 3
				((Entities*)entities)->push_back(new MiniEntity(cheese, pos));
		}
	}
};

Enemy* walker = new Enemy(6, olc::CYAN, olc::BLACK, 1, 3, 3);