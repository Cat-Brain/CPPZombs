#include "Projectile.h"

class Enemy : public DToCol
{
public:
	Enemy(Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), Color color2 = Color(olc::BLACK), int mass = 1, int maxHealth = 1, int health = 1) :
		DToCol(pos, color, color2, mass, maxHealth, health)
	{ }

	void Update(olc::PixelGameEngine * screen, vector<Entity*>*entities, int frameCount, Inputs inputs) override
	{
		if (frameCount % TickPer() == 0)
			TUpdate(screen, (Entities*)entities, frameCount, inputs);

		DToCol::Update(screen, entities, frameCount, inputs);
	}

	bool IsEnemy() override
	{
		return true;
	}

	virtual int TickPer()
	{
		return 2;
	}

	virtual int GetDamage()
	{
		return 1;
	}

	virtual void TUpdate(olc::PixelGameEngine * screen, Entities* entities, int frameCount, Inputs inputs)
	{
		int index;
		if (!TryMove(Squarmalized(playerPos - pos), 1, *(vector<Entity*>*)entities, &index) && std::find(entities->enemies.begin(), entities->enemies.end(), (*entities)[index]) == entities->enemies.end())
		{
			(*entities)[index]->DealDamage(GetDamage(), entities);
		}
	}
};

class Walker : public Enemy
{
public:
	using Enemy::Enemy;

	int TickPer() override
	{
		return 6;
	}
};