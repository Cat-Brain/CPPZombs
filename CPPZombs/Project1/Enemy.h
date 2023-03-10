#include "Defence.h"

namespace Enemies
{
#pragma region Enemy types
	// The base class of all enemies.
	class Enemy;
	enum MUPDATE
	{
		DEFAULTMU, SNAKEMU, POUNCERSNAKEMU, VACUUMERMU, CENTICRAWLER, POUNCERMU, CATMU, TANKMU
	};
	vector<function<bool(Enemy*)>> mUpdates;

	enum AUPDATE
	{
		DEFAULTAU, EXPLODERAU, BOOMCATAU, TANKAU
	};
	vector<function<bool(Enemy*)>> aUpdates;

	class Enemy : public DToCol
	{
	public:
		MUPDATE mUpdate;
		AUPDATE aUpdate;
		float timePer, lastTime, speed, timePerMove, lastMove;

		int points, firstWave;
		int damage;

		Enemy(float timePer = 0.5f, float speed = 0.25f, int points = 1, int firstWave = 1, int damage = 1, float radius = 0.5f,
			RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			DToCol(vZero, radius, color, color2, mass, maxHealth, health, name),
			timePer(timePer), lastTime(0.0f), speed(speed), timePerMove(0.0f), lastMove(0.0f), points(points), firstWave(firstWave),
			damage(damage), mUpdate(MUPDATE::DEFAULTMU), aUpdate(AUPDATE::DEFAULTAU)
		{
			update = UPDATE::ENEMYU;
			uiUpdate = UIUPDATE::ENEMYUIU;
			onDeath = ONDEATH::ENEMYOD;
			isEnemy = true;
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

		unique_ptr<Entity> Clone(Vec2 pos = vZero, Vec2 dir = vZero, Entity* creator = nullptr) override
		{
			return make_unique<Enemy>(this, pos);
		}

		bool MUpdate()
		{
			return mUpdates[mUpdate](this);
		}
		bool MUpdate(MUPDATE tempMUpdate)
		{
			return mUpdates[tempMUpdate](this);
		}

		bool AUpdate()
		{
			return aUpdates[aUpdate](this);
		}
		bool AUpdate(AUPDATE tempAUpdate)
		{
			return aUpdates[tempAUpdate](this);
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

		Deceiver(float timePer = 0.5f, float speed = 2, int points = 1, int firstWave = 1, int damage = 1, float radius = 0.5f,
			RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, speed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name), color3(color3), noise1(), noise2(), noise3()
		{
			Start();
			dUpdate = DUPDATE::DECEIVERDU;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
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

		Parent(Enemy* child, float timePer = 0.5f, float speed = 2, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, speed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name), child(child)
		{
			Start();
			dUpdate = DUPDATE::PARENTDU;
			onDeath = ONDEATH::PARENTOD;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
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

		Exploder(float explosionRadius, float timePer = 0.5f, float speed = 2, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, speed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name), explosionRadius(explosionRadius)
		{
			dUpdate = DUPDATE::EXPLODERDU;
			aUpdate = AUPDATE::EXPLODERAU;
			onDeath = ONDEATH::EXPLODEROD;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
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

		Snake(int length, float timePer = 0.5f, float speed = 2, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, speed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name), length(length), color3(color3), color4(color4)
		{
			onDeath = ONDEATH::SNAKEOD;
			mUpdate = MUPDATE::SNAKEMU;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Snake>* enemies = new unique_ptr<Snake>[length];
			for (int i = 0; i < length; i++)
			{
				enemies[i] = make_unique<Snake>(*this);
				enemies[i]->baseClass = baseClass;
				enemies[i]->pos = pos + RandCircPoint();
				enemies[i]->points = points / length;
				enemies[i]->Start();
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

		PouncerSnake(float pounceTime, float speed, int length, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Snake(length, timePer, speed, points, firstWave, damage, radius, color, color2, color3, color4, mass, maxHealth, health, name),
			pounceTime(pounceTime)
		{
			this->timePerMove = timePerMove;
			update = UPDATE::POUNCERSNAKEU;
			onDeath = ONDEATH::POUNCERSNAKEOD;
			mUpdate = MUPDATE::POUNCERSNAKEMU;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<PouncerSnake>* enemies = new unique_ptr<PouncerSnake>[length];
			for (int i = 0; i < length; i++)
			{
				enemies[i] = make_unique<PouncerSnake>(*this);
				enemies[i]->baseClass = baseClass;
				enemies[i]->pos = pos + RandCircPoint();
				enemies[i]->points = points / length;
				enemies[i]->Start();
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

	class ColorCycler : public Enemy
	{
	public:
		vector<RGBA> colorsToCycle;
		float colorOffset, colorCycleSpeed;

		ColorCycler(vector<RGBA> colorsToCycle, float colorCycleSpeed = 1.0f, float timePer = 0.5f, float speed = 2, int points = 1,
			int firstWave = 1, int damage = 1, float radius = 0.5f, RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, speed, points, firstWave, damage, radius, colorsToCycle[0], color2, mass, maxHealth, health, name),
			colorsToCycle(colorsToCycle), colorCycleSpeed(colorCycleSpeed), colorOffset(0.0f)
		{
			dUpdate = DUPDATE::COLORCYCLERDU;
		}

		void Start() override
		{
			Enemy::Start();
			colorOffset = RandFloat() * colorCycleSpeed;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
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
		float vacDist, vacSpeed, desiredDistance;
		Items items;

		Vacuumer(float vacDist, float vacSpeed, float desiredDistance, float timePer = 0.5f, float speed = 2, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, speed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name),
			vacDist(vacDist), vacSpeed(vacSpeed), desiredDistance(desiredDistance), items(0)
		{
			update = UPDATE::VACUUMERU;
			onDeath = ONDEATH::VACUUMEROD;
			mUpdate = MUPDATE::VACUUMERMU;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
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
			float timePer = 0.5f, float moveSpeed = 2.0f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, moveSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name),
			baseLeg(baseLeg), legCount(legCount), legLength(legLength), legTolerance(legTolerance), legCycleSpeed(legCycleSpeed)
		{
			onDeath = ONDEATH::SPIDEROD;
			update = UPDATE::SPIDERU;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
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

		Vec2 LegPos(int index)
		{
			float rotation = (float(index) / legCount + legRotation) * 2 * PI_F;
			return Vec2(pos) + Vec2(sinf(rotation), cosf(rotation)) * legLength;
		}

		void UpdateLegs()
		{
			for (int i = 0; i < legCount; i++)
			{
				if (tTime - lastLegUpdates[i] > legCycleSpeed)
				{
					lastLegUpdates[i] += legCycleSpeed;
					Vec2 desiredPos = LegPos(i) + Normalized(Vec2(game->PlayerPos() - pos)) * legTolerance * 0.5f;
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
			float timePer = 0.5f, float moveSpeed = 2.0f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			baseChild(baseChild), Spider(baseLeg, legCount, legLength, legTolerance, legCycleSpeed,
				timePer, moveSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name) { }

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
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
				game->entities->push_back(baseChild->Clone(Vec2(pos) - Vec2(radius + 1) + Vec2(RandFloat() * ((radius + 1) * 2), RandFloat() * ((radius + 1) * 2)), up, this));
			return Spider::DealDamage(damage, damageDealer);
		}
	};

	class Centicrawler : public Spider
	{
	public:
		Centicrawler* back = nullptr, * front = nullptr;

		Centicrawler(LegParticle baseLeg, int legCount = 8, float legLength = 5.0f, float legTolerance = 3.0f, float legCycleSpeed = 1.0f,
			float timePer = 0.5f, float moveSpeed = 2.0f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Spider(baseLeg, legCount, legLength, legTolerance, legCycleSpeed, timePer, moveSpeed, points, firstWave, damage, radius, color, color2,
				mass, maxHealth, health, name)
		{
			update = UPDATE::CENTICRAWLERU;
			onDeath = ONDEATH::CENTICRAWLEROD;
			mUpdate = MUPDATE::CENTICRAWLER;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Centicrawler> newEnemy = make_unique<Centicrawler>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->back = nullptr;
			newEnemy->front = nullptr;
			newEnemy->Start();
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
			Enemy(timePer, moveSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name),
			pounceTime(pounceTime)
		{
			this->timePerMove = timePerMove;
			update = UPDATE::POUNCERU;
			dUpdate = DUPDATE::POUNCERDU;
			mUpdate = MUPDATE::POUNCERMU;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Pouncer> newEnemy = make_unique<Pouncer>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = Normalized(Vec2(game->PlayerPos() - newEnemy->pos));
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
			update = UPDATE::CATU;
			dUpdate = DUPDATE::CATDU;
			mUpdate = MUPDATE::CATMU;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Cat> newEnemy = make_unique<Cat>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = Normalized(Vec2(game->PlayerPos() - newEnemy->pos));
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
			aUpdate = AUPDATE::BOOMCATAU;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<BoomCat> newEnemy = make_unique<BoomCat>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
		}
	};

	class Cataclysm : public BoomCat
	{
	public:
		float lastStartedCircle = -100, circleTime, circleRadius, spinSpeed;
		float lastShoot = -100, timePerShot;
		Projectile* projectile;
		RGBA color4;

		Cataclysm(float circleTime, float circleRadius, float spinSpeed, Projectile* projectile, float timePerShot, float explosionRadius,
			float homeSpeed, float pounceTime, float moveSpeed, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			BoomCat(explosionRadius, homeSpeed, pounceTime, moveSpeed, timePer, timePerMove, points, firstWave, damage, radius, color, color2, color3,
				mass, maxHealth, health, name), circleTime(circleTime), circleRadius(circleRadius), spinSpeed(spinSpeed), projectile(projectile), timePerShot(timePerShot), color4(color4)
		{
			update = UPDATE::CATACLYSMU;
			dUpdate = DUPDATE::CATACLYSMDU;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Cataclysm> newEnemy = make_unique<Cataclysm>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
		}

		int DealDamage(int damage, Entity* damageDealer) override
		{
			if (tTime - lastStartedCircle > circleTime)
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
		iVec2 currentMovingDirection = vZero;

		Tank(Projectile* projectile, float timePer = 0.5f, float moveSpeed = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, moveSpeed, points, firstWave, damage, radius, color, color2, mass, maxHealth, health, name), projectile(projectile)
		{
			mUpdate = MUPDATE::TANKMU;
			aUpdate = AUPDATE::TANKAU;
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Tank> newEnemy = make_unique<Tank>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
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
			pSnake->Update(UPDATE::ENEMYU);

			if (pSnake->front == nullptr)
			{
				if (tTime - pSnake->lastMove <= pSnake->pounceTime)
					pSnake->TryMove(pSnake->dir * game->dTime * pSnake->speed, pSnake->mass * 2);
			}
			else
			{
				float dist = glm::distance(pSnake->front->pos, pSnake->pos);
				pSnake->TryMove((pSnake->front->pos - pSnake->pos) / dist * (dist - pSnake->radius - pSnake->front->radius), 1);
			}
		}

		void VacuumerU(Entity* entity)
		{
			Vacuumer* vacuumer = static_cast<Vacuumer*>(entity);
			vacuumer->Update(UPDATE::ENEMYU);

			game->entities->VacuumBoth(vacuumer->pos, vacuumer->vacDist, vacuumer->vacSpeed);

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

			spider->Update(UPDATE::ENEMYU);
			spider->UpdateLegs();
		}

		void CenticrawlerU(Entity* entity)
		{
			Centicrawler* centicrawler = static_cast<Centicrawler*>(entity);

			centicrawler->Update(UPDATE::SPIDERU);
			if (centicrawler->front == nullptr)
			{
				Centicrawler* farthestBack = centicrawler;
				while (farthestBack->back != nullptr)
					farthestBack = farthestBack->back;
				vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(centicrawler->pos, centicrawler->radius + 0.5f);
				for (int i = 0; i < hitEntities.size(); i++)
				{
					Centicrawler* currentCenticrawler = static_cast<Centicrawler*>(hitEntities[i]);
					if (currentCenticrawler != centicrawler && currentCenticrawler->baseClass == centicrawler->baseClass && currentCenticrawler->back == nullptr && currentCenticrawler != farthestBack)
					{
						currentCenticrawler->back = centicrawler;
						centicrawler->front = currentCenticrawler;
						return;
					}
				}
			}
		}

		void PouncerU(Entity* entity)
		{
			Pouncer* pouncer = static_cast<Pouncer*>(entity);

			pouncer->Update(UPDATE::ENEMYU);

			if (tTime - pouncer->lastMove <= pouncer->pounceTime)
				pouncer->TryMove(pouncer->dir * game->dTime * pouncer->speed, pouncer->mass * 2);
		}

		void CatU(Entity* entity)
		{
			Cat* cat = static_cast<Cat*>(entity);

			float currentRotation = atan2f(cat->dir.y, cat->dir.x);
			Vec2 desiredDir = game->PlayerPos() - cat->pos;
			float desiredRotation = atan2f(desiredDir.y, desiredDir.x);

			currentRotation -= (roundf(ModF(desiredRotation - currentRotation, PI_F * 2) / (PI_F * 2)) * 2 - 1) * cat->homeSpeed * game->dTime;

			cat->dir = Vec2(cosf(currentRotation), sinf(currentRotation));

			cat->Update(UPDATE::POUNCERU);
		}

		void CataclysmU(Entity* entity)
		{
			Cataclysm* cat = static_cast<Cataclysm*>(entity);

			if (tTime - cat->lastStartedCircle < cat->circleTime)
			{
				float rotation = (tTime - cat->lastStartedCircle) * cat->spinSpeed;
				cat->TryMove(game->PlayerPos() - cat->pos + Vec2(sinf(rotation) * cat->circleRadius, cosf(rotation) * cat->circleRadius), cat->mass * 2);

				if (tTime - cat->lastShoot > cat->timePerShot)
				{
					cat->lastShoot = tTime;
					int count = cat->maxHealth - cat->health + 1;
					Vec2 pos = cat->pos;

					for (int i = 1; i < count; i++)
					{
						pos = glm::rotate(pos - Vec2(game->PlayerPos()), PI_F * 2 / count) + Vec2(game->PlayerPos());
						game->entities->push_back(cat->projectile->Clone(pos, game->PlayerPos() - pos, cat));
					}

					game->entities->push_back(cat->projectile->Clone(cat->pos, game->PlayerPos() - cat->pos, cat));

				}
			}
			else cat->Update(UPDATE::CATU);
		}
	}

	namespace DUpdates
	{
		void DeceiverDU(Entity* entity)
		{
			Deceiver* deceiver = static_cast<Deceiver*>(entity);
			float r = deceiver->noise1.GetNoise(tTime, 0.0f);
			float g = deceiver->noise2.GetNoise(tTime, 0.0f);
			float b = deceiver->noise3.GetNoise(tTime, 0.0f);
			deceiver->color.r = static_cast<byte>(r * 255);
			deceiver->color.g = static_cast<byte>(g * 255);
			deceiver->color.b = static_cast<byte>(b * 255);

			deceiver->DUpdate(DUPDATE::DTOCOLDU);

			float healthRatio = (float)deceiver->health / deceiver->maxHealth;
			deceiver->color.r = static_cast<byte>(r * deceiver->color3.r);
			deceiver->color.g = static_cast<byte>(g * deceiver->color3.g);
			deceiver->color.b = static_cast<byte>(b * deceiver->color3.b);

			Vec2 tempPos = deceiver->pos;

			deceiver->pos = Vec2(game->PlayerPos().x * 2 - deceiver->pos.x, deceiver->pos.y);
			deceiver->DUpdate(DUPDATE::DTOCOLDU);
			deceiver->pos = Vec2(deceiver->pos.x, game->PlayerPos().y * 2 - deceiver->pos.y);
			deceiver->DUpdate(DUPDATE::DTOCOLDU);
			deceiver->pos = Vec2(game->PlayerPos().x * 2 - deceiver->pos.x, deceiver->pos.y);
			deceiver->DUpdate(DUPDATE::DTOCOLDU);

			deceiver->pos = tempPos;
		}

		void ParentDU(Entity* entity)
		{
			Parent* parent = static_cast<Parent*>(entity);
			parent->DUpdate(DUPDATE::DTOCOLDU);
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
			exploder->DUpdate(DUPDATE::DTOCOLDU);
			exploder->radius = tempRadius;
			exploder->color.a = tempAlpha;
			exploder->DUpdate(DUPDATE::DTOCOLDU);
		}

		void ColorCyclerDU(Entity* entity)
		{
			ColorCycler* colorCycler = static_cast<ColorCycler*>(entity);
			float currentPlace = (tTime * colorCycler->colorCycleSpeed + colorCycler->colorOffset);
			int intCurrentPlace = static_cast<int>(currentPlace);
			colorCycler->color = colorCycler->colorsToCycle[intCurrentPlace % colorCycler->colorsToCycle.size()].CLerp(
				colorCycler->colorsToCycle[(static_cast<size_t>(intCurrentPlace) + 1) % colorCycler->colorsToCycle.size()], currentPlace - floorf(currentPlace));

			colorCycler->DUpdate(DUPDATE::DTOCOLDU);
		}

		void PouncerDU(Entity* entity)
		{
			Pouncer* pouncer = static_cast<Pouncer*>(entity);

			float ratio = pouncer->radius / SQRTTWO_F;
			game->DrawRightTri(pouncer->pos + pouncer->dir * ratio, vOne * (ratio * 2),
				atan2f(pouncer->dir.y, pouncer->dir.x) - PI_F * 0.5f, pouncer->color);

			pouncer->DUpdate(DUPDATE::DTOCOLDU);
		}

		void CatDU(Entity* entity)
		{
			Cat* cat = static_cast<Cat*>(entity);

			int count = cat->maxHealth - cat->health + 1;

			Vec2 tempPos = cat->pos;
			RGBA tempColor = cat->color;
			cat->color = cat->color3;

			for (int i = 1; i < count; i++)
			{
				cat->pos = glm::rotate(cat->pos - Vec2(game->PlayerPos()), PI_F * 2 / count) + Vec2(game->PlayerPos());
				cat->DUpdate(DUPDATE::ENTITYDU);
			}

			cat->color = tempColor;
			cat->pos = tempPos;
			cat->DUpdate(DUPDATE::POUNCERDU);
		}

		void CataclysmDU(Entity* entity)
		{
			Cataclysm* cat = static_cast<Cataclysm*>(entity);
			RGBA tempColor = cat->color;
			if (tTime - cat->lastStartedCircle < cat->circleTime)
				cat->color = cat->color4.CLerp(cat->color, sinf(tTime * 4 * PI_F) * 0.5f + 0.5f);
			cat->DUpdate(DUPDATE::CATDU);
			cat->color = tempColor;
		}
	}

	namespace UIUpdates
	{
		void EnemyUIU(Entity* entity)
		{
			string health = to_string(entity->health);
			Vec2 bottomLeft = entity->BottomLeft();
			Vec2 topRight = bottomLeft + Vec2(font.TextWidth(entity->name + " " + health) * COMMON_TEXT_SCALE / font.minimumSize, font.maxVertOffset / 2) / 2.f;
			entity->DrawUIBox(bottomLeft, topRight, static_cast<float>(COMMON_BOARDER_WIDTH), entity->name + " " + health, entity->color);
		}
	}

	namespace OnDeaths
	{
		void EnemyOD(Entity* entity, Entity* damageDealer)
		{
			Enemy* enemy = static_cast<Enemy*>(entity);
			totalGamePoints += enemy->points;
			int randomValue = rand() % 2048; // 0-2047
			if (randomValue > 1022) // Half of the time is true I think.
			{
				if (randomValue > 1500) // 1501-2047 ~= 1/4
					game->entities->push_back(Collectibles::copper->Clone(enemy->pos));
				else if (randomValue % 16 < Collectibles::Seeds::plantSeeds.size()) // This system will work until there's > 16 plants.
					game->entities->push_back(Collectibles::Seeds::plantSeeds[randomValue % 16]->Clone(enemy->pos));
			}
		}

		void ParentOD(Entity* entity, Entity* damageDealer)
		{
			Parent* parent = static_cast<Parent*>(entity);
			parent->OnDeath(ONDEATH::ENEMYOD, damageDealer);

			game->entities->push_back(parent->child->Clone(parent->pos + up));
			game->entities->push_back(parent->child->Clone(parent->pos + right));
			game->entities->push_back(parent->child->Clone(parent->pos + down));
			game->entities->push_back(parent->child->Clone(parent->pos + left));
		}

		void ExploderOD(Entity* entity, Entity* damageDealer)
		{
			Exploder* exploder = static_cast<Exploder*>(entity);
			exploder->OnDeath(ONDEATH::ENEMYOD, damageDealer);
			CreateExplosion(exploder->pos, exploder->explosionRadius, exploder->color, exploder->name, 0, exploder->damage, exploder);
		}

		void SnakeOD(Entity* entity, Entity* damageDealer)
		{
			Snake* snake = static_cast<Snake*>(entity);
			snake->OnDeath(ONDEATH::ENEMYOD, damageDealer);
			if (snake->back != nullptr)
			{
				snake->back->front = nullptr;
				snake->back->color = snake->color3;
				/*Snake* currentSegment = snake;
				int length = 0;
				while (currentSegment->back != nullptr)
				{
					length++;
					currentSegment = currentSegment->back;
				}
				currentSegment = snake;
				for (int i = 0; i < length; i++)
				{
					currentSegment = currentSegment->back;
					currentSegment->length = length;
				}*/
			}
			if (snake->front != nullptr)
			{
				snake->front->back = nullptr;
				/*Snake* currentSegment = snake;
				int length = 0;
				while (currentSegment->front != nullptr)
				{
					length++;
					currentSegment = currentSegment->front;
				}
				currentSegment = snake;
				for (int i = 0; i < length; i++)
				{
					currentSegment = currentSegment->front;
					currentSegment->length = length;
				}*/
			}
		}

		void PouncerSnakeOD(Entity* entity, Entity* damageDealer)
		{
			PouncerSnake* pSnake = static_cast<PouncerSnake*>(entity);
			pSnake->OnDeath(ONDEATH::SNAKEOD, damageDealer);
			if (pSnake->back != nullptr)
				((PouncerSnake*)pSnake->back)->dir = pSnake->dir;
		}

		void VacuumerOD(Entity* entity, Entity* damageDealer)
		{
			Vacuumer* vacuumer = static_cast<Vacuumer*>(entity);
			vacuumer->OnDeath(ENEMYOD, damageDealer);
			for (Item item : vacuumer->items)
			{
				game->entities->push_back(make_unique<Collectible>(item, vacuumer->pos));
			}
		}

		void SpiderOD(Entity* entity, Entity* damageDealer)
		{
			Spider* spider = static_cast<Spider*>(entity);
			spider->OnDeath(ONDEATH::ENEMYOD, damageDealer);
			for (int i = 0; i < spider->legCount; i++)
				delete spider->legs[i];
		}

		void CenticrawlerOD(Entity* entity, Entity* damageDealer)
		{
			Centicrawler* centicrawler = static_cast<Centicrawler*>(entity);
			centicrawler->OnDeath(ONDEATH::SPIDEROD, damageDealer);
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
			enemy->TryMove(Normalized(Vec2(game->PlayerPos() - enemy->pos)) * game->dTime * enemy->speed, enemy->mass * 2);
			return false;
		}
	
		bool SnakeMU(Enemy* enemy)
		{
			Snake* snake = static_cast<Snake*>(enemy);
			if (snake->front == nullptr)
				snake->TryMove(Normalized(game->PlayerPos() - snake->pos) * game->dTime * snake->speed, snake->mass * 2);
			else
			{
				float dist = glm::distance(snake->front->pos, snake->pos);
				snake->TryMove((snake->front->pos - snake->pos) / dist * (dist - snake->radius - snake->front->radius), 1);
			}
			return false;
		}

		bool PouncerSnakeMU(Enemy* enemy)
		{
			PouncerSnake* pSnake = static_cast<PouncerSnake*>(enemy);
			pSnake->dir = Normalized(Vec2(game->PlayerPos() - pSnake->pos));
			return true;
		}

		bool VacuumerMU(Enemy* enemy)
		{
			Vacuumer* vacuumer = static_cast<Vacuumer*>(enemy);
			if (abs(Squistance(vacuumer->pos, game->PlayerPos()) - vacuumer->desiredDistance) > 2.0f)
				vacuumer->TryMove(Normalized(Vec2(game->PlayerPos() - enemy->pos)) * enemy->speed * game->dTime *
					(-1.f + 2 * int(Squistance(vacuumer->pos, game->PlayerPos()) > vacuumer->desiredDistance)), vacuumer->mass * 2);
			return false;
		}

		bool CenticrawlerMU(Enemy* enemy)
		{
			Centicrawler* centicrawler = static_cast<Centicrawler*>(enemy);
			if (centicrawler->front == nullptr)
				centicrawler->TryMove(Normalized(game->PlayerPos() - centicrawler->pos) * game->dTime * centicrawler->speed, centicrawler->mass * 2);
			else
			{
				float dist = glm::distance(centicrawler->front->pos, centicrawler->pos);
				centicrawler->TryMove((centicrawler->front->pos - centicrawler->pos) / dist * (dist - centicrawler->radius - centicrawler->front->radius), 1);
			}
			return false;
			return true;
		}

		bool PouncerMU(Enemy* enemy)
		{
			Pouncer* pouncer = static_cast<Pouncer*>(enemy);
			pouncer->dir = Normalized(Vec2(game->PlayerPos() - pouncer->pos));
			return true;
		}

		bool CatMU(Enemy* enemy)
		{
			return true;
		}

		bool TankMU(Enemy* enemy)
		{
			Tank* tank = static_cast<Tank*>(enemy);
			iVec2 disp = game->PlayerPos() - tank->pos;
			if (sgn(float(disp.x)) != sgn(float(tank->currentMovingDirection.x)) && sgn(float(disp.y)) != sgn(float(tank->currentMovingDirection.y)))
				tank->currentMovingDirection = abs(disp.x) > abs(disp.y) ? iVec2(sgn(float(disp.x)), 0) : iVec2(0, sgn(float(disp.y)));
			tank->TryMove(Vec2(tank->currentMovingDirection) * tank->speed * game->dTime, tank->mass * 2);
			return false;
		}
	}

	namespace AUpdates
	{
		bool DefaultAU(Enemy* enemy)
		{
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(enemy->pos, enemy->radius + 0.5f);
			int randomization = rand();
			for (int i = 0; i < hitEntities.size(); i++)
			{
				Entity* entity = hitEntities[(static_cast<size_t>(i) + randomization) % hitEntities.size()];
				if (entity != enemy && !entity->isEnemy)
				{
					entity->DealDamage(enemy->damage, enemy);
					break;
				}
			}

			return true;
		}

		bool ExploderAU(Enemy* enemy)
		{
			Exploder* exploder = static_cast<Exploder*>(enemy);
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(exploder->pos, exploder->explosionRadius);
			int randomization = rand();
			for (int i = 0; i < hitEntities.size(); i++)
			{
				Entity* entity = hitEntities[(static_cast<size_t>(i) + randomization) % hitEntities.size()];
				if (entity != exploder && !entity->isEnemy)
				{
					CreateExplosion(exploder->pos, exploder->explosionRadius, exploder->color, exploder->name, 0, exploder->damage, exploder);
					return true;
				}
			}
			return false;
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
			game->entities->push_back(tank->projectile->Clone(tank->pos, (game->PlayerPos() - tank->pos) * tank->projectile->duration, tank));
			return true;
		}
	}
#pragma endregion


#pragma region Enemies
	//Predefinitions - Special
	LegParticle* spiderLeg = new LegParticle(vZero, nullptr, RGBA(0, 0, 0, 255), 32.0f, 0.25f);
	Projectile* tinyTankProjectile = new Projectile(15.0f, 1, 8.0f, 0.5f, RGBA(51, 51, 51), 1, 1, 1, "Tiny Tank Projectile");
	Enemy* child = new Enemy(1.0f, 8, 0, 0, 1, 0.5f, RGBA(255, 0, 255), RGBA(), 1, 1, 1, "Child");
	Centicrawler* centicrawler = new Centicrawler(*spiderLeg, 3, 3.0f, 0.25f, 1.0f, 0.5f, 4, 0, 0, 1, 0.5f, RGBA(186, 7, 66), RGBA(), 1, 6, 6, "Centicrawler");

	// Earlies - 1
	Enemy* walker = new Enemy(0.75f, 2, 1, 1, 1, 0.5f, RGBA(0, 255, 255), RGBA(), 1, 3, 3, "Walker");
	Enemy* tanker = new Enemy(1.0f, 1.5f, 2, 1, 1, 1.5f, RGBA(255), RGBA(), 5, 12, 12, "Tanker");
	Spider* spider = new Spider(*spiderLeg, 6, 3.0f, 0.25f, 1.0f, 0.5f, 4, 2, 1, 1, 0.5f, RGBA(79, 0, 26), RGBA(), 1, 2, 2, "Spider");
	Tank* tinyTank = new Tank(tinyTankProjectile, 1.0f, 4, 2, 1, 1, 0.5f, RGBA(127, 127), RGBA(), 1, 1, 1, "Tiny Tank");

	// Mids - 4
	Deceiver* deceiver = new Deceiver(0.5f, 4, 4, 4, 1, 0.5f, RGBA(255, 255, 255), RGBA(), RGBA(255, 255, 255, 153), 1, 3, 3, "Deceiver");
	Exploder* exploder = new Exploder(2.5f, 1.0f, 3, 4, 4, 1, 0.5f, RGBA(153, 255, 0), RGBA(), 1, 3, 3, "Exploder");
	Vacuumer* vacuumer = new Vacuumer(4, 1, 8, 0.125f, 8, 3, 4, 0, 0.5f, RGBA(255, 255, 255), RGBA(), 1, 3, 3, "Vacuumer");
	Vacuumer* pusher = new Vacuumer(6, -3, 2, 0.125f, 8, 3, 4, 0, 0.5f, RGBA(255, 153, 255), RGBA(), 1, 3, 3, "Pusher");
	Pouncer* frog = new Pouncer(2.0f, 16.0f, 1.0f, 4.0f, 4, 4, 1, 0.5f, RGBA(107, 212, 91), RGBA(), 3, 3, 3, "Frog");

	// Mid-lates - 6
	Parent* parent = new Parent(child, 1.0f, 1.0f, 4, 6, 1, 2.5f, RGBA(127, 0, 127), RGBA(), 1, 10, 10, "Parent");
	Parent* spiderParent = new Parent(centicrawler, 1.0f, 1.0f, 4, 6, 1, 2.5f, RGBA(140, 35, 70), RGBA(), 5, 10, 10, "Spider Parent");
	Snake* snake = new Snake(30, 0.5f, 4, 30, 6, 1, 0.5f, RGBA(0, 255), RGBA(), RGBA(255, 255), RGBA(0, 127), 2, 3, 3, "Snake");
	Enemy* megaTanker = new Enemy(1.0f, 1.0f, 20, 6, 1, 2.5f, RGBA(174, 0, 255), RGBA(), 10, 48, 48, "Mega Tanker");
	
	// Lates - 8
	ColorCycler* hyperSpeedster = new ColorCycler({ RGBA(255), RGBA(255, 255), RGBA(0, 0, 255) }, 2.0f, 0.5f, 4, 8, 8, 1, 0.5f, RGBA(), 1, 24, 24, "Hyper Speedster");
	Exploder* gigaExploder = new Exploder(7.5f, 1.0f, 4, 8, 8, 1, 1.5f, RGBA(153, 255), RGBA(), 1, 3, 3, "Giga Exploder");
	Snake* bigSnake = new Snake(30, 0.5f, 4, 60, 8, 1, 1.5f, RGBA(0, 255), RGBA(), RGBA(255, 255), RGBA(0, 127), 2, 9, 9, "Big Snake");
	PouncerSnake* pouncerSnake = new PouncerSnake(3.0f, 24.0f, 30, 0.5f, 8.0f, 60, 8, 1, 0.5f, RGBA(0, 0, 255), RGBA(), RGBA(0, 255, 255), RGBA(0, 0, 127), 2, 3, 3, "Pouncer Snake");

	// Very lates - 12
	Cat* cat = new Cat(1.5f, 2.0f, 16.0f, 0.25f, 3.0f, 45, 12, 1, 0.5f, RGBA(209, 96, 36), RGBA(), RGBA(186, 118, 82), 1, 9, 9, "Cat");
	BoomCat* boomCat = new BoomCat(4.5f, 1.5f, 2.0f, 12.0f, 1.0f, 4.0f, 45, 12, 1, 1.5f, RGBA(255, 120, 97), RGBA(), RGBA(158, 104, 95), 9, 9, 9, "Boom Cat");
	Spoobderb* spoobderb = new Spoobderb(centicrawler, *spiderLeg, 30, 25.0f, 3.0f, 2.5f, 0.5f, 2, 50, 12, 1, 3.5f, RGBA(77, 14, 35), RGBA(), 50, 100, 100, "Spoobderb - The 30 footed beast");

	// Bosses - Special
	Projectile* catProjectile = new Projectile(25.0f, 1, cat->speed, cat->radius, cat->color, 1, 1, 1, "Cataclysmic Bullet");
	Cataclysm* cataclysm = new Cataclysm(10.0f, 25.0f, PI_F / 5, catProjectile, 0.0625f, 6.5f, 1.5f, 5.0f, 12.0f, 0.5f, 5.0f, 1000, 0, 1, 3.5f, RGBA(), RGBA(), RGBA(158, 104, 95), RGBA(127), 50, 9, 9, "Cataclysm - The nine lived feind");
#pragma endregion

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

	class Instance : public vector<Enemy*>
	{
	public:
		using vector<Enemy*>::vector;

		void SpawnEnemy(int index)
		{
			float randomValue = RandFloat() * 6.283184f;
			game->entities->push_back((*this)[index]->Clone(Vec2(cosf(randomValue), sinf(randomValue)) * game->zoom * 1.415f + Vec2(game->PlayerPos())));
		}

		void SpawnRandomEnemies()
		{
			int totalCost = 1, costToAchieve = Types::GetRoundPoints();
			int currentlySpawnableEnemyCount = 1;
			for (int i = 1; i < (*this).size(); i++)
				currentlySpawnableEnemyCount += int((*this)[i]->firstWave <= waveCount && (*this)[i]->Cost() <= costToAchieve);
			vector<Enemy*> currentlySpawnableEnemies(currentlySpawnableEnemyCount);
			currentlySpawnableEnemies[0] = (*this)[0];
			for (int i = 1, j = 1; j < currentlySpawnableEnemyCount; i++)
				if ((*this)[i]->firstWave <= waveCount && (*this)[i]->Cost() <= costToAchieve)
					currentlySpawnableEnemies[j++] = (*this)[i];

			while (totalCost < costToAchieve)
			{
				int currentIndex = rand() % currentlySpawnableEnemyCount;
				SpawnEnemy(currentIndex);
				totalCost += currentlySpawnableEnemies[currentIndex]->Cost();
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
		{hyperSpeedster, gigaExploder, bigSnake, pouncerSnake},
		{cat, boomCat, spoobderb}
	};

	Types spawnableBosses
	{
		{cataclysm}
	};
}