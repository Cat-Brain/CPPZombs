#include "Printers.h"

class Enemy : public DToCol
{
public:
	float timePer, lastTime;

	int points, firstWave;
	int damage;

	Enemy(float timePer = 0.5f, int points = 1, int firstWave = 1, int damage = 1, Vec2 dimensions = vOne, Color color = Color(olc::WHITE), Color color2 = Color(olc::BLACK), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		DToCol(vZero, dimensions, color, color2, mass, maxHealth, health, name), timePer(timePer), lastTime(0.0f), points(points), firstWave(firstWave), damage(damage)
	{
	}

	Enemy(Enemy* baseClass, Vec2 pos) :
		Enemy(*baseClass)
	{
		this->baseClass = baseClass;
		this->pos = pos;
		lastTime = (float)rand() / (float)RAND_MAX * timePer + tTime;
	}

	void Start() override
	{
		lastTime = tTime + RandFloat() * timePer;
	}

	Entity* Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		return new Enemy(this, pos);
	}

	virtual void BetterClone(Game* game, Entities* entities, Vec2 pos)
	{
		Enemy* newEnemy = new Enemy(this, pos);
		newEnemy->Start();
		entities->push_back(newEnemy);
	}

	bool IsEnemy() override
	{
		return true;
	}

	void Update(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
	{
		if (tTime - lastTime >= timePer)
		{
			TUpdate(game, entities, frameCount, inputs, dTime);
			lastTime = tTime;
		}
	}

	virtual void TUpdate(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime)
	{
		Entity* entity = nullptr;
		TryMove2(Squarmalized(playerPos - pos), mass, entities, &entity, nullptr);
		if (entity != nullptr && !entity->IsEnemy())
		{
			entity->DealDamage(damage, entities, this);
		}
	}

	Vec2 BottomRight() override
	{
		return DToCol::BottomRight() + Vec2(8 + (int)to_string(health).length() * 8, 0);
	}

	void UIUpdate(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
	{
		DrawUIBox(game, TopLeft(), BottomRight(), name + " " + to_string(health), color);
	}

	bool PosInUIBounds(Vec2 screenSpacePos) override
	{
		Vec2 topLeft = TopLeft();
		Vec2 bottomRight = BottomRight();
		return screenSpacePos.x >= topLeft.x && screenSpacePos.x <= bottomRight.x &&
			screenSpacePos.y >= topLeft.y && screenSpacePos.y <= bottomRight.y;
	}

	bool TryMove2(Vec2 dir, int force, Entities* entities, Entity* avoid)
	{
		Entity* hitEntityValue = nullptr;
		Entity** hitEntity = &hitEntityValue;
		if (!TryMove(dir, force, entities, hitEntity, avoid))
		{
			if (*hitEntity != nullptr && (*hitEntity)->IsEnemy())
			{
				Vec2 newDir = dir;
				bool randResult = PsuedoRandom() % 2;
				RotateRight45(newDir);
				if (randResult)
					RotateLeft(newDir); // Total of rotating left 45 degrees.
				if (!TryMove(newDir, force, entities, hitEntity, avoid))
				{
					if (*hitEntity != nullptr && (*hitEntity)->IsEnemy())
					{
						if (randResult)
							RotateRight(newDir);
						else
							RotateLeft(newDir);
						return TryMove(newDir, force, entities, hitEntity, avoid);
					}
				}
			}
			else return false;
		}
		return true;
	}

	bool TryMove2(Vec2 dir, int force, Entities* entities, Entity** hitEntity, Entity* avoid)
	{
		if (!TryMove(dir, force, entities, hitEntity, avoid))
		{
			if (*hitEntity != nullptr && (*hitEntity)->IsEnemy())
			{
				Vec2 newDir = dir;
				bool randResult = PsuedoRandom() % 2;
				RotateRight45(newDir);
				if (randResult)
					RotateLeft(newDir); // Total of rotating left 45 degrees.
				if (!TryMove(newDir, force, entities, hitEntity, avoid))
				{
					if (*hitEntity != nullptr && (*hitEntity)->IsEnemy())
					{
						if (randResult)
							RotateRight(newDir);
						else
							RotateLeft(newDir);
						return TryMove(newDir, force, entities, hitEntity, avoid);
					}
				}
			}
			else return false;
		}
		return true;
	}

	void OnDeath(Entities* entities, Entity* damageDealer) override
	{
		totalGamePoints += points;
		int randomValue = rand() % 2048; // 0-2047
		if (randomValue > 1022) // Half of the time is true I think.
		{
			if (randomValue > 1500) // 1501-2047 ~= 1/4
				entities->push_back(Collectibles::copper->Clone(pos));
			if (randomValue % 16 == 0) // ~1/16 of the time. The following and these can only have one of them happen
				entities->push_back(Collectibles::Seeds::copperTreeSeed->Clone(pos));
			else if (randomValue % 16 == 1)
				entities->push_back(Collectibles::Seeds::ironTreeSeed->Clone(pos));
			else if (randomValue % 16 == 2)
				entities->push_back(Collectibles::Seeds::cheeseTreeSeed->Clone(pos));
			else if (randomValue % 16 == 3)
				entities->push_back(Collectibles::Seeds::rubyTreeSeed->Clone(pos));
			else if (randomValue % 16 == 4)
				entities->push_back(Collectibles::Seeds::emeraldTreeSeed->Clone(pos));
			/*else if (randomValue % 16 == 5)
				entities->push_back(Shootables::cSmallPrinter->Clone(pos));
			else if (randomValue % 16 == 6)
				entities->push_back(Shootables::cSmallVacuum->Clone(pos));*/
		}
	}

	virtual int Cost()
	{
		return points;
	}
};

namespace EnemyClasses
{
	class Deceiver : public Enemy
	{
	public:
		Color color3;
		FastNoiseLite noise1, noise2, noise3; // <-For random colors.

		Deceiver(float timePer = 0.5f, int points = 1, int firstWave = 1, int damage = 1, Vec2 dimensions = vOne,
			Color color = olc::WHITE, Color color2 = olc::BLACK, Color color3 = olc::WHITE,
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, points, firstWave, damage, dimensions, color, color2, mass, maxHealth, health, name), color3(color3), noise1(), noise2(), noise3()
		{
			Start();
		}

		void BetterClone(Game* game, Entities* entities, Vec2 pos) override
		{
			EnemyClasses::Deceiver* newEnemy = new EnemyClasses::Deceiver(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			newEnemy->noise1.SetSeed(PsuedoRandom());
			newEnemy->noise2.SetSeed(PsuedoRandom());
			newEnemy->noise3.SetSeed(PsuedoRandom());
			entities->push_back(newEnemy);
		}

		void DUpdate(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime)
		{
			Color tempColor = color;
			Color multiplier = Color(static_cast<int>(255 * noise1.GetNoise(tTime, 0.0f)),
				static_cast<int>(255 * noise2.GetNoise(tTime, 0.0f)), static_cast<int>(255 * noise3.GetNoise(tTime, 0.0f)));
			color = Color(color.r * multiplier.r, color.g * multiplier.g, color.b * multiplier.b, color.a);

			Enemy::DUpdate(game, entities, frameCount, inputs, dTime);

			float healthRatio = (float)health / maxHealth;
			color = Color(color3.r * multiplier.r, color3.g * multiplier.g, color3.b * multiplier.b, color3.a);

			Vec2 tempPos = pos;

			pos = Vec2(playerPos.x * 2 - pos.x, pos.y);
			Enemy::DUpdate(game, entities, frameCount, inputs, dTime);
			pos = Vec2(pos.x, playerPos.y * 2 - pos.y);
			Enemy::DUpdate(game, entities, frameCount, inputs, dTime);
			pos = Vec2(playerPos.x * 2 - pos.x, pos.y);
			Enemy::DUpdate(game, entities, frameCount, inputs, dTime);

			color = tempColor;
			pos = tempPos;
		}

	};

	class Parent : public Enemy
	{
	public:
		Enemy* child;

		Parent(Enemy* child, float timePer = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, Color color = olc::WHITE, Color color2 = olc::BLACK,
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, points, firstWave, damage, dimensions, color, color2, mass, maxHealth, health, name), child(child)
		{
			Start();
		}

		void BetterClone(Game* game, Entities* entities, Vec2 pos) override
		{
			EnemyClasses::Parent* newEnemy = new EnemyClasses::Parent(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			entities->push_back(newEnemy);
		}

		void OnDeath(Entities* entities, Entity* damageDealer) override
		{
			Enemy::OnDeath(entities, damageDealer);

			entities->push_back(child->Clone(pos + up));
			entities->push_back(child->Clone(pos + right));
			entities->push_back(child->Clone(pos + down));
			entities->push_back(child->Clone(pos + left));
		}
	};

	class Exploder : public Enemy
	{
	public:
		Vec2 explosionDimensions;

		Exploder(Vec2 explosionDimensions, float timePer = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, Color color = olc::WHITE, Color color2 = olc::BLACK,
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, points, firstWave, damage, dimensions, color, color2, mass, maxHealth, health, name), explosionDimensions(explosionDimensions)
		{ }

		void BetterClone(Game* game, Entities* entities, Vec2 pos) override
		{
			EnemyClasses::Exploder* newEnemy = new EnemyClasses::Exploder(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			entities->push_back(newEnemy);
		}

		void OnDeath(Entities* entities, Entity* damageDealer) override
		{
			Enemy::OnDeath(entities, damageDealer);
			entities->push_back(new ExplodeNextFrame(damage, explosionDimensions, pos, name));
			entities->push_back(new FadeOut(1.5f, pos, explosionDimensions, color));
		}
	};

	class Snake : public Enemy
	{
	public:
		Vec2 lastPos;
		Color color3, color4;
		Snake* back = nullptr, *front = nullptr;
		int length;

		Snake(int length, float timePer = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, Color color = olc::WHITE, Color color2 = olc::BLACK,
			Color color3 = olc::RED, Color color4 = olc::DARK_GREEN,
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, points, firstWave, damage, dimensions, color, color2, mass, maxHealth, health, name), length(length), color3(color3), color4(color4)
		{
			Start();
		}

		void Start() override
		{
			Enemy::Start();
			lastPos = pos;
		}

		void BetterClone(Game* game, Entities* entities, Vec2 pos) override
		{
			Snake** enemies = new Snake * [length];
			for (int i = 0; i < length; i++)
			{
				enemies[i] = new Snake(*this);
				enemies[i]->baseClass = baseClass;
				enemies[i]->pos = pos;
				enemies[i]->Start();
				enemies[i]->color = olc::PixelLerp(color, color4, sinf(static_cast<float>(i)) * 0.5f + 0.5f);
				enemies[i]->lastTime = tTime;
			}
			
			if (length > 1)
			{
				enemies[0]->front = enemies[1];
				enemies[length - 1]->back = enemies[length - 2];
			}
			for (int i = 1; i < length - 1; i++)
			{
				enemies[i]->back = enemies[i - 1];
				enemies[i]->front = enemies[i + 1];
			}

			for (int i = length - 1; i > -1; i--)
				entities->push_back(enemies[i]);
		}

		void Update(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
		{
			Enemy::Update(game, entities, frameCount, inputs, dTime);
			//if (oldPos != pos && back != nullptr)
				//back->pos = oldPos;
			//if (front != nullptr)
				//TryMove2(Squarmalized(front->pos - pos), mass, entities, nullptr);
			if (front == nullptr)
				color = color3;
		}

		void TUpdate(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
		{
			if (front == nullptr)
			{
				Entity* entity = nullptr;
				TryMove2(Squarmalized(playerPos - pos), mass, entities, &entity, nullptr);
				if (entity != nullptr && !entity->IsEnemy())
				{
					entity->DealDamage(damage, entities, this);
				}
			}
			else
			{
				vector<Entity*> nearbyEnemies = entities->FindCorpOverlaps(pos, vOne * 2);
				for (Entity* entity : nearbyEnemies)
					if (!entity->IsEnemy())
						entity->DealDamage(damage, entities, this);
			}
			if (lastPos != pos)
			{
				if (back != nullptr)
					back->pos = lastPos;
				lastPos = pos;
			}
		}

		void OnDeath(Entities* entities, Entity* damageDealer) override
		{
			Enemy::OnDeath(entities, damageDealer);
			if (back != nullptr)
				back->front = nullptr;
			if (front != nullptr)
				front->back = nullptr;
		}
		
		int Cost() override
		{
			return points * length;
		}
	};

	class ColorCycler : public Enemy
	{
	public:
		vector<Color> colorsToCycle;
		float colorOffset, colorCycleSpeed;

		ColorCycler(vector<Color> colorsToCycle, float colorCycleSpeed = 1.0f, float timePer = 0.5f, int points = 1,
			int firstWave = 1, int damage = 1, Vec2 dimensions = vOne, Color color2 = olc::BLACK,
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, points, firstWave, damage, dimensions, colorsToCycle[0], color2, mass, maxHealth, health, name),
			colorsToCycle(colorsToCycle), colorCycleSpeed(colorCycleSpeed), colorOffset(0.0f)
		{ }

		void Start() override
		{
			Enemy::Start();
			colorOffset = RandFloat() * colorCycleSpeed;
		}

		void BetterClone(Game* game, Entities* entities, Vec2 pos) override
		{
			EnemyClasses::ColorCycler* newEnemy = new EnemyClasses::ColorCycler(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			entities->push_back(newEnemy);
		}

		void DUpdate(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
		{
			float currentPlace = (tTime * colorCycleSpeed + colorOffset);
			int intCurrentPlace = static_cast<int>(currentPlace);
			color = olc::PixelLerp(colorsToCycle[intCurrentPlace % colorsToCycle.size()],
				colorsToCycle[(intCurrentPlace + 1) % colorsToCycle.size()], currentPlace - floorf(currentPlace));

			Enemy::DUpdate(game, entities, frameCount, inputs, dTime);
		}
	};

	class Vacuumer : public Enemy
	{
	public:
		int vacDist, desiredDistance;
		Items items;

		Vacuumer(int vacDist, int desiredDistance, float timePer = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, Color color = olc::WHITE, Color color2 = olc::BLACK,
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, points, firstWave, damage, dimensions, color, color2, mass, maxHealth, health, name),
			vacDist(vacDist), desiredDistance(desiredDistance), items(0)
		{ }

		void BetterClone(Game* game, Entities* entities, Vec2 pos) override
		{
			EnemyClasses::Vacuumer* newEnemy = new EnemyClasses::Vacuumer(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			entities->push_back(newEnemy);
		}

		void TUpdate(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
		{
			float currentDistance = 0;
			if (fabsf((currentDistance = Distance(playerPos, pos)) - desiredDistance) > 1.0f)
			{
				Entity* entity = nullptr;
				TryMove2(Squarmalized((int(currentDistance > desiredDistance) * 2 - 1) * (playerPos - pos)), mass, entities, &entity, nullptr);
				if (entity != nullptr && !entity->IsEnemy())
				{
					entity->DealDamage(damage, entities, this);
				}
			}
			entities->Vacuum(pos, vacDist);
			vector<Entity*> collectibles = EntitiesOverlaps(pos, dimensions, entities->collectibles);
			for (Entity* collectible : collectibles)
			{
				items.push_back(((Collectible*)collectible)->baseItem);
				collectible->DestroySelf(entities, this);
			}
		}

		void OnDeath(Entities* entities, Entity* damageDealer) override
		{
			Enemy::OnDeath(entities, damageDealer);
			for (Item item : items)
			{
				entities->push_back(new Collectible(item, pos));
			}
		}
	};

	class Ranger : public Vacuumer
	{
	public:
		using Vacuumer::Vacuumer;

		void BetterClone(Game* game, Entities* entities, Vec2 pos) override
		{
			EnemyClasses::Ranger* newEnemy = new EnemyClasses::Ranger(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			entities->push_back(newEnemy);
		}

		void TUpdate(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
		{
			Vacuumer::TUpdate(game, entities, frameCount, inputs, dTime);

			if (items.size() > 0)
			{
				Item shotItem = items[0].Clone(1);
				items.TryTakeIndex(0);
				entities->push_back(basicShotItem->Clone(shotItem, pos, (playerPos - pos) * shotItem.range, this));
			}
		}
	};
}




Enemy* walker = new Enemy(0.75f, 1, 1, 1, vOne, olc::CYAN, olc::BLACK, 1, 3, 3, "Walker");
Enemy* tanker = new Enemy(1.0f, 2, 2, 1, vOne * 2, olc::RED, olc::BLACK, 5, 12, 12, "Tanker");
Enemy* speedster = new Enemy(0.5f, 2, 3, 1, vOne, olc::YELLOW, olc::BLACK, 1, 2, 2, "Speedster");
EnemyClasses::ColorCycler* hyperSpeedster = new EnemyClasses::ColorCycler({olc::RED, olc::YELLOW, olc::BLUE}, 2.0f, 0.25f, 4, 10, 1, vOne, olc::BLACK, 1, 24, 24, "Hyper Speedster");
Enemy* megaTanker = new Enemy(1.0f, 20, 15, 1, vOne * 3, Color(174, 0, 255), olc::BLACK, 10, 48, 48, "Mega Tanker");

EnemyClasses::Deceiver* deceiver = new EnemyClasses::Deceiver(0.5f, 5, 4, 1, vOne, olc::WHITE, olc::BLACK, Color(255, 255, 255, 200), 1, 3, 3, "Deceiver");

Enemy* child = new Enemy(0.125f, 10, 0, 1, vOne, olc::MAGENTA, olc::BLACK, 1, 1, 1, "Child");
EnemyClasses::Parent* parent = new EnemyClasses::Parent(child, 1.0f, 10, 6, 1, vOne * 3, olc::DARK_MAGENTA, olc::BLACK, 5, 10, 10, "Parent");

EnemyClasses::Exploder* exploder = new EnemyClasses::Exploder(vOne * 3, 0.25f, 10, 5, 1, vOne, Color(153, 255, 0), olc::BLACK, 1, 3, 3, "Exploder");
EnemyClasses::Exploder* gigaExploder = new EnemyClasses::Exploder(vOne * 8, 0.25f, 25, 13, 1, vOne * 2, Color(153, 255, 0), olc::BLACK, 1, 3, 3, "Giga Exploder");
EnemyClasses::Snake* snake = new EnemyClasses::Snake(30, 0.25f, 1, 7, 1, vOne, olc::GREEN, olc::BLACK, olc::RED, olc::DARK_GREEN, 2, 3, 3, "Snake");

EnemyClasses::Vacuumer* vacuumer = new EnemyClasses::Vacuumer(12, 12, 0.125f, 6, 5, 0, vOne, olc::GREY, olc::BLACK, 1, 3, 3, "Vacuumer");
EnemyClasses::Ranger* ranger = new EnemyClasses::Ranger(12, 12, 0.125f, 12, 13, 0, vOne * 3, olc::GREY, olc::BLACK, 1, 12, 12, "Ranger");

vector<Enemy*> spawnableEnemyTypes{ walker, tanker, speedster, hyperSpeedster, megaTanker, deceiver, parent, exploder, gigaExploder,
snake, vacuumer, ranger };