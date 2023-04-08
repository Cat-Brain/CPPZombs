#include "Defence.h"

namespace Enemies
{
#pragma region Enemy types
	// The base class of all enemies.
	class Enemy;
	enum class MUPDATE
	{
		DEFAULT, SNAKE, POUNCERSNAKE, SNAKECONNECTED, VACUUMER, CENTICRAWLER, POUNCER, CAT, TANK
	};
	vector<function<bool(Enemy*)>> mUpdates;

	enum class AUPDATE
	{
		DEFAULT, EXPLODER, BOOMCAT, TANK
	};
	vector<function<bool(Enemy*)>> aUpdates;

	class Enemy : public DToCol
	{
	public:
		MUPDATE mUpdate;
		AUPDATE aUpdate;
		float timePer, lastTime, speed, maxSpeed, timePerMove, lastMove;

		int points, firstWave;
		int damage;

		Enemy(float timePer = 0.5f, float speed = 0.25f, float maxSpeed = 0.25f, int points = 1, int firstWave = 1, int damage = 1, float radius = 0.5f,
			RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			DToCol(vZero, radius, color, color2, mass, maxHealth, health, name),
			timePer(timePer), lastTime(0.0f), speed(speed), maxSpeed(maxSpeed), timePerMove(0.0f), lastMove(0.0f), points(points), firstWave(firstWave),
			damage(damage), mUpdate(MUPDATE::DEFAULT), aUpdate(AUPDATE::DEFAULT)
		{
			update = UPDATE::ENEMY;
			uiUpdate = UIUPDATE::ENEMY;
			onDeath = ONDEATH::ENEMY;
			isEnemy = true;
		}

		Enemy(Enemy* baseClass, Vec3 pos) :
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

		unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) override
		{
			return make_unique<Enemy>(this, pos);
		}

		bool MUpdate()
		{
			return mUpdates[UnEnum(mUpdate)](this);
		}
		bool MUpdate(MUPDATE tempMUpdate)
		{
			return mUpdates[UnEnum(tempMUpdate)](this);
		}

		bool AUpdate()
		{
			return aUpdates[UnEnum(aUpdate)](this);
		}
		bool AUpdate(AUPDATE tempAUpdate)
		{
			return aUpdates[UnEnum(tempAUpdate)](this);
		}

		virtual int Cost()
		{
			return points;
		}
	};


	class Deceiver : public Enemy
	{
	public:
		RGBA color3;
		FastNoiseLite noise1, noise2, noise3; // <-For random colors.

		Deceiver(float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, int points = 1, int firstWave = 1, int damage = 1, float radius = 0.5f,
			RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, speed, maxSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name), color3(color3), noise1(), noise2(), noise3()
		{
			Start();
			dUpdate = DUPDATE::DECEIVER;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Deceiver> newEnemy = make_unique<Deceiver>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			newEnemy->noise1.SetSeed(PsuedoRandom());
			newEnemy->noise2.SetSeed(PsuedoRandom());
			newEnemy->noise3.SetSeed(PsuedoRandom());
			return std::move(newEnemy);
		}
	};

	class Parent : public Enemy
	{
	public:
		Enemy* child;

		Parent(Enemy* child, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, speed, maxSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name), child(child)
		{
			Start();
			dUpdate = DUPDATE::PARENT;
			onDeath = ONDEATH::PARENT;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Parent> newEnemy = make_unique<Parent>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
		}
	};

	class Exploder : public Enemy
	{
	public:
		float explosionRadius;

		Exploder(float explosionRadius, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, speed, maxSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name), explosionRadius(explosionRadius)
		{
			dUpdate = DUPDATE::EXPLODER;
			aUpdate = AUPDATE::EXPLODER;
			onDeath = ONDEATH::EXPLODER;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Exploder> newEnemy = make_unique<Exploder>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}
	};

	class Snake : public Enemy
	{
	public:
		RGBA color3, color4;
		Snake* back = nullptr, * front = nullptr;
		int length;
		float segmentWobbleForce, segmentWobbleFrequency;

		Snake(int length, float segmentWobbleForce, float segmentWobbleFrequency, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, speed, maxSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name), length(length), color3(color3),
			color4(color4), segmentWobbleForce(segmentWobbleForce), segmentWobbleFrequency(segmentWobbleFrequency)
		{
			earlyDUpdate = EDUPDATE::SNAKE;
			onDeath = ONDEATH::SNAKE;
			mUpdate = MUPDATE::SNAKE;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Snake>* enemies = new unique_ptr<Snake>[length];
			for (int i = 0; i < length; i++)
			{
				enemies[i] = make_unique<Snake>(*this);
				enemies[i]->baseClass = baseClass;
				enemies[i]->pos = pos + RandCircPoint();
				enemies[i]->points = points / length;
				enemies[i]->Start();
				enemies[i]->radius = radius + segmentWobbleForce * sinf(static_cast<float>(i) * segmentWobbleFrequency);
				enemies[i]->color = color.CLerp(color4, sinf(static_cast<float>(i)) * 0.5f + 0.5f);
				enemies[i]->lastTime = tTime;
			}
			if (length > 1)
			{
				enemies[0]->front = enemies[1].get();
				enemies[length - 1]->back = enemies[length - 2].get();
				enemies[length - 1]->color = color3;
			}
			for (int i = 1; i < length - 1; i++)
			{
				enemies[i]->back = enemies[i - 1].get();
				enemies[i]->front = enemies[i + 1].get();
			}

			for (int i = length - 1; i > 0; i--) // Back to front for loop. Does not do 0.
				game->entities->push_back(std::move(enemies[i]));
			return std::move(enemies[0]); // Do 0 here.
		}

		int Cost() override
		{
			return points / 5;
		}
	};

	class PouncerSnake : public Snake
	{
	public:
		float pounceTime;

		PouncerSnake(float pounceTime, float speed, int length, float segmentWobbleForce, float segmentWobbleFrequency, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Snake(length, segmentWobbleForce, segmentWobbleFrequency, timePer, speed, speed, points, firstWave, damage, radius, color, color2, color3, color4, mass, maxHealth, health, name),
			pounceTime(pounceTime)
		{
			this->timePerMove = timePerMove;
			update = UPDATE::POUNCERSNAKE;
			vUpdate = VUPDATE::ENTITY;
			onDeath = ONDEATH::POUNCERSNAKE;
			mUpdate = MUPDATE::POUNCERSNAKE;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<PouncerSnake>* enemies = new unique_ptr<PouncerSnake>[length];
			for (int i = 0; i < length; i++)
			{
				enemies[i] = make_unique<PouncerSnake>(*this);
				enemies[i]->baseClass = baseClass;
				enemies[i]->pos = pos + RandCircPoint();
				enemies[i]->points = points / length;
				enemies[i]->Start();
				enemies[i]->radius = radius + segmentWobbleForce * sinf(static_cast<float>(i) * segmentWobbleFrequency);
				enemies[i]->color = color.CLerp(color4, sinf(static_cast<float>(i)) * 0.5f + 0.5f);
				enemies[i]->lastTime = tTime;
			}

			if (length > 1)
			{
				enemies[0]->front = enemies[1].get();
				enemies[length - 1]->back = enemies[length - 2].get();
				enemies[length - 1]->color = color3;
			}
			for (int i = 1; i < length - 1; i++)
			{
				enemies[i]->back = enemies[i - 1].get();
				enemies[i]->front = enemies[i + 1].get();
			}

			for (int i = length - 1; i > 0; i--) // Back to front for loop. Does not do 0.
				game->entities->push_back(std::move(enemies[i]));
			return std::move(enemies[0]); // Do 0 here.
		}
	};

	class SnakeConnected : public Enemy
	{
	public:
		RGBA color3, color4;
		SnakeConnected* front = nullptr, *next = nullptr;
		int length;
		float segmentWobbleForce, segmentWobbleFrequency;

		SnakeConnected(int length, float segmentWobbleForce, float segmentWobbleFrequency, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, speed, maxSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name), length(length), color3(color3),
			color4(color4), segmentWobbleForce(segmentWobbleForce), segmentWobbleFrequency(segmentWobbleFrequency)
		{
			dUpdate = DUPDATE::SNAKECONNECTED;
			earlyDUpdate = EDUPDATE::SNAKECONNECTED;
			uiUpdate = UIUPDATE::SNAKECONNECTED;
			onDeath = ONDEATH::SNAKECONNECTED;
			mUpdate = MUPDATE::SNAKECONNECTED;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<SnakeConnected>* enemies = new unique_ptr<SnakeConnected>[length];
			for (int i = 0; i < length; i++)
			{
				enemies[i] = make_unique<SnakeConnected>(*this);
				enemies[i]->baseClass = baseClass;
				enemies[i]->pos = pos + RandCircPoint();
				enemies[i]->Start();
				enemies[i]->radius = radius + segmentWobbleForce * sinf(static_cast<float>(i) * segmentWobbleFrequency);
				enemies[i]->color = color.CLerp(color4, sinf(static_cast<float>(i)) * 0.5f + 0.5f);
				enemies[i]->lastTime = tTime;
			}
			enemies[length - 1]->color = color3;
			enemies[length - 1]->front = enemies[length - 1].get();
			enemies[length - 1]->next = nullptr;
			enemies[length - 1]->observers.resize(length - 1);
			for (int i = 0; i < length - 1; i++)
			{
				enemies[i]->front = enemies[length - 1].get();
				enemies[length - 1]->observers[i] = enemies[i].get();
				enemies[i]->next = enemies[i + 1].get();
			}

			for (int i = length - 1; i > 0; i--) // Back to front for loop. Does not do 0.
				game->entities->push_back(std::move(enemies[i]));
			return std::move(enemies[0]); // Do 0 here.
		}

		void UnAttach(Entity* entity) override
		{
			printf("!");
			if (entity == front)
				DestroySelf(nullptr);
		}

		int DealDamage(int damage, Entity* damageDealer) override
		{
			if (next == nullptr)
				return Enemy::DealDamage(damage, damageDealer);
			return front->Enemy::DealDamage(damage, damageDealer);
		}
	};

	class ColorCycler : public Enemy
	{
	public:
		vector<RGBA> colorsToCycle;
		float colorOffset, colorCycleSpeed;

		ColorCycler(vector<RGBA> colorsToCycle, float colorCycleSpeed = 1.0f, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, int points = 1,
			int firstWave = 1, int damage = 1, float radius = 0.5f, RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, speed, maxSpeed, points, firstWave, damage, radius, colorsToCycle[0], color2, mass, maxHealth, health, name),
			colorsToCycle(colorsToCycle), colorCycleSpeed(colorCycleSpeed), colorOffset(0.0f)
		{
			dUpdate = DUPDATE::COLORCYCLER;
		}

		void Start() override
		{
			Enemy::Start();
			colorOffset = RandFloat() * colorCycleSpeed;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<ColorCycler> newEnemy = make_unique<ColorCycler>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}
	};

	class Vacuumer : public Enemy
	{
	public:
		float vacDist, vacSpeed, maxVacSpeed, desiredDistance;
		Items items;

		Vacuumer(float vacDist, float vacSpeed, float maxVacSpeed, float desiredDistance, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, int points = 1,
			int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, speed, maxSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name),
			vacDist(vacDist), vacSpeed(vacSpeed), maxVacSpeed(maxVacSpeed), desiredDistance(desiredDistance), items(0)
		{
			update = UPDATE::VACUUMER;
			onDeath = ONDEATH::VACUUMER;
			mUpdate = MUPDATE::VACUUMER;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Vacuumer> newEnemy = make_unique<Vacuumer>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
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
			float timePer = 0.5f, float moveSpeed = 2.0f, float maxSpeed = 0.25f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, moveSpeed, maxSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name),
			baseLeg(baseLeg), legCount(legCount), legLength(legLength), legTolerance(legTolerance), legCycleSpeed(legCycleSpeed)
		{
			onDeath = ONDEATH::SPIDER;
			update = UPDATE::SPIDER;
			earlyDUpdate = EDUPDATE::SPIDER;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Spider> newEnemy = make_unique<Spider>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
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

		Vec3 LegPos(int index)
		{
			float rotation = (float(index) / legCount + legRotation) * 2 * PI_F;
			return pos + Vec3(CircPoint2(rotation), 0) * legLength;
		}

		void UpdateLegs()
		{
			for (int i = 0; i < legCount; i++)
			{
				if (tTime - lastLegUpdates[i] > legCycleSpeed)
				{
					lastLegUpdates[i] += legCycleSpeed;
					Vec3 desiredPos = LegPos(i) + Normalized(game->PlayerPos() - pos) * legTolerance * 0.5f;
					if (glm::distance2(legs[i]->desiredPos, desiredPos) > legTolerance * legTolerance)
						legs[i]->desiredPos = desiredPos;
				}
				legs[i]->Update();
			}
		}
	};

	class Spoobderb : public Spider
	{
	public:
		Entity* baseChild;

		Spoobderb(Entity* baseChild, LegParticle baseLeg, int legCount = 8, float legLength = 5.0f, float legTolerance = 3.0f, float legCycleSpeed = 1.0f,
			float timePer = 0.5f, float moveSpeed = 2.0f, float maxSpeed = 0.25f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			baseChild(baseChild), Spider(baseLeg, legCount, legLength, legTolerance, legCycleSpeed,
				timePer, moveSpeed, maxSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name) { }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Spoobderb> newEnemy = make_unique<Spoobderb>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
		}

		int DealDamage(int damage, Entity* damageDealer) override
		{
			for (int i = 0; i < damage; i++)
				game->entities->push_back(baseChild->Clone(pos - Vec3(radius + 1) + Vec3(RandFloat() * ((radius + 1) * 2), RandFloat() * ((radius + 1) * 2), RandFloat() * ((radius + 1) * 2)), up, this));
			return Spider::DealDamage(damage, damageDealer);
		}
	};

	class Centicrawler : public Spider
	{
	public:
		Centicrawler* back = nullptr, * front = nullptr;
		float segmentWobbleForce, segmentWobbleFrequency;

		Centicrawler(float segmentWobbleForce, float segmentWobbleFrequency, LegParticle baseLeg, int legCount = 8, float legLength = 5.0f, float legTolerance = 3.0f, float legCycleSpeed = 1.0f,
			float timePer = 0.5f, float moveSpeed = 2.0f, float maxSpeed = 0.25f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Spider(baseLeg, legCount, legLength, legTolerance, legCycleSpeed, timePer, moveSpeed, maxSpeed, points, firstWave, damage, radius, color, color2,
				mass, maxHealth, health, name), segmentWobbleForce(segmentWobbleForce), segmentWobbleFrequency(segmentWobbleFrequency)
		{
			update = UPDATE::CENTICRAWLER;
			onDeath = ONDEATH::CENTICRAWLER;
			mUpdate = MUPDATE::CENTICRAWLER;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Centicrawler> newEnemy = make_unique<Centicrawler>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->back = nullptr;
			newEnemy->front = nullptr;
			newEnemy->Start();
			newEnemy->radius = radius + segmentWobbleForce * sinf(rand() * segmentWobbleFrequency);
			return std::move(newEnemy);
		}
	};

	class Pouncer : public Enemy
	{
	public:
		float pounceTime;

		Pouncer(float pounceTime, float moveSpeed = 2, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, moveSpeed, moveSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name),
			pounceTime(pounceTime)
		{
			this->timePerMove = timePerMove;
			update = UPDATE::POUNCER;
			vUpdate = VUPDATE::ENTITY;
			dUpdate = DUPDATE::POUNCER;
			mUpdate = MUPDATE::POUNCER;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Pouncer> newEnemy = make_unique<Pouncer>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = Normalized(game->PlayerPos() - newEnemy->pos);
			newEnemy->Start();
			return std::move(newEnemy);
		}
	};

	class Cat : public Pouncer
	{
	public:
		float homeSpeed;
		RGBA color3;

		Cat(float homeSpeed, float pounceTime, float moveSpeed = 2, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Pouncer(pounceTime, moveSpeed, timePer, timePerMove, points, firstWave, damage, radius, color, color2, mass, maxHealth, health,
				name), homeSpeed(homeSpeed), color3(color3)
		{
			update = UPDATE::CAT;
			dUpdate = DUPDATE::CAT;
			mUpdate = MUPDATE::CAT;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Cat> newEnemy = make_unique<Cat>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = Normalized(game->PlayerPos() - newEnemy->pos);
			newEnemy->Start();
			return std::move(newEnemy);
		}

		int DealDamage(int damage, Entity* damageDealer) override
		{
			return Enemy::DealDamage(Clamp(damage, -1, 1), damageDealer);
		}
	};

	class BoomCat : public Cat
	{
	public:
		float explosionRadius;

		BoomCat(float explosionRadius, float homeSpeed, float pounceTime, float moveSpeed = 2, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Cat(homeSpeed, pounceTime, moveSpeed, timePer, timePerMove, points, firstWave, damage, radius, color, color2, color3, mass, maxHealth, health, name), explosionRadius(explosionRadius)
		{
			aUpdate = AUPDATE::BOOMCAT;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<BoomCat> newEnemy = make_unique<BoomCat>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = Normalized(game->PlayerPos() - newEnemy->pos);
			newEnemy->Start();
			return std::move(newEnemy);
		}
	};

	class Cataclysm : public BoomCat
	{
	public:
		float lastStartedCircle = -100, circleTime, circleRadius, spinSpeed;
		float lastShoot = -100, timePerShot;
		Projectile* projectile, *projectile2;
		RGBA color4;

		Cataclysm(float circleTime, float circleRadius, float spinSpeed, Projectile* projectile, Projectile* projectile2, float timePerShot, float explosionRadius,
			float homeSpeed, float pounceTime, float moveSpeed, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			BoomCat(explosionRadius, homeSpeed, pounceTime, moveSpeed, timePer, timePerMove, points, firstWave, damage, radius, color, color2, color3,
				mass, maxHealth, health, name), circleTime(circleTime), circleRadius(circleRadius), spinSpeed(spinSpeed), projectile(projectile),
			projectile2(projectile2), timePerShot(timePerShot), color4(color4)
		{
			update = UPDATE::CATACLYSM;
			dUpdate = DUPDATE::CATACLYSM;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Cataclysm> newEnemy = make_unique<Cataclysm>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = Normalized(game->PlayerPos() - newEnemy->pos);
			newEnemy->Start();
			return std::move(newEnemy);
		}

		int DealDamage(int damage, Entity* damageDealer) override
		{
			if (tTime - lastStartedCircle > circleTime / difficultyGrowthModifier[game->difficulty])
			{
				if (damage > 0)
				{
					lastStartedCircle = tTime;
				}
				return Cat::DealDamage(damage, damageDealer);
			}
			return 0;
		}
	};

	class Tank : public Enemy
	{
	public:
		Projectile* projectile;
		float turnSpeed;

		Tank(Projectile* projectile, float turnSpeed, float timePer = 0.5f, float moveSpeed = 0.5f, float maxSpeed = 0.25f, int points = 1,
			int firstWave = 1, int damage = 1, float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, moveSpeed, maxSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name),
			projectile(projectile), turnSpeed(turnSpeed)
		{
			dUpdate = DUPDATE::TANK;
			mUpdate = MUPDATE::TANK;
			aUpdate = AUPDATE::TANK;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Tank> newEnemy = make_unique<Tank>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = Normalized(game->PlayerPos() - newEnemy->pos);
			newEnemy->Start();
			return newEnemy;
		}
	};

	namespace Updates
	{
		void EnemyU(Entity* entity)
		{
			Enemy* enemy = static_cast<Enemy*>(entity);
			if (tTime - enemy->lastMove >= enemy->timePerMove)
				if (enemy->MUpdate())
					enemy->lastMove = tTime;

			if (tTime - enemy->lastTime >= enemy->timePer)
				if (enemy->AUpdate())
					enemy->lastTime = tTime;
		}

		void PouncerSnakeU(Entity* entity)
		{
			PouncerSnake* pSnake = static_cast<PouncerSnake*>(entity);
			pSnake->Update(UPDATE::ENEMY);
			if (pSnake->front == nullptr && tTime - pSnake->lastMove > pSnake->pounceTime && tTime - game->dTime - pSnake->lastMove < pSnake->pounceTime)
				pSnake->vUpdate = VUPDATE::FRICTION;
		}

		void VacuumerU(Entity* entity)
		{
			Vacuumer* vacuumer = static_cast<Vacuumer*>(entity);
			vacuumer->Update(UPDATE::ENEMY);

			game->entities->Vacuum(vacuumer->pos, vacuumer->vacDist, vacuumer->vacSpeed, vacuumer->maxVacSpeed, true);

			vector<Entity*> collectibles = EntitiesOverlaps(vacuumer->pos, vacuumer->radius, game->entities->collectibles);
			for (Entity* collectible : collectibles)
			{
				vacuumer->items.push_back(((Collectible*)collectible)->baseItem);
				collectible->DestroySelf(vacuumer);
			}
		}

		void SpiderU(Entity* entity)
		{
			Spider* spider = static_cast<Spider*>(entity);

			spider->Update(UPDATE::ENEMY);
		}

		void CenticrawlerU(Entity* entity)
		{
			Centicrawler* centicrawler = static_cast<Centicrawler*>(entity);

			centicrawler->Update(UPDATE::SPIDER);
			if (centicrawler->front == nullptr)
			{
				Centicrawler* farthestBack = centicrawler;
				while (farthestBack->back != nullptr)
					farthestBack = farthestBack->back;
				Entity* hitEntity = game->entities->FirstOverlap(centicrawler->pos, centicrawler->radius + 0.5f, MaskF::IsSameType, centicrawler);
				Centicrawler* currentCenticrawler = static_cast<Centicrawler*>(hitEntity);
				if (currentCenticrawler != centicrawler && currentCenticrawler->baseClass == centicrawler->baseClass && currentCenticrawler->back == nullptr && currentCenticrawler != farthestBack)
				{
					currentCenticrawler->back = centicrawler;
					centicrawler->front = currentCenticrawler;
					return;
				}
			}
		}

		void PouncerU(Entity* entity)
		{
			Pouncer* pouncer = static_cast<Pouncer*>(entity);

			pouncer->Update(UPDATE::ENEMY);

			if (tTime - pouncer->lastMove > pouncer->pounceTime && tTime - game->dTime - pouncer->lastMove < pouncer->pounceTime)
				pouncer->vUpdate = VUPDATE::FRICTION;
		}

		void CatU(Entity* entity)
		{
			Cat* cat = static_cast<Cat*>(entity);

			cat->dir = RotateTowardsNorm(cat->vel != vZero && glm::normalize(cat->vel) != cat->dir ? cat->vel : cat->dir, game->PlayerPos() - cat->pos, cat->homeSpeed * game->dTime);
			cat->vel = cat->dir * glm::length(cat->vel);

			cat->Update(UPDATE::POUNCER);
		}

		void CataclysmU(Entity* entity)
		{
			Cataclysm* cat = static_cast<Cataclysm*>(entity);

			if (tTime - cat->lastShoot > cat->timePerShot && tTime - cat->lastStartedCircle >= cat->circleTime && tTime - cat->lastStartedCircle < cat->circleTime / difficultyGrowthModifier[game->difficulty])
			{
				game->entities->push_back(cat->projectile2->Clone(cat->pos, RotateBy(game->PlayerPos() - cat->pos, PI_F * 0.125f), cat));
				game->entities->push_back(cat->projectile2->Clone(cat->pos, RotateBy(game->PlayerPos() - cat->pos, 0), cat));
				game->entities->push_back(cat->projectile2->Clone(cat->pos, RotateBy(game->PlayerPos() - cat->pos, PI_F * -0.125f), cat));
			}
			
			if (tTime - cat->lastStartedCircle < cat->circleTime)
			{
				float rotation = (tTime - cat->lastStartedCircle) * cat->spinSpeed;
				cat->SetPos(game->PlayerPos() + Vec3(sinf(rotation) * cat->circleRadius, cosf(rotation) * cat->circleRadius, 0));
			
				if (tTime - cat->lastShoot > cat->timePerShot)
				{
					cat->lastShoot = tTime;
					int count = cat->maxHealth - cat->health + 1;
					Vec3 pos = cat->pos;

					for (int i = 1; i < count; i++)
					{
						pos = Vec3(glm::rotate(Vec2(pos - game->PlayerPos()), PI_F * 2 / count), pos.z) + game->PlayerPos();
						game->entities->push_back(cat->projectile->Clone(pos, game->PlayerPos() - pos, cat));
					}

					game->entities->push_back(cat->projectile->Clone(cat->pos, game->PlayerPos() - cat->pos, cat));
				}

				cat->dir = Normalized(game->PlayerPos() - cat->pos);
				cat->vel = cat->dir * cat->speed;
			}
			else cat->Update(UPDATE::CAT);
		}
	}

	namespace DUpdates
	{
		void DeceiverDU(Entity* entity)
		{
			Deceiver* deceiver = static_cast<Deceiver*>(entity);
			float r = deceiver->noise1.GetNoise(tTime, 0.0f) * 0.5f + 0.5f;
			float g = deceiver->noise2.GetNoise(tTime, 0.0f) * 0.5f + 0.5f;
			float b = deceiver->noise3.GetNoise(tTime, 0.0f) * 0.5f + 0.5f;
			deceiver->color.r = static_cast<byte>(r * 255);
			deceiver->color.g = static_cast<byte>(g * 255);
			deceiver->color.b = static_cast<byte>(b * 255);

			deceiver->DUpdate(DUPDATE::DTOCOL);

			float healthRatio = (float)deceiver->health / deceiver->maxHealth;
			deceiver->color.r = static_cast<byte>(r * deceiver->color3.r);
			deceiver->color.g = static_cast<byte>(g * deceiver->color3.g);
			deceiver->color.b = static_cast<byte>(b * deceiver->color3.b);

			Vec3 tempPos = deceiver->pos;

			deceiver->pos = Vec3(game->PlayerPos().x * 2 - deceiver->pos.x, deceiver->pos.y, deceiver->pos.z);
			deceiver->DUpdate(DUPDATE::DTOCOL);
			deceiver->pos = Vec3(deceiver->pos.x, game->PlayerPos().y * 2 - deceiver->pos.y, deceiver->pos.z);
			deceiver->DUpdate(DUPDATE::DTOCOL);
			deceiver->pos = Vec3(game->PlayerPos().x * 2 - deceiver->pos.x, deceiver->pos.y, deceiver->pos.z);
			deceiver->DUpdate(DUPDATE::DTOCOL);

			deceiver->pos = tempPos;
		}

		void ParentDU(Entity* entity)
		{
			Parent* parent = static_cast<Parent*>(entity);
			parent->DUpdate(DUPDATE::DTOCOL);
			parent->child->Draw(parent->pos + up);
			parent->child->Draw(parent->pos + left);
			parent->child->Draw(parent->pos + down);
			parent->child->Draw(parent->pos + right);
		}

		void ExploderDU(Entity* entity)
		{
			Exploder* exploder = static_cast<Exploder*>(entity);
			float tempRadius = exploder->radius;
			exploder->radius = exploder->explosionRadius;
			byte tempAlpha = exploder->color.a;
			exploder->color.a /= 5;
			exploder->DUpdate(DUPDATE::DTOCOL);
			exploder->radius = tempRadius;
			exploder->color.a = tempAlpha;
			exploder->DUpdate(DUPDATE::DTOCOL);
		}

		void SnakeConnectedDU(Entity* entity)
		{
			SnakeConnected* snake = static_cast<SnakeConnected*>(entity);
			int tempHealth = snake->health;
			snake->health = snake->front->health;
			snake->DUpdate(DUPDATE::DTOCOL);
			snake->health = tempHealth;
		}

		void ColorCyclerDU(Entity* entity)
		{
			ColorCycler* colorCycler = static_cast<ColorCycler*>(entity);
			float currentPlace = (tTime * colorCycler->colorCycleSpeed + colorCycler->colorOffset);
			int intCurrentPlace = static_cast<int>(currentPlace);
			colorCycler->color = colorCycler->colorsToCycle[intCurrentPlace % colorCycler->colorsToCycle.size()].CLerp(
				colorCycler->colorsToCycle[(static_cast<size_t>(intCurrentPlace) + 1) % colorCycler->colorsToCycle.size()], currentPlace - floorf(currentPlace));

			colorCycler->DUpdate(DUPDATE::DTOCOL);
		}

		void PouncerDU(Entity* entity)
		{
			Pouncer* pouncer = static_cast<Pouncer*>(entity);

			float ratio = pouncer->radius / SQRTTWO_F;
			game->DrawRightTri(pouncer->pos + pouncer->dir * ratio, vOne * (ratio * 2),
				atan2f(pouncer->dir.y, pouncer->dir.x) - PI_F * 0.5f, pouncer->color);

			pouncer->DUpdate(DUPDATE::DTOCOL);
		}

		void CatDU(Entity* entity)
		{
			Cat* cat = static_cast<Cat*>(entity);

			int count = cat->maxHealth - cat->health + 1;

			Vec3 tempPos = cat->pos;
			RGBA tempColor = cat->color;
			cat->color = cat->color3;

			for (int i = 1; i < count; i++)
			{
				cat->pos = Vec3(glm::rotate(Vec2(cat->pos) - Vec2(game->PlayerPos()), PI_F * 2 / count), cat->pos.z) + game->PlayerPos();
				cat->DUpdate(DUPDATE::ENTITY);
			}

			cat->color = tempColor;
			cat->pos = tempPos;
			cat->DUpdate(DUPDATE::POUNCER);
		}

		void CataclysmDU(Entity* entity)
		{
			Cataclysm* cat = static_cast<Cataclysm*>(entity);
			RGBA tempColor = cat->color;
			if (tTime - cat->lastStartedCircle < cat->circleTime / difficultyGrowthModifier[game->difficulty])
				cat->color = cat->color4.CLerp(cat->color, sinf(tTime * 4 * PI_F) * 0.5f + 0.5f);
			cat->DUpdate(DUPDATE::CAT);
			cat->color = tempColor;
		}

		void TankDU(Entity* entity)
		{
			Tank* tank = static_cast<Tank*>(entity);
			tank->DUpdate(DUPDATE::DTOCOL);
			Vec3 rightDir = RotateRight(tank->dir) * tank->radius, upDir = tank->dir * tank->radius;
			float treadRadius = tank->radius * 0.33333f;
			game->DrawCircle(tank->pos + rightDir + tank->dir * -0.33333f, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos + rightDir + tank->dir * -0.16666f, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos + rightDir, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos + rightDir + tank->dir * 0.16666f, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos + rightDir + tank->dir * 0.33333f, tank->projectile->color, treadRadius);

			game->DrawCircle(tank->pos - rightDir + tank->dir * -0.33333f, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos - rightDir + tank->dir * -0.16666f, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos - rightDir, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos - rightDir + tank->dir * 0.16666f, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos - rightDir + tank->dir * 0.33333f, tank->projectile->color, treadRadius);

			game->DrawCircle(tank->pos + upDir * 1.5f, tank->projectile->color, treadRadius);
			game->DrawLine(tank->pos, tank->pos + upDir * 1.5f, tank->projectile->color, treadRadius);
		}
	}

	namespace EDUpdates
	{
		void SnakeEDU(Entity* entity)
		{
			Snake* snake = static_cast<Snake*>(entity);

			if (snake->front == nullptr)
			{
				Snake* currentBack = snake->back;
				while (currentBack)
				{
					float dist = glm::distance(currentBack->front->pos, currentBack->pos);
					currentBack->SetPos(currentBack->pos + (currentBack->front->pos - currentBack->pos) / dist * (dist - currentBack->radius - currentBack->front->radius));
					currentBack = currentBack->back;
				}
			}
		}

		void SnakeConnectedEDU(Entity* entity)
		{
			SnakeConnected* snake = static_cast<SnakeConnected*>(entity);

			if (snake->next != nullptr)
			{
				float dist = glm::distance(snake->next->pos, snake->pos);
				snake->SetPos(snake->pos + (snake->next->pos - snake->pos) / dist * (dist - snake->radius - snake->next->radius));
			}
		}

		void SpiderEDU(Entity* entity)
		{
			Spider* spider = static_cast<Spider*>(entity);

			spider->UpdateLegs();
		}
	}

	namespace UIUpdates
	{
		void EnemyUIU(Entity* entity)
		{
			string health = to_string(entity->health);
			Vec2 bottomLeft = entity->BottomLeft();
			Vec2 topRight = bottomLeft + Vec2(font.TextWidth(entity->name + " " + health + "hp") * COMMON_TEXT_SCALE / font.minimumSize, font.maxVertOffset / 2) / 2.f;
			entity->DrawUIBox(bottomLeft, topRight, static_cast<float>(COMMON_BOARDER_WIDTH), entity->name + " " + health + "hp", entity->color);
		}

		void SnakeConnectedUIU(Entity* entity)
		{
			SnakeConnected* snake = static_cast<SnakeConnected*>(entity);
			snake->health = snake->front->health;
			snake->UIUpdate(UIUPDATE::ENEMY);
		}
	}

	namespace OnDeaths
	{
		void EnemyOD(Entity* entity, Entity* damageDealer)
		{
			totalGamePoints += static_cast<Enemy*>(entity)->points;
		}

		void ParentOD(Entity* entity, Entity* damageDealer)
		{
			Parent* parent = static_cast<Parent*>(entity);
			parent->OnDeath(ONDEATH::ENEMY, damageDealer);

			game->entities->push_back(parent->child->Clone(parent->pos + up));
			game->entities->push_back(parent->child->Clone(parent->pos + right));
			game->entities->push_back(parent->child->Clone(parent->pos + down));
			game->entities->push_back(parent->child->Clone(parent->pos + left));
		}

		void ExploderOD(Entity* entity, Entity* damageDealer)
		{
			Exploder* exploder = static_cast<Exploder*>(entity);
			exploder->OnDeath(ONDEATH::ENEMY, damageDealer);
			CreateExplosion(exploder->pos, exploder->explosionRadius, exploder->color, exploder->name, 0, exploder->damage, exploder);
		}

		void SnakeOD(Entity* entity, Entity* damageDealer)
		{
			Snake* snake = static_cast<Snake*>(entity);
			snake->OnDeath(ONDEATH::ENEMY, damageDealer);
			if (snake->back != nullptr)
			{
				snake->back->front = nullptr;
				snake->back->color = snake->color3;
			}
			if (snake->front != nullptr)
				snake->front->back = nullptr;
		}

		void PouncerSnakeOD(Entity* entity, Entity* damageDealer)
		{
			PouncerSnake* pSnake = static_cast<PouncerSnake*>(entity);
			pSnake->OnDeath(ONDEATH::SNAKE, damageDealer);
			if (pSnake->back != nullptr)
				((PouncerSnake*)pSnake->back)->dir = pSnake->dir;
		}

		void SnakeConnectedOD(Entity* entity, Entity* damageDealer)
		{
			SnakeConnected* snake = static_cast<SnakeConnected*>(entity);
			if (snake->next == nullptr)
				snake->OnDeath(ONDEATH::ENEMY, damageDealer);
		}

		void VacuumerOD(Entity* entity, Entity* damageDealer)
		{
			Vacuumer* vacuumer = static_cast<Vacuumer*>(entity);
			vacuumer->OnDeath(ONDEATH::ENEMY, damageDealer);
			for (Item item : vacuumer->items)
			{
				game->entities->push_back(make_unique<Collectible>(item, vacuumer->pos));
			}
		}

		void SpiderOD(Entity* entity, Entity* damageDealer)
		{
			Spider* spider = static_cast<Spider*>(entity);
			spider->OnDeath(ONDEATH::ENEMY, damageDealer);
			for (int i = 0; i < spider->legCount; i++)
				delete spider->legs[i];
		}

		void CenticrawlerOD(Entity* entity, Entity* damageDealer)
		{
			Centicrawler* centicrawler = static_cast<Centicrawler*>(entity);
			centicrawler->OnDeath(ONDEATH::SPIDER, damageDealer);
			if (centicrawler->back != nullptr)
				centicrawler->back->front = nullptr;
			if (centicrawler->front != nullptr)
				centicrawler->front->back = nullptr;
		}
	}

	namespace MUpdates
	{
		bool DefaultMU(Enemy* enemy)
		{
			enemy->vel = TryAdd2(enemy->vel, Normalized(game->PlayerPos() - enemy->pos) * game->dTime * (enemy->speed + game->planet->friction), enemy->maxSpeed);
			return false;
		}
	
		bool SnakeMU(Enemy* enemy)
		{
			Snake* snake = static_cast<Snake*>(enemy);
			if (snake->front == nullptr)
				snake->vel = TryAdd2(snake->vel, Normalized(game->PlayerPos() - snake->pos) * game->dTime * (snake->speed + game->planet->friction),
					snake->maxSpeed);
			return false;
		}

		bool PouncerSnakeMU(Enemy* enemy)
		{
			PouncerSnake* pSnake = static_cast<PouncerSnake*>(enemy);
			pSnake->dir = Normalized(game->PlayerPos() - pSnake->pos);
			if (pSnake->front == nullptr)
			{
				pSnake->vel = pSnake->dir * pSnake->speed;
				pSnake->vUpdate = VUPDATE::ENTITY;
			}
			return true;
		}

		bool SnakeConnectedMU(Enemy* enemy)
		{
			SnakeConnected* snake = static_cast<SnakeConnected*>(enemy);
			if (snake->next == nullptr)
				snake->vel = TryAdd2(snake->vel, Normalized(game->PlayerPos() - snake->pos) * game->dTime * (snake->speed + game->planet->friction),
					snake->maxSpeed);
			return false;
		}

		bool VacuumerMU(Enemy* enemy)
		{
			Vacuumer* vacuumer = static_cast<Vacuumer*>(enemy);
			float distance = glm::distance(game->PlayerPos(), vacuumer->pos);
			vacuumer->vel = TryAdd2(vacuumer->vel, (game->PlayerPos() - vacuumer->pos) * (game->dTime * (vacuumer->speed + game->planet->friction) *
				(float(distance > vacuumer->desiredDistance) * 2 - 1) / distance), vacuumer->maxSpeed);
			return false;
		}

		bool CenticrawlerMU(Enemy* enemy)
		{
			Centicrawler* centicrawler = static_cast<Centicrawler*>(enemy);
			if (centicrawler->front == nullptr)
				centicrawler->MUpdate(MUPDATE::DEFAULT);
			else
			{
				float dist = glm::distance(centicrawler->front->pos, centicrawler->pos);
				centicrawler->SetPos(centicrawler->pos + (centicrawler->front->pos - centicrawler->pos) / dist * (dist - centicrawler->radius - centicrawler->front->radius));
			}
			return false;
		}

		bool PouncerMU(Enemy* enemy)
		{
			Pouncer* pouncer = static_cast<Pouncer*>(enemy);
			pouncer->dir = Normalized(game->PlayerPos() - pouncer->pos);
			pouncer->vel = pouncer->dir * pouncer->speed;
			pouncer->vUpdate = VUPDATE::ENTITY;
			return true;
		}

		bool CatMU(Enemy* enemy)
		{
			Cat* cat = static_cast<Cat*>(enemy);
			cat->vel = cat->dir * cat->speed;
			cat->vUpdate = VUPDATE::ENTITY;
			return true;
		}

		bool TankMU(Enemy* enemy)
		{
			Tank* tank = static_cast<Tank*>(enemy);

			tank->dir = RotateTowardsNorm(tank->vel != vZero && glm::normalize(tank->vel) != tank->dir ? tank->vel : tank->dir, game->PlayerPos() -
				tank->pos, tank->turnSpeed * game->dTime);
			tank->vel = TryAdd2(tank->dir * glm::length(tank->vel), tank->dir * game->dTime * (tank->speed + game->planet->friction), tank->maxSpeed);

			return false;
		}
	}

	namespace AUpdates
	{
		bool DefaultAU(Enemy* enemy)
		{
			return game->entities->TryDealDamage(enemy->damage, enemy->pos, enemy->radius + 0.1f, MaskF::IsNonEnemy, enemy);
		}

		bool ExploderAU(Enemy* enemy)
		{
			Exploder* exploder = static_cast<Exploder*>(enemy);
			if (!game->entities->OverlapsAny(exploder->pos, exploder->explosionRadius, MaskF::IsNonEnemy, exploder))
				return false;
			CreateExplosion(exploder->pos, exploder->explosionRadius, exploder->color, exploder->name, 0, exploder->damage, exploder);
			return true;
		}

		bool BoomcatAU(Enemy* enemy)
		{
			BoomCat* bCat = static_cast<BoomCat*>(enemy);
			CreateExplosion(bCat->pos, bCat->explosionRadius, bCat->color, bCat->name, 0, bCat->damage, bCat);
			return true;
		}

		bool TankAU(Enemy* enemy)
		{
			Tank* tank = static_cast<Tank*>(enemy);
			game->entities->push_back(tank->projectile->Clone(tank->pos + tank->dir * tank->radius * 1.5f, tank->dir * tank->projectile->duration, tank));
			return true;
		}
	}
#pragma endregion


#pragma region Enemies
	//Predefinitions - Special
	LegParticle* spiderLeg = new LegParticle(vZero, nullptr, RGBA(0, 0, 0, 255), 32.0f, 0.25f);
	Projectile* tinyTankProjectile = new Projectile(15.0f, 1, 8.0f, 0.5f, RGBA(51, 51, 51), 1, 1, 1, "Tiny Tank Projectile");
	Enemy* child = new Enemy(1.0f, 12, 12, 0, 0, 1, 0.5f, RGBA(255, 0, 255), RGBA(), 1, 1, 1, "Child");
	Centicrawler* centicrawler = new Centicrawler(0.1f, 1, *spiderLeg, 3, 3.0f, 0.25f, 1.0f, 0.5f, 4, 4, 0, 0, 1, 0.5f, RGBA(186, 7, 66), RGBA(), 1, 6, 6, "Centicrawler");

	// Earlies - 1
	Enemy* walker = new Enemy(0.75f, 2, 2, 1, 1, 1, 0.5f, RGBA(0, 255, 255), RGBA(), 1, 3, 3, "Walker");
	Enemy* tanker = new Enemy(1.0f, 1.5f, 1.5f, 2, 1, 1, 1.5f, RGBA(255), RGBA(), 5, 12, 12, "Tanker");
	Spider* spider = new Spider(*spiderLeg, 6, 3.0f, 0.25f, 1.0f, 0.5f, 4, 4, 2, 1, 1, 0.5f, RGBA(79, 0, 26), RGBA(), 1, 2, 2, "Spider");
	Tank* tinyTank = new Tank(tinyTankProjectile, 0.78f, 1.0f, 4, 4, 3, 1, 1, 0.5f, RGBA(127, 127), RGBA(), 1, 3, 3, "Tiny Tank");

	// Mids - 4
	Deceiver* deceiver = new Deceiver(0.5f, 4, 4, 4, 4, 1, 0.5f, RGBA(255, 255, 255), RGBA(), RGBA(255, 255, 255, 153), 1, 3, 3, "Deceiver");
	Exploder* exploder = new Exploder(2.5f, 1.0f, 3, 3, 4, 4, 1, 0.5f, RGBA(153, 255, 0), RGBA(), 1, 3, 3, "Exploder");
	Vacuumer* vacuumer = new Vacuumer(4, 16, 2, 8, 0.125f, 8, 8, 3, 4, 0, 0.5f, RGBA(255, 255, 255), RGBA(), 1, 3, 3, "Vacuumer");
	Vacuumer* pusher = new Vacuumer(6, -32, 2, 2, 0.125f, 8, 8, 3, 4, 0, 0.5f, RGBA(255, 153, 255), RGBA(), 1, 3, 3, "Pusher");
	Pouncer* frog = new Pouncer(2, 16, 1, 4, 4, 4, 1, 0.5f, RGBA(107, 212, 91), RGBA(), 3, 3, 3, "Frog");

	// Mid-lates - 6
	Parent* parent = new Parent(child, 1, 1, 1, 4, 6, 1, 2.5f, RGBA(127, 0, 127), RGBA(), 1, 10, 10, "Parent");
	Parent* spiderParent = new Parent(centicrawler, 1, 1, 1, 4, 6, 1, 2.5f, RGBA(140, 35, 70), RGBA(), 5, 10, 10, "Spider Parent");
	Snake* snake = new Snake(30, 0.1f, 1, 0.5f, 4, 4, 30, 6, 1, 0.5f, RGBA(0, 255), RGBA(), RGBA(255, 255), RGBA(0, 127), 2, 3, 3, "Snake");
	Enemy* megaTanker = new Enemy(1, 1, 1, 20, 6, 1, 2.5f, RGBA(174, 0, 255), RGBA(), 10, 48, 48, "Mega Tanker");
	
	// Lates - 8
	ColorCycler* hyperSpeedster = new ColorCycler({ RGBA(255), RGBA(255, 255), RGBA(0, 0, 255) }, 2.0f, 0.5f, 3, 6, 8, 8, 1, 0.5f, RGBA(), 1, 24, 24, "Hyper Speedster");
	Exploder* gigaExploder = new Exploder(7.5f, 1.0f, 4, 4, 8, 8, 1, 1.5f, RGBA(153, 255), RGBA(), 1, 3, 3, "Giga Exploder");
	SnakeConnected* bigSnake = new SnakeConnected(30, 0.3f, 1, 0.5f, 4, 4, 30, 8, 1, 1.f, RGBA(0, 255), RGBA(), RGBA(255, 255), RGBA(0, 127), 5, 45, 45, "Big Snake");
	PouncerSnake* pouncerSnake = new PouncerSnake(3.0f, 24.0f, 30, 0.1f, 1, 0.5f, 8.0f, 60, 8, 1, 0.5f, RGBA(0, 0, 255), RGBA(), RGBA(0, 255, 255), RGBA(0, 0, 127), 2, 3, 3, "Pouncer Snake");

	// Very lates - 12
	Cat* cat = new Cat(1.5f, 2.0f, 16.0f, 0.25f, 3.0f, 45, 12, 1, 0.5f, RGBA(209, 96, 36), RGBA(), RGBA(186, 118, 82), 1, 9, 9, "Cat");
	BoomCat* boomCat = new BoomCat(4.5f, 1.5f, 2.0f, 12.0f, 1.0f, 4.0f, 45, 12, 1, 1.5f, RGBA(255, 120, 97), RGBA(), RGBA(158, 104, 95), 9, 9, 9, "Boom Cat");
	Spoobderb* spoobderb = new Spoobderb(centicrawler, *spiderLeg, 30, 25.0f, 3.0f, 2.5f, 0.5f, 2, 2, 50, 12, 1, 3.5f, RGBA(77, 14, 35), RGBA(), 50, 100, 100, "Spoobderb - The 30 footed beast");

	// Bosses - Special
	Projectile* catProjectile = new Projectile(25.0f, 1, cat->speed, cat->radius, cat->color, 1, 1, 1, "Cataclysmic Bullet");
	Projectile* catProjectile2 = new Projectile(25.0f, 1, cat->speed * 2, cat->radius, cat->color, 1, 1, 1, "Cataclysmic Bullet");
	Cataclysm* cataclysm = new Cataclysm(10.0f, 25.0f, PI_F / 5, catProjectile, catProjectile2, 0.0625f, 6.5f, 0.5f, 4.0f, 12.0f, 0.5f, 5.0f, 1000, 0, 1, 3.5f, RGBA(), RGBA(), RGBA(158, 104, 95), RGBA(127), 50, 9, 9, "Cataclysm - The nine lived feind");
#pragma endregion


#pragma region Spawning
	class Instance;
	class Types : public vector<vector<Enemy*>>
	{
	public:
		using vector<vector<Enemy*>>::vector;

		static int GetRoundPoints()
		{
			if (game->difficulty == DIFFICULTY::EASY)
				return static_cast<int>(pow(1.25, waveCount)) + waveCount * 2 - 1;
			else if (game->difficulty == DIFFICULTY::MEDIUM)
				return static_cast<int>(pow(1.37, waveCount)) + waveCount * 3 - 1;
			else // difficulty == hard
				return static_cast<int>(pow(1.45, waveCount)) + waveCount * 5 + 3;
		}

		Instance RandomClone();
	};

	int superWaveModifiers[] = { 3, 4, 5 }, difficultySeedSuperWaveCount[] = { 5, 3, 2 };

	class Instance : public vector<Enemy*>
	{
	public:
		using vector<Enemy*>::vector;
		bool superWave = false;

		void SpawnEnemy(int index)
		{
			float randomValue = RandFloat() * 6.283184f;
			game->entities->push_back((*this)[index]->Clone(Vec3(cosf(randomValue), sinf(randomValue), 0) * game->DistToCorner() + game->PlayerPos()));
		}

		void SpawnRandomEnemies()
		{
			if (superWave)
				waveCount += superWaveModifiers[game->difficulty];

			SpawnRandomEnemies(Types::GetRoundPoints());



			if (superWave)
			{
				waveCount -= superWaveModifiers[game->difficulty];
				for (int i = 0; i < difficultySeedSuperWaveCount[game->difficulty]; i++)
					game->entities->push_back(Collectibles::Seeds::plantSeeds[rand() % Collectibles::Seeds::plantSeeds.size()]->Clone(game->PlayerPos()));
				((Entity*)game->player)->DealDamage(-1, nullptr);
				superWave = false;
			}
		}

		void SpawnOneRandom()
		{
			SpawnEnemy(rand() % size());
		}

		void SpawnRandomEnemies(int cost)
		{
			int totalCost = 0;
			int currentlySpawnableEnemyCount = 1;
			for (int i = 1; i < (*this).size(); i++)
				currentlySpawnableEnemyCount += int((*this)[i]->firstWave <= waveCount && (*this)[i]->Cost() <= cost);
			vector<Enemy*> currentlySpawnableEnemies(currentlySpawnableEnemyCount);
			currentlySpawnableEnemies[0] = (*this)[0];
			for (int i = 1, j = 1; j < currentlySpawnableEnemyCount; i++)
				if ((*this)[i]->firstWave <= waveCount && (*this)[i]->Cost() <= cost)
					currentlySpawnableEnemies[j++] = (*this)[i];

			while (totalCost < cost)
			{
				int currentIndex = rand() % currentlySpawnableEnemyCount;
				SpawnEnemy(currentIndex);
				totalCost += currentlySpawnableEnemies[currentIndex]->Cost();
			}
		}
	};

	Instance Types::RandomClone()
	{
		Instance result(size());
		for (int i = 0; i < size(); i++)
			result[i] = (*this)[i][rand() % (*this)[i].size()];
		return result;
	}

	Types naturalSpawns
	{
		{walker, tanker, spider, tinyTank},
		{deceiver, exploder, vacuumer, pusher, frog},
		{parent, spiderParent, snake, megaTanker},
		{/*hyperSpeedster, gigaExploder, */bigSnake/*, pouncerSnake*/},
		{cat, boomCat, spoobderb}
	};

	Types spawnableBosses
	{
		{cataclysm}
	};
#pragma endregion
}