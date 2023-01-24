#include "Printers.h"

class Enemy : public DToCol
{
public:
	float timePer, lastTime;

	int points, firstWave;
	int damage;

	Enemy(float timePer = 0.5f, int points = 1, int firstWave = 1, int damage = 1, Vec2 dimensions = vOne, Color color = olc::WHITE,
		Color color2 = olc::BLACK, Color subsurfaceResistance = olc::WHITE, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		DToCol(vZero, dimensions, color, color2, subsurfaceResistance, mass, maxHealth, health, name), timePer(timePer), lastTime(0.0f), points(points), firstWave(firstWave), damage(damage)
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

	shared_ptr<Entity> Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
	{
		return make_shared<Enemy>(this, pos);
	}

	bool IsEnemy() override
	{
		return true;
	}

	void Update() override
	{
		if (tTime - lastTime >= timePer)
		{
			TUpdate();
			lastTime = tTime;
		}
	}

	virtual void TUpdate()
	{
		Entity* entity = nullptr;
		TryMove2(Squarmalized(playerPos - pos), mass, nullptr, &entity);
		if (entity != nullptr && !entity->IsEnemy())
		{
			entity->DealDamage(damage, this);
		}
	}

	Vec2 BottomRight() override
	{
		return DToCol::BottomRight() + Vec2(8 + (int)to_string(health).length() * 8, 0);
	}

	void UIUpdate() override
	{
		DrawUIBox(TopLeft(), BottomRight(), name + " " + to_string(health), color);
	}

	bool PosInUIBounds(Vec2 screenSpacePos) override
	{
		Vec2 topLeft = TopLeft();
		Vec2 bottomRight = BottomRight();
		return screenSpacePos.x >= topLeft.x && screenSpacePos.x <= bottomRight.x &&
			screenSpacePos.y >= topLeft.y && screenSpacePos.y <= bottomRight.y;
	}

	bool TryMove2(Vec2 dir, int force, Entity* avoid, Entity** hitEntity)
	{
		Entity* hitEntityValue = nullptr;
		if (hitEntity == nullptr)
			hitEntity = &hitEntityValue;
		if (!TryMove(dir, force, avoid, hitEntity))
		{
			if (*hitEntity != nullptr && (*hitEntity)->IsEnemy())
			{
				Vec2 newDir = dir;
				bool randResult = PsuedoRandom() % 2;
				RotateRight45(newDir);
				if (randResult)
					RotateLeft(newDir); // Total of rotating left 45 degrees.
				if (!TryMove(newDir, force, avoid, hitEntity))
				{
					if (*hitEntity != nullptr && (*hitEntity)->IsEnemy())
					{
						if (randResult)
							RotateRight(newDir);
						else
							RotateLeft(newDir);
						return TryMove(newDir, force, avoid, hitEntity);
					}
				}
			}
			else return false;
		}
		return true;
	}

	void OnDeath(Entity* damageDealer) override
	{
		totalGamePoints += points;
		int randomValue = rand() % 2048; // 0-2047
		if (randomValue > 1022) // Half of the time is true I think.
		{
			if (randomValue > 1500) // 1501-2047 ~= 1/4
				game->entities->push_back(Collectibles::copper->Clone(pos));
			else if (randomValue % 16 < Collectibles::Seeds::plantSeeds.size()) // This system will work until there's > 16 plants.
				game->entities->push_back(Collectibles::Seeds::plantSeeds[randomValue % 16]->Clone(pos));
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
			Color color = olc::WHITE, Color color2 = olc::BLACK, Color color3 = olc::WHITE, Color subsurfaceResistance = olc::WHITE,
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, points, firstWave, damage, dimensions, color, color2, subsurfaceResistance, mass, maxHealth, health, name), color3(color3), noise1(), noise2(), noise3()
		{
			Start();
		}

		shared_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			shared_ptr<EnemyClasses::Deceiver> newEnemy = make_shared<EnemyClasses::Deceiver>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			newEnemy->noise1.SetSeed(PsuedoRandom());
			newEnemy->noise2.SetSeed(PsuedoRandom());
			newEnemy->noise3.SetSeed(PsuedoRandom());
			return newEnemy;
		}

		void DUpdate() override
		{
			Color tempColor = color;
			Color multiplier = Color(static_cast<int>(255 * noise1.GetNoise(tTime, 0.0f)),
				static_cast<int>(255 * noise2.GetNoise(tTime, 0.0f)), static_cast<int>(255 * noise3.GetNoise(tTime, 0.0f)));
			color = Color(color.r * multiplier.r, color.g * multiplier.g, color.b * multiplier.b, color.a);

			Enemy::DUpdate();

			float healthRatio = (float)health / maxHealth;
			color = Color(color3.r * multiplier.r, color3.g * multiplier.g, color3.b * multiplier.b, color3.a);

			Vec2 tempPos = pos;

			pos = Vec2(playerPos.x * 2 - pos.x, pos.y);
			Enemy::DUpdate();
			pos = Vec2(pos.x, playerPos.y * 2 - pos.y);
			Enemy::DUpdate();
			pos = Vec2(playerPos.x * 2 - pos.x, pos.y);
			Enemy::DUpdate();

			color = tempColor;
			pos = tempPos;
		}

	};

	class Parent : public Enemy
	{
	public:
		Enemy* child;

		Parent(Enemy* child, float timePer = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, Color color = olc::WHITE, Color color2 = olc::BLACK, Color subsurfaceResistance = olc::WHITE,
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, points, firstWave, damage, dimensions, color, color2, subsurfaceResistance, mass, maxHealth, health, name), child(child)
		{
			Start();
		}

		shared_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			shared_ptr<EnemyClasses::Parent> newEnemy = make_shared<EnemyClasses::Parent>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}

		void DUpdate() override
		{
			Enemy::DUpdate();
			child->Draw(pos + up);
			child->Draw(pos + left);
			child->Draw(pos + down);
			child->Draw(pos + right);
		}

		void OnDeath(Entity* damageDealer) override
		{
			Enemy::OnDeath(damageDealer);

			game->entities->push_back(child->Clone(pos + up));
			game->entities->push_back(child->Clone(pos + right));
			game->entities->push_back(child->Clone(pos + down));
			game->entities->push_back(child->Clone(pos + left));
		}
	};

	class Exploder : public Enemy
	{
	public:
		Vec2 explosionDimensions;

		Exploder(Vec2 explosionDimensions, float timePer = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, Color color = olc::WHITE, Color color2 = olc::BLACK, Color subsurfaceResistance = olc::WHITE,
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, points, firstWave, damage, dimensions, color, color2, subsurfaceResistance, mass, maxHealth, health, name), explosionDimensions(explosionDimensions)
		{ }

		shared_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity * creator = nullptr) override
		{
			shared_ptr<EnemyClasses::Exploder> newEnemy = make_shared<EnemyClasses::Exploder>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}

		void OnDeath(Entity* damageDealer) override
		{
			Enemy::OnDeath(damageDealer);
			game->entities->push_back(make_shared<ExplodeNextFrame>(damage, explosionDimensions, color, pos, name));
			game->entities->push_back(make_shared<FadeOut>(1.5f, pos, explosionDimensions, color));
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
			Color subsurfaceResistance = olc::WHITE, Color color3 = olc::RED, Color color4 = olc::DARK_GREEN,
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, points, firstWave, damage, dimensions, color, color2, subsurfaceResistance, mass, maxHealth, health, name), length(length), color3(color3), color4(color4)
		{
			Start();
		}

		void Start() override
		{
			Enemy::Start();
			lastPos = pos;
		}

		shared_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			shared_ptr<Snake>* enemies = new shared_ptr<Snake>[length];
			for (int i = 0; i < length; i++)
			{
				enemies[i] = make_shared<Snake>(*this);
				enemies[i]->baseClass = baseClass;
				enemies[i]->pos = pos;
				enemies[i]->Start();
				enemies[i]->color = olc::PixelLerp(color, color4, sinf(static_cast<float>(i)) * 0.5f + 0.5f);
				enemies[i]->lastTime = tTime;
			}
			
			if (length > 1)
			{
				enemies[0]->front = enemies[1].get();
				enemies[length - 1]->back = enemies[length - 2].get();
			}
			for (int i = 1; i < length - 1; i++)
			{
				enemies[i]->back = enemies[i - 1].get();
				enemies[i]->front = enemies[i + 1].get();
			}

			for (int i = length - 1; i > 0; i--) // Back to front for loop. Does not do 0.
				game->entities->push_back(enemies[i]);
			return enemies[0]; // Do 0 here.
		}

		void DUpdate() override
		{
			if (front == nullptr)
				color = color3;
			Enemy::DUpdate();
		}

		void TUpdate() override
		{
			if (front == nullptr)
			{
				Entity* entity = nullptr;
				TryMove2(Squarmalized(playerPos - pos), mass, nullptr, &entity);
				if (entity != nullptr && !entity->IsEnemy())
				{
					entity->DealDamage(damage, this);
				}
			}
			else
			{
				vector<shared_ptr<Entity>> nearbyEnemies = game->entities->FindCorpOverlaps(pos, vOne * 2);
				for (shared_ptr<Entity> entity : nearbyEnemies)
					if (!entity->IsEnemy())
						entity->DealDamage(damage, this);
			}
			if (lastPos != pos)
			{
				if (back != nullptr)
					back->SetPos(lastPos);
				lastPos = pos;
			}
		}

		void OnDeath(Entity* damageDealer) override
		{
			Enemy::OnDeath(damageDealer);
			if (back != nullptr)
				back->front = nullptr;
			if (front != nullptr)
				front->back = nullptr;
		}
		
		int Cost() override
		{
			return points * length / 3;
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
			Enemy(timePer, points, firstWave, damage, dimensions, colorsToCycle[0], color2, olc::BLACK, mass, maxHealth, health, name),
			colorsToCycle(colorsToCycle), colorCycleSpeed(colorCycleSpeed), colorOffset(0.0f)
		{ }

		void Start() override
		{
			Enemy::Start();
			colorOffset = RandFloat() * colorCycleSpeed;
		}

		shared_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			shared_ptr<EnemyClasses::ColorCycler> newEnemy = make_shared<EnemyClasses::ColorCycler>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}

		void DUpdate() override
		{
			float currentPlace = (tTime * colorCycleSpeed + colorOffset);
			int intCurrentPlace = static_cast<int>(currentPlace);
			color = olc::PixelLerp(colorsToCycle[intCurrentPlace % colorsToCycle.size()],
				colorsToCycle[(intCurrentPlace + 1) % colorsToCycle.size()], currentPlace - floorf(currentPlace));

			Enemy::DUpdate();
		}
	};

	class Vacuumer : public Enemy
	{
	public:
		int vacDist, desiredDistance;
		Items items;

		Vacuumer(int vacDist, int desiredDistance, float timePer = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, Color color = olc::WHITE, Color color2 = olc::BLACK, Color subsurfaceResistance = olc::WHITE,
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, points, firstWave, damage, dimensions, color, color2, subsurfaceResistance, mass, maxHealth, health, name),
			vacDist(vacDist), desiredDistance(desiredDistance), items(0)
		{ }

		shared_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity * creator = nullptr) override
		{
			shared_ptr<EnemyClasses::Vacuumer> newEnemy = make_shared<EnemyClasses::Vacuumer>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}

		void TUpdate() override
		{
			float currentDistance = 0;
			if (fabsf((currentDistance = Distance(playerPos, pos)) - desiredDistance) > 1.0f)
			{
				Entity* entity = nullptr;
				TryMove2(Squarmalized((int(currentDistance > desiredDistance) * 2 - 1) * (playerPos - pos)), mass, nullptr, &entity);
				if (entity != nullptr && !entity->IsEnemy())
				{
					entity->DealDamage(damage, this);
				}
			}
			game->entities->Vacuum(pos, vacDist);
			vector<shared_ptr<Entity>> collectibles = EntitiesOverlaps(pos, dimensions, game->entities->collectibles);
			for (shared_ptr<Entity> collectible : collectibles)
			{
				items.push_back(((Collectible*)collectible.get())->baseItem);
				collectible->DestroySelf(this);
			}
		}

		void OnDeath(Entity* damageDealer) override
		{
			Enemy::OnDeath(damageDealer);
			for (Item item : items)
			{
				game->entities->push_back(make_shared<Collectible>(item, pos));
			}
		}
	};

	class Ranger : public Vacuumer
	{
	public:
		using Vacuumer::Vacuumer;

		shared_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			shared_ptr<EnemyClasses::Ranger> newEnemy = make_shared<EnemyClasses::Ranger>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}

		void TUpdate() override
		{
			Vacuumer::TUpdate();

			if (items.size() > 0)
			{
				Item shotItem = items[0].Clone(1);
				items.TryTakeIndex(0);
				game->entities->push_back(basicShotItem->Clone(shotItem, pos, (playerPos - pos) * static_cast<int>(shotItem.range), this));
			}
		}
	};

	class Spider : public Enemy
	{
	public:
		vector<LegParticle*> legs;
		vector<float> lastLegUpdates;
		LegParticle baseLeg;
		int legCount;
		float legLength, legTolerance, legCycleSpeed;
		float legRotation = 0.0f;

		Spider(LegParticle baseLeg, int legCount = 8, float legLength = 5.0f, float legTolerance = 3.0f, float legCycleSpeed = 1.0f,
			float timePer = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, Color color = olc::WHITE, Color color2 = olc::BLACK, Color subsurfaceResistance = olc::WHITE,
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, points, firstWave, damage, dimensions, color, color2, subsurfaceResistance, mass, maxHealth, health, name),
			baseLeg(baseLeg), legCount(legCount), legLength(legLength), legTolerance(legTolerance), legCycleSpeed(legCycleSpeed) { }

		shared_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			shared_ptr<EnemyClasses::Spider> newEnemy = make_shared<EnemyClasses::Spider>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}

		void Start() override
		{
			Enemy::Start();
			legRotation = RandFloat();

			legs = vector<LegParticle*>(legCount);
			lastLegUpdates = vector<float>(legCount);
			float offsettedTTime = tTime + RandFloat() * legCycleSpeed;

			for (int i = 0; i < legCount; i++)
			{
				legs[i] = new LegParticle(baseLeg);
				legs[i]->parent = this;
				legs[i]->desiredPos = LegPos(i);
				legs[i]->pos = legs[i]->desiredPos;
				lastLegUpdates[i] = offsettedTTime + i * legCycleSpeed / legCount;
			}
		}

		Vec2f LegPos(int index)
		{
			float rotation = (float(index) / legCount + legRotation) * 2 * PI_F;
			return pos + Vec2f(sinf(rotation), cosf(rotation)) * legLength;
		}

		void EarlyDUpdate() override
		{
			for (int i = 0; i < legCount; i++)
				legs[i]->LowResUpdate();
			Enemy::EarlyDUpdate();
		}

		void Update() override
		{
			Enemy::Update();
			for (int i = 0; i < legCount; i++)
			{
				if (tTime - lastLegUpdates[i] > legCycleSpeed)
				{
					lastLegUpdates[i] += legCycleSpeed;
					Vec2f desiredPos = LegPos(i) + Normalized(playerPos - pos) * legTolerance * 0.5f;
					if (Distance(legs[i]->desiredPos, desiredPos) > legTolerance)
						legs[i]->desiredPos = desiredPos;
				}
				legs[i]->Update();
			}
		}
		
		void OnDeath(Entity* damageDealer) override
		{
			Enemy::OnDeath(damageDealer);
			for (int i = 0; i < legCount; i++)
				delete legs[i];
		}
	};

	class Spoobderb : public Spider
	{
	public:
		Entity* baseChild;

		Spoobderb(Entity* baseChild, LegParticle baseLeg, int legCount = 8, float legLength = 5.0f, float legTolerance = 3.0f, float legCycleSpeed = 1.0f,
			float timePer = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, Color color = olc::WHITE, Color color2 = olc::BLACK, Color subsurfaceResistance = olc::WHITE,
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			baseChild(baseChild), Spider(baseLeg, legCount, legLength, legTolerance, legCycleSpeed,
				timePer, points, firstWave, damage, dimensions, color, color2, subsurfaceResistance, mass, maxHealth, health, name) { }

		shared_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			shared_ptr<EnemyClasses::Spoobderb> newEnemy = make_shared<EnemyClasses::Spoobderb>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}

		int DealDamage(int damage, Entity* damageDealer) override
		{
			for (int i = 0; i < damage; i++)
				game->entities->push_back(baseChild->Clone(pos - dimensions + vOne + Vec2(rand() % ((dimensions.x + 1) * 2), rand() % ((dimensions.y + 1) * 2)), up, this));
			return Spider::DealDamage(damage, damageDealer);
		}
	};
}

//Predefinitions
LegParticle* spiderLeg = new LegParticle(vZero, nullptr, Color(0, 0, 0, 150), 32.0f);
Enemy* child = new Enemy(0.125f, 0, 0, 1, vOne, olc::MAGENTA, olc::BLACK, Color(0, 50, 0), 1, 1, 1, "Child");

// Earlies.
Enemy* walker = new Enemy(0.75f, 1, 1, 1, vOne, olc::CYAN, olc::BLACK, Color(50, 0, 0), 1, 3, 3, "Walker");
Enemy* tanker = new Enemy(1.0f, 2, 1, 1, vOne * 2, olc::RED, olc::BLACK, Color(0, 25, 25), 5, 12, 12, "Tanker");
EnemyClasses::Spider* spider = new EnemyClasses::Spider(*spiderLeg, 6, 3.0f, 1.0f, 1.0f, 0.25f, 2, 1, 1, vOne, Color(79, 0, 26), olc::BLACK, olc::VERY_DARK_GREY, 1, 2, 2, "Spider");

// Mids:
EnemyClasses::Deceiver* deceiver = new EnemyClasses::Deceiver(0.5f, 4, 4, 1, vOne, olc::WHITE, olc::BLACK, Color(255, 255, 255, 153), olc::BLACK, 1, 3, 3, "Deceiver");
EnemyClasses::Exploder* exploder = new EnemyClasses::Exploder(vOne * 3, 0.25f, 4, 4, 1, vOne, Color(153, 255, 0), olc::BLACK, Color(25, 0, 25), 1, 3, 3, "Exploder");
EnemyClasses::Vacuumer* vacuumer = new EnemyClasses::Vacuumer(12, 12, 0.125f, 3, 4, 0, vOne, olc::GREY, olc::BLACK, Color(50, 50, 50), 1, 3, 3, "Vacuumer");

// Mid-lates:
EnemyClasses::Parent* parent = new EnemyClasses::Parent(child, 1.0f, 4, 6, 1, vOne * 3, olc::DARK_MAGENTA, olc::BLACK, Color(0, 50, 0), 5, 10, 10, "Parent");
EnemyClasses::Parent* spiderParent = new EnemyClasses::Parent(spider, 1.0f, 4, 6, 1, vOne * 3, Color(140, 35, 70), olc::BLACK, Color(0, 50, 0), 5, 10, 10, "Spider Parent");
EnemyClasses::Snake* snake = new EnemyClasses::Snake(30, 0.25f, 1, 6, 1, vOne, olc::GREEN, olc::BLACK, Color(50, 0, 0), olc::RED, olc::DARK_GREEN, 2, 3, 3, "Snake");

// Lates:
EnemyClasses::ColorCycler* hyperSpeedster = new EnemyClasses::ColorCycler({olc::RED, olc::YELLOW, olc::BLUE}, 2.0f, 0.25f, 8, 8, 1, vOne, olc::BLACK, 1, 24, 24, "Hyper Speedster");
Enemy* megaTanker = new Enemy(1.0f, 20, 8, 1, vOne * 3, Color(174, 0, 255), olc::BLACK, Color(0, 25, 25), 10, 48, 48, "Mega Tanker");
EnemyClasses::Exploder* gigaExploder = new EnemyClasses::Exploder(vOne * 8, 0.25f, 8, 8, 1, vOne * 2, Color(153, 255, 0), olc::BLACK, Color(25, 0, 25), 1, 3, 3, "Giga Exploder");
EnemyClasses::Ranger* ranger = new EnemyClasses::Ranger(12, 12, 0.125f, 6, 8, 0, vOne * 3, olc::GREY, olc::BLACK, Color(50, 50, 50), 1, 12, 12, "Ranger");

// Bosses:
EnemyClasses::Spoobderb* spoobderb = new EnemyClasses::Spoobderb(spider, *spiderLeg, 30, 25.0f, 3.0f, 2.5f, 0.5f, 250, 0, 1, vOne * 7, Color(77, 14, 35), olc::BLACK, olc::VERY_DARK_GREY, 50, 100, 100, "Spoobderb - The 30 footed beast");

class EnemiesInstance;
class Enemies : public vector<vector<Enemy*>>
{
public:
	using vector<vector<Enemy*>>::vector;

	static int GetRoundPoints()
	{
		return static_cast<int>(pow(1.37, waveCount)) + waveCount * 3 - 1;
	}

	EnemiesInstance RandomClone();
};

class EnemiesInstance : public vector<Enemy*>
{
public:
	using vector<Enemy*>::vector;

	void SpawnRandomEnemies()
	{
		int totalCost = 0, costToAchieve = Enemies::GetRoundPoints();
		int currentlySpawnableEnemyCount = 0;
		for (int i = 0; i < (*this).size(); i++)
			currentlySpawnableEnemyCount += int((*this)[i]->firstWave <= waveCount && (*this)[i]->Cost() <= costToAchieve);
		vector<Enemy*> currentlySpawnableEnemies(currentlySpawnableEnemyCount);
		for (int i = 0, j = 0; j < currentlySpawnableEnemyCount; i++)
			if ((*this)[i]->firstWave <= waveCount && (*this)[i]->Cost() <= costToAchieve)
				currentlySpawnableEnemies[j++] = (*this)[i];

		while (totalCost < costToAchieve)
		{
			int currentIndex = rand() % currentlySpawnableEnemyCount;
			float randomValue = RandFloat() * 6.283184f;
			game->entities->push_back(currentlySpawnableEnemies[currentIndex]->Clone(Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos));
			totalCost += currentlySpawnableEnemies[currentIndex]->Cost();
		}
	}

	void SpawnRandomEnemies(int cost)
	{
		int totalCost = 0;
		int currentlySpawnableEnemyCount = 0;
		for (int i = 0; i < (*this).size(); i++)
			currentlySpawnableEnemyCount += int((*this)[i]->firstWave <= waveCount && (*this)[i]->Cost() <= cost);
		vector<Enemy*> currentlySpawnableEnemies(currentlySpawnableEnemyCount);
		for (int i = 0, j = 0; j < currentlySpawnableEnemyCount; i++)
			if ((*this)[i]->firstWave <= waveCount && (*this)[i]->Cost() <= cost)
				currentlySpawnableEnemies[j++] = (*this)[i];

		while (totalCost < cost)
		{
			int currentIndex = rand() % currentlySpawnableEnemyCount;
			float randomValue = RandFloat() * 6.283184f;
			game->entities->push_back(currentlySpawnableEnemies[currentIndex]->Clone(Vec2f(cosf(randomValue), sinf(randomValue)) * screenDimH * 1.415f + playerPos));
			totalCost += currentlySpawnableEnemies[currentIndex]->Cost();
		}
	}
};

EnemiesInstance Enemies::RandomClone()
{
	EnemiesInstance result(size());
	for (int i = 0; i < size(); i++)
		result[i] = (*this)[i][rand() % (*this)[i].size()];
	return result;
}

Enemies spawnableEnemies{ {walker, tanker, spider}, {deceiver, exploder, vacuumer}, {parent, spiderParent, snake},
	{hyperSpeedster, megaTanker, gigaExploder, ranger} };

Enemies spawnableBosses{ {spoobderb} };