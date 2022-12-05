#include "Defence.h"

class Enemy : public DToCol
{
public:
	float timePer, lastMove, points;

	Enemy(float timePer = 0.5f, int points = 1, Color color = Color(olc::WHITE), Color color2 = Color(olc::BLACK), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		DToCol(Vec2(0, 0), color, color2, mass, maxHealth, health, name), timePer(timePer), points(points)
	{
		Start();
	}

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

	bool TryMove2(Vec2 dir, int force, vector<Entity*>* entities, Entity** hitEntity, Entity* avoid)
	{
		if (TryMove(dir, force, entities, hitEntity, avoid))
		{
			if ((*hitEntity)->IsEnemy())
			{
				Vec2 newDir = dir;
				bool randResult = PsuedoRandom() % 2;
				RotateRight45(newDir);
				if (randResult)
					RotateLeft(newDir); // Total of rotating left 45 degrees.
				if (TryMove(newDir, force, entities, hitEntity, avoid))
				{
					if ((*hitEntity)->IsEnemy())
					{
						if (randResult)
							RotateRight(newDir);
						else
							RotateLeft(newDir);
						return TryMove(newDir, force, entities, hitEntity, avoid);
					}
					else
						return true;
				}
			}
			else
				return true;
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


namespace EnemyClasses
{
	class Deceiver : public Enemy
	{
	public:
		Color color3;
		FastNoiseLite noise1, noise2, noise3; // <-For random colors.

		Deceiver(float timePer = 0.5f, int points = 1, Color color = olc::WHITE, Color color2 = olc::BLACK, Color color3 = olc::WHITE, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, points, color, color2, mass, maxHealth, health, name), color3(color3), noise1(), noise2(), noise3()
		{
			Start();
		}

		Deceiver(Deceiver* baseClass, Vec2 pos) :
			Deceiver(*baseClass)
		{
			this->pos = pos;
			this->baseClass = baseClass;
			lastMove = (float)rand() / (float)RAND_MAX * timePer + tTime; // Randomly offsetted.
			noise1.SetSeed(PsuedoRandom());
			noise2.SetSeed(PsuedoRandom());
			noise3.SetSeed(PsuedoRandom());
			Start();
		}

		void DUpdate(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime)
		{
			Color tempColor = color;
			Color multiplier = Color(255 * noise1.GetNoise(tTime, 0.0f), 255 * noise2.GetNoise(tTime, 0.0f), 255 * noise3.GetNoise(tTime, 0.0f));
			color = Color(color.r * multiplier.r, color.g * multiplier.g, color.b * multiplier.b, color.a);

			Enemy::DUpdate(screen, entities, frameCount, inputs, dTime);

			float healthRatio = (float)health / maxHealth;
			color = Color(color3.r * multiplier.r, color3.g * multiplier.g, color3.b * multiplier.b, color3.a);

			Vec2 tempPos = pos;

			pos = Vec2(playerPos.x * 2 - pos.x, pos.y);
			Enemy::DUpdate(screen, entities, frameCount, inputs, dTime);
			pos = Vec2(pos.x, playerPos.y * 2 - pos.y);
			Enemy::DUpdate(screen, entities, frameCount, inputs, dTime);
			pos = Vec2(playerPos.x * 2 - pos.x, pos.y);
			Enemy::DUpdate(screen, entities, frameCount, inputs, dTime);

			color = tempColor;
			pos = tempPos;
		}

	};
}




Enemy* walker = new Enemy(2.0f / 3.0f, 1, olc::CYAN, olc::BLACK, 1, 3, 3, "Walker");
Enemy* tanker = new Enemy(1.0f, 2, olc::RED, olc::BLACK, 5, 12, 12, "Tanker");
Enemy* speedster = new Enemy(0.5f, 2, olc::YELLOW, olc::BLACK, 1, 2, 2, "Speedster");
Enemy* hyperSpeedster = new Enemy(0.25f, 10, Color(255, 127, 0), olc::BLACK, 1, 24, 24, "Hyper Speedster");

EnemyClasses::Deceiver* deceiver = new EnemyClasses::Deceiver(0.5f, 5, olc::WHITE, olc::BLACK, Color(255, 255, 255, 200), 1, 3, 3, "Deceiver");