#include "Defence.h"

namespace Enemies
{
	Entity* NearestFoe(Vec3 pos, float farDist, Entity* entity)
	{
		Entity* result = nullptr;
		vector<Entity*> nearbyFoes = game->entities->FindOverlaps(pos, farDist, MaskF::IsNonAlly, entity);
		float dist = farDist * farDist, newDist;
		for (Entity* nEntity : nearbyFoes)
			if ((newDist = glm::distance2(entity->pos, nEntity->pos)) < dist)
			{
				dist = newDist;
				result = nEntity;
			}
		return result;
	}

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

	class EnemyData : public EntityData
	{
	public:
		MUPDATE mUpdate;
		AUPDATE aUpdate;

		EnemyData(MUPDATE mUpdate = MUPDATE::DEFAULT, AUPDATE aUpdate = AUPDATE::DEFAULT, UPDATE update = UPDATE::ENTITY,
			VUPDATE vUpdate = VUPDATE::FRICTION, DUPDATE dUpdate = DUPDATE::ENTITY, EDUPDATE eDUpdate = EDUPDATE::ENTITY,
			UIUPDATE uiUpdate = UIUPDATE::ENTITY, ONDEATH onDeath = ONDEATH::ENTITY) :
			EntityData(update, vUpdate, dUpdate, eDUpdate, uiUpdate, onDeath),
			mUpdate(mUpdate), aUpdate(aUpdate) {}
	};

	EnemyData enemyData = EnemyData(MUPDATE::DEFAULT, AUPDATE::DEFAULT, UPDATE::ENEMY, VUPDATE::FRICTION, DUPDATE::DTOCOL, EDUPDATE::ENTITY, UIUPDATE::ENEMY, ONDEATH::ENEMY);
	class Enemy : public DToCol
	{
	public:
		float timePer, lastTime, speed, maxSpeed, timePerMove, lastMove, jumpTime, lastJump;

		int points, firstWave;
		int damage;

		Enemy(EntityData* data, Allegiance allegiance = 0, float timePer = 0.5f, float speed = 0.25f, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1, int firstWave = 1, int damage = 1, float radius = 0.5f,
			RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			DToCol(data, vZero, radius, color, color2, mass, bounciness, maxHealth, health, name, allegiance),
			timePer(timePer), lastTime(0.0f), speed(speed), maxSpeed(maxSpeed), timePerMove(0.0f), lastMove(0.0f), points(points), firstWave(firstWave),
			damage(damage), jumpTime(jumpTime), lastJump(0)
		{ }

		Enemy(Enemy* baseClass, Vec3 pos) :
			Enemy(*baseClass)
		{
			this->baseClass = baseClass;
			this->pos = pos;
		}

		unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) override
		{
			return make_unique<Enemy>(this, pos);
		}

		inline bool MUpdate()
		{
			return MUpdate(static_cast<EnemyData*>(data)->mUpdate);
		}
		bool MUpdate(MUPDATE tempMUpdate)
		{
			return mUpdates[UnEnum(tempMUpdate)](this);
		}

		inline bool AUpdate()
		{
			return AUpdate(static_cast<EnemyData*>(data)->aUpdate);
		}
		bool AUpdate(AUPDATE tempAUpdate)
		{
			return aUpdates[UnEnum(tempAUpdate)](this);
		}

		virtual int Cost()
		{
			return points;
		}

		virtual Vec3 BitePos()
		{
			return pos + dir * radius * 0.2f;
		}

		virtual float BiteRad()
		{
			return radius * 0.9f;
		}

		virtual void ShouldTryJump()
		{
			if (tTime - lastJump > jumpTime && game->entities->OverlapsAny(BitePos(), BiteRad(), MaskF::IsCorporealNotCollectible, this))
			{
				vel += Vec3(0, 0, 6);
				lastJump = tTime;
			}
		}

		void MoveDir()
		{
			vel = Vec3(TryAdd2V2(vel, Vec2(dir) * game->dTime * (speed + game->planet->friction),
				maxSpeed), vel.z);
		}

		Vec3 FindDir()
		{
			Entity* entity = NearestFoe(pos, 30.f, this);
			return Normalized((entity == nullptr ? reinterpret_cast<Entity*>(game->base) : entity)->pos - pos);
		}
	};


	EnemyData deceiverData = EnemyData(MUPDATE::DEFAULT, AUPDATE::DEFAULT, UPDATE::ENEMY, VUPDATE::FRICTION, DUPDATE::DECEIVER, EDUPDATE::ENTITY, UIUPDATE::ENEMY, ONDEATH::ENEMY);
	class Deceiver : public Enemy
	{
	public:
		RGBA color3;

		Deceiver(EntityData* data, Allegiance allegiance = 0, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1, int firstWave = 1, int damage = 1, float radius = 0.5f,
			RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, timePer, speed, maxSpeed, jumpTime, points, firstWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name), color3(color3)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<Deceiver> newEnemy = make_unique<Deceiver>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->color = RGBA::RandNormalized(newEnemy->color.a);
			newEnemy->Start();
			return std::move(newEnemy);
		}
	};

	EnemyData parentData = EnemyData(MUPDATE::DEFAULT, AUPDATE::DEFAULT, UPDATE::ENEMY, VUPDATE::FRICTION, DUPDATE::PARENT, EDUPDATE::ENTITY, UIUPDATE::ENEMY, ONDEATH::PARENT);
	class Parent : public Enemy
	{
	public:
		Enemy* child;

		Parent(EntityData* data, Enemy* child, Allegiance allegiance = 0, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, timePer, speed, maxSpeed, jumpTime, points, firstWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name), child(child)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<Parent> newEnemy = make_unique<Parent>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
		}
	};

	EnemyData exploderData = EnemyData(MUPDATE::DEFAULT, AUPDATE::EXPLODER, UPDATE::ENEMY, VUPDATE::FRICTION, DUPDATE::EXPLODER, EDUPDATE::ENTITY, UIUPDATE::ENEMY, ONDEATH::EXPLODER);
	class Exploder : public Enemy
	{
	public:
		float explosionRadius;

		Exploder(EntityData* data, float explosionRadius, Allegiance allegiance = 0, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, timePer, speed, maxSpeed, jumpTime, points, firstWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name), explosionRadius(explosionRadius)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<Exploder> newEnemy = make_unique<Exploder>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}

		Vec3 ExplosionPos()
		{
			return pos + down * radius;
		}
	};

	EnemyData snakeData = EnemyData(MUPDATE::SNAKE, AUPDATE::DEFAULT, UPDATE::ENEMY, VUPDATE::SNAKE, DUPDATE::DTOCOL, EDUPDATE::ENTITY, UIUPDATE::ENEMY, ONDEATH::SNAKE);
	class Snake : public Enemy
	{
	public:
		RGBA color3, color4;
		Snake* back = nullptr, * front = nullptr;
		int length;
		float segmentWobbleForce, segmentWobbleFrequency;

		Snake(EntityData* data, int length, float segmentWobbleForce, float segmentWobbleFrequency, Allegiance allegiance = 0, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, timePer, speed, maxSpeed, jumpTime, points, firstWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name), length(length), color3(color3),
			color4(color4), segmentWobbleForce(segmentWobbleForce), segmentWobbleFrequency(segmentWobbleFrequency)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<Snake>* enemies = new unique_ptr<Snake>[length];
			enemies[0] = make_unique<Snake>(*this);
			enemies[0]->baseClass = baseClass;
			enemies[0]->pos = pos;
			enemies[0]->Start();
			enemies[0]->radius = radius + segmentWobbleForce * sinf(static_cast<float>(0) * segmentWobbleFrequency);
			enemies[0]->color = color.CLerp(color4, sinf(static_cast<float>(0)) * 0.5f + 0.5f);
			enemies[0]->lastTime = tTime;
			for (int i = 1; i < length; i++)
			{
				enemies[i] = make_unique<Snake>(*this);
				enemies[i]->baseClass = baseClass;
				enemies[i]->radius = radius + segmentWobbleForce * sinf(static_cast<float>(i) * segmentWobbleFrequency);
				enemies[i]->pos = enemies[i - 1]->pos + up * (enemies[i - 1]->radius + enemies[i]->radius);
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

	EnemyData pouncerSnakeData = EnemyData(MUPDATE::POUNCERSNAKE, AUPDATE::DEFAULT, UPDATE::ENEMY, VUPDATE::SNAKE, DUPDATE::DTOCOL, EDUPDATE::ENTITY, UIUPDATE::ENEMY, ONDEATH::POUNCERSNAKE);
	class PouncerSnake : public Snake
	{
	public:
		float pounceTime;

		PouncerSnake(EntityData* data, float pounceTime, float speed, float jumpTime, int length, float segmentWobbleForce, float segmentWobbleFrequency, Allegiance allegiance = 0, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Snake(data, length, segmentWobbleForce, segmentWobbleFrequency, allegiance, timePer, speed, speed, jumpTime, points, firstWave,
				damage, radius, color, color2, color3, color4, mass, bounciness, maxHealth, health, name),
			pounceTime(pounceTime)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<PouncerSnake>* enemies = new unique_ptr<PouncerSnake>[length];
			enemies[0] = make_unique<PouncerSnake>(*this);
			enemies[0]->baseClass = baseClass;
			enemies[0]->pos = pos;
			enemies[0]->Start();
			enemies[0]->radius = radius + segmentWobbleForce * sinf(static_cast<float>(0) * segmentWobbleFrequency);
			enemies[0]->color = color.CLerp(color4, sinf(static_cast<float>(0)) * 0.5f + 0.5f);
			enemies[0]->lastTime = tTime;
			for (int i = 1; i < length; i++)
			{
				enemies[i] = make_unique<PouncerSnake>(*this);
				enemies[i]->baseClass = baseClass;
				enemies[i]->radius = radius + segmentWobbleForce * sinf(static_cast<float>(i) * segmentWobbleFrequency);
				enemies[i]->pos = enemies[i - 1]->pos + up * (enemies[i - 1]->radius + enemies[i]->radius);
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

	EnemyData snakeConnectedData = EnemyData(MUPDATE::SNAKECONNECTED, AUPDATE::DEFAULT, UPDATE::ENEMY, VUPDATE::SNAKECONNECTED, DUPDATE::SNAKECONNECTED, EDUPDATE::ENTITY, UIUPDATE::SNAKECONNECTED, ONDEATH::SNAKECONNECTED);
	class SnakeConnected : public Enemy
	{
	public:
		RGBA color3, color4;
		SnakeConnected* back = nullptr, *front = nullptr, *next = nullptr;
		int length;
		float segmentWobbleForce, segmentWobbleFrequency;

		SnakeConnected(EntityData* data, int length, float segmentWobbleForce, float segmentWobbleFrequency, Allegiance allegiance = 0, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, timePer, speed, maxSpeed, jumpTime, points, firstWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name), length(length), color3(color3),
			color4(color4), segmentWobbleForce(segmentWobbleForce), segmentWobbleFrequency(segmentWobbleFrequency)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<SnakeConnected>* enemies = new unique_ptr<SnakeConnected>[length];
			enemies[0] = make_unique<SnakeConnected>(*this);
			enemies[0]->baseClass = baseClass;
			enemies[0]->pos = pos;
			enemies[0]->Start();
			enemies[0]->radius = radius + segmentWobbleForce * sinf(static_cast<float>(0) * segmentWobbleFrequency);
			enemies[0]->color = color.CLerp(color4, sinf(static_cast<float>(0)) * 0.5f + 0.5f);
			enemies[0]->lastTime = tTime;
			for (int i = 1; i < length; i++)
			{
				enemies[i] = make_unique<SnakeConnected>(*this);
				enemies[i]->baseClass = baseClass;
				enemies[i]->radius = radius + segmentWobbleForce * sinf(static_cast<float>(i) * segmentWobbleFrequency);
				enemies[i]->pos = enemies[i - 1]->pos + up * (enemies[i - 1]->radius + enemies[i]->radius);
				enemies[i]->Start();
				enemies[i]->color = color.CLerp(color4, sinf(static_cast<float>(i)) * 0.5f + 0.5f);
				enemies[i]->lastTime = tTime;
			}
			// Front:
			enemies[length - 1]->color = color3;
			enemies[length - 1]->front = enemies[length - 1].get();
			enemies[length - 1]->next = nullptr;
			enemies[length - 1]->back = enemies[length - 2].get();
			enemies[length - 1]->observers.resize(size_t(length) - 1);
			// Last:
			enemies[0]->front = enemies[length - 1].get();
			enemies[length - 1]->observers[0] = enemies[0].get();
			enemies[0]->next = enemies[1].get();
			for (int i = 1; i < length - 1; i++) // From second to last to second to front.
			{
				enemies[i]->front = enemies[length - 1].get();
				enemies[length - 1]->observers[i] = enemies[i].get();
				enemies[i]->back = enemies[i - 1].get();
				enemies[i]->next = enemies[i + 1].get();
			}

			for (int i = 0; i < length - 1; i++) // Back to front for loop. Does not do 0.
				game->entities->push_back(std::move(enemies[i]));
			return std::move(enemies[length - 1]); // Do 0 here.
		}

		void UnAttach(Entity* entity) override
		{
			if (entity == front)
			{
				DelayedDestroySelf();
				health = 0; // Set to 0 but may be further decreased before frame ends.
			}
		}

		int ApplyHit(int damage, Entity* damageDealer) override
		{
			if (health <= 0) return 1;
			if (next != nullptr)
				return front->ApplyHit(damage, damageDealer);

			int result = Enemy::ApplyHitHarmless(damage, damageDealer);
			if (result == 1)
			{
				health = 0;
				OnDeath(damageDealer);
				DelayedDestroySelf();
			}
			return result;
		}
	};

	EnemyData colorCyclerData = EnemyData(MUPDATE::DEFAULT, AUPDATE::DEFAULT, UPDATE::ENEMY, VUPDATE::FRICTION, DUPDATE::COLORCYCLER, EDUPDATE::ENTITY, UIUPDATE::ENEMY, ONDEATH::ENEMY);
	class ColorCycler : public Enemy
	{
	public:
		vector<RGBA> colorsToCycle;
		float colorOffset, colorCycleSpeed;

		ColorCycler(EntityData* data, vector<RGBA> colorsToCycle, float colorCycleSpeed = 1.0f, Allegiance allegiance = 0, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1,
			int firstWave = 1, int damage = 1, float radius = 0.5f, RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, timePer, speed, maxSpeed, jumpTime, points, firstWave, damage, radius, colorsToCycle[0], color2, mass, bounciness, maxHealth, health, name),
			colorsToCycle(colorsToCycle), colorCycleSpeed(colorCycleSpeed), colorOffset(0.0f)
		{ }

		void Start() override
		{
			Enemy::Start();
			colorOffset = RandFloat() * colorCycleSpeed;
		}

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<ColorCycler> newEnemy = make_unique<ColorCycler>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}
	};

	EnemyData vacuumerData = EnemyData(MUPDATE::VACUUMER, AUPDATE::DEFAULT, UPDATE::VACUUMER, VUPDATE::FRICTION, DUPDATE::DTOCOL, EDUPDATE::ENTITY, UIUPDATE::ENEMY, ONDEATH::VACUUMER);
	class Vacuumer : public Enemy
	{
	public:
		float vacDist, vacSpeed, maxVacSpeed, desiredDistance;
		Items items;

		Vacuumer(EntityData* data, float vacDist, float vacSpeed, float maxVacSpeed, float desiredDistance, Allegiance allegiance = 0, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1,
			int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, timePer, speed, maxSpeed, jumpTime, points, firstWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name),
			vacDist(vacDist), vacSpeed(vacSpeed), maxVacSpeed(maxVacSpeed), desiredDistance(desiredDistance), items(0)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<Vacuumer> newEnemy = make_unique<Vacuumer>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}
	};

	EnemyData spiderData = EnemyData(MUPDATE::DEFAULT, AUPDATE::DEFAULT, UPDATE::SPIDER, VUPDATE::FRICTION, DUPDATE::DTOCOL, EDUPDATE::SPIDER, UIUPDATE::ENEMY, ONDEATH::SPIDER);
	class Spider : public Enemy
	{
	public:
		vector<LegParticle*> legs;
		vector<float> lastLegUpdates;
		LegParticle baseLeg;
		int legCount;
		float legLength, legTolerance, legCycleSpeed;
		float legRotation = 0.0f;

		Spider(EntityData* data, LegParticle baseLeg, int legCount = 8, float legLength = 5.0f, float legTolerance = 3.0f, float legCycleSpeed = 1.0f,
			Allegiance allegiance = 0, float timePer = 0.5f, float moveSpeed = 2.0f, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, timePer, moveSpeed, maxSpeed, jumpTime, points, firstWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name),
			baseLeg(baseLeg), legCount(legCount), legLength(legLength), legTolerance(legTolerance), legCycleSpeed(legCycleSpeed)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
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
					Vec3 desiredPos = LegPos(i) + dir * legTolerance * 0.5f;
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

		Spoobderb(EntityData* data, Entity* baseChild, LegParticle baseLeg, int legCount = 8, float legLength = 5.0f, float legTolerance = 3.0f, float legCycleSpeed = 1.0f,
			Allegiance allegiance = 0, float timePer = 0.5f, float moveSpeed = 2.0f, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			baseChild(baseChild), Spider(data, baseLeg, legCount, legLength, legTolerance, legCycleSpeed,
				allegiance, timePer, moveSpeed, maxSpeed, jumpTime, points, firstWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name) { }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<Spoobderb> newEnemy = make_unique<Spoobderb>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
		}

		int ApplyHit(int damage, Entity* damageDealer) override
		{
			for (int i = 0; i < damage; i++)
				if ((health - i) % 10 == 0)
					game->entities->push_back(baseChild->Clone(pos - Vec3(radius + 1) + Vec3(RandFloat() * ((radius + 1) * 2), RandFloat() * ((radius + 1) * 2), RandFloat() * ((radius + 1) * 2)), north, this));
			return Spider::ApplyHit(damage, damageDealer);
		}
	};

	EnemyData centicrawlerData = EnemyData(MUPDATE::CENTICRAWLER, AUPDATE::DEFAULT, UPDATE::CENTICRAWLER, VUPDATE::FRICTION, DUPDATE::DTOCOL, EDUPDATE::SPIDER, UIUPDATE::ENEMY, ONDEATH::CENTICRAWLER);
	class Centicrawler : public Spider
	{
	public:
		Centicrawler* back = nullptr, * front = nullptr;
		float segmentWobbleForce, segmentWobbleFrequency;

		Centicrawler(EntityData* data, float segmentWobbleForce, float segmentWobbleFrequency, LegParticle baseLeg, int legCount = 8, float legLength = 5.0f, float legTolerance = 3.0f, float legCycleSpeed = 1.0f,
			Allegiance allegiance = 0, float timePer = 0.5f, float moveSpeed = 2.0f, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Spider(data, baseLeg, legCount, legLength, legTolerance, legCycleSpeed, allegiance, timePer, moveSpeed, maxSpeed, jumpTime, points, firstWave, damage, radius, color, color2,
				mass, bounciness, maxHealth, health, name), segmentWobbleForce(segmentWobbleForce), segmentWobbleFrequency(segmentWobbleFrequency)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
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

	EnemyData pouncerData = EnemyData(MUPDATE::POUNCER, AUPDATE::DEFAULT, UPDATE::POUNCER, VUPDATE::FRICTION, DUPDATE::POUNCER, EDUPDATE::ENTITY, UIUPDATE::ENEMY, ONDEATH::ENEMY);
	class Pouncer : public Enemy
	{
	public:
		float pounceTime;

		Pouncer(EntityData* data, float pounceTime, float moveSpeed = 2, float jumpTime = 0.5f, Allegiance allegiance = 0, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, timePer, moveSpeed, moveSpeed, jumpTime, points, firstWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name),
			pounceTime(pounceTime)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<Pouncer> newEnemy = make_unique<Pouncer>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = FindDir();
			newEnemy->Start();
			return std::move(newEnemy);
		}
	};

	EnemyData catData = EnemyData(MUPDATE::CAT, AUPDATE::DEFAULT, UPDATE::CAT, VUPDATE::FRICTION, DUPDATE::CAT, EDUPDATE::ENTITY, UIUPDATE::ENEMY, ONDEATH::ENEMY);
	class Cat : public Pouncer
	{
	public:
		float homeSpeed;
		RGBA color3;

		Cat(EntityData* data, float homeSpeed, float pounceTime, float moveSpeed = 2, float jumpTime = 0.5f, Allegiance allegiance = 0, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Pouncer(data, pounceTime, moveSpeed, jumpTime, allegiance, timePer, timePerMove, points, firstWave, damage, radius, color, color2, mass, bounciness, maxHealth, health,
				name), homeSpeed(homeSpeed), color3(color3)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<Cat> newEnemy = make_unique<Cat>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = FindDir();
			newEnemy->Start();
			return std::move(newEnemy);
		}

		int ApplyHit(int damage, Entity* damageDealer) override
		{
			return Enemy::ApplyHit(Clamp(damage, -1, 1), damageDealer);
		}
	};

	EnemyData boomCatData = EnemyData(MUPDATE::CAT, AUPDATE::BOOMCAT, UPDATE::CAT, VUPDATE::FRICTION, DUPDATE::CAT, EDUPDATE::ENTITY, UIUPDATE::ENEMY, ONDEATH::ENEMY);
	class BoomCat : public Cat
	{
	public:
		float explosionRadius;

		BoomCat(EntityData* data, float explosionRadius, float homeSpeed, float pounceTime, float moveSpeed = 2, float jumpTime = 0.5f, Allegiance allegiance = 0, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Cat(data, homeSpeed, pounceTime, moveSpeed, jumpTime, allegiance, timePer, timePerMove, points, firstWave, damage, radius, color, color2, color3,
				mass, bounciness, maxHealth, health, name), explosionRadius(explosionRadius)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<BoomCat> newEnemy = make_unique<BoomCat>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = FindDir();
			newEnemy->Start();
			return std::move(newEnemy);
		}
	};

	EnemyData cataclysmData = EnemyData(MUPDATE::CAT, AUPDATE::BOOMCAT, UPDATE::CATACLYSM, VUPDATE::FRICTION, DUPDATE::CATACLYSM, EDUPDATE::ENTITY, UIUPDATE::ENEMY, ONDEATH::ENEMY);
	class Cataclysm : public BoomCat
	{
	public:
		float lastStartedCircle = -100, circleTime, circleRadius, spinSpeed;
		float lastShoot = -100, timePerShot;
		Projectile* projectile, *projectile2;
		RGBA color4;

		Cataclysm(EntityData* data, float circleTime, float circleRadius, float spinSpeed, Projectile* projectile, Projectile* projectile2, float timePerShot, float explosionRadius,
			float homeSpeed, float pounceTime, float moveSpeed, float jumpTime = 0.5f, Allegiance allegiance = 0, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			BoomCat(data, explosionRadius, homeSpeed, pounceTime, moveSpeed, jumpTime, allegiance, timePer, timePerMove, points, firstWave, damage, radius, color, color2, color3,
				mass, bounciness, maxHealth, health, name), circleTime(circleTime), circleRadius(circleRadius), spinSpeed(spinSpeed), projectile(projectile),
			projectile2(projectile2), timePerShot(timePerShot), color4(color4)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<Cataclysm> newEnemy = make_unique<Cataclysm>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = FindDir();
			newEnemy->Start();
			return std::move(newEnemy);
		}

		int ApplyHit(int damage, Entity* damageDealer) override
		{
			if (tTime - lastStartedCircle > circleTime / difficultyGrowthModifier[game->settings.difficulty])
			{
				if (damage > 0)
				{
					lastStartedCircle = tTime;
				}
				return Cat::ApplyHit(damage, damageDealer);
			}
			return 0;
		}
	};

	EnemyData tankData = EnemyData(MUPDATE::TANK, AUPDATE::TANK, UPDATE::ENEMY, VUPDATE::FRICTION, DUPDATE::TANK, EDUPDATE::ENTITY, UIUPDATE::ENEMY, ONDEATH::ENEMY);
	class Tank : public Enemy
	{
	public:
		Projectile* projectile;
		float turnSpeed;

		Tank(EntityData* data, Projectile* projectile, float turnSpeed, Allegiance allegiance = 0, float timePer = 0.5f, float moveSpeed = 0.5f, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1,
			int firstWave = 1, int damage = 1, float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, timePer, moveSpeed, maxSpeed, jumpTime, points, firstWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name),
			projectile(projectile), turnSpeed(turnSpeed)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<Tank> newEnemy = make_unique<Tank>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = FindDir();
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
				if (currentCenticrawler != nullptr && currentCenticrawler != centicrawler && currentCenticrawler->baseClass == centicrawler->baseClass && currentCenticrawler->back == nullptr && currentCenticrawler != farthestBack)
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
			pouncer->ShouldTryJump();
		}

		void CatU(Entity* entity)
		{
			Cat* cat = static_cast<Cat*>(entity);

			cat->dir = RotateTowardsNorm(cat->dir, cat->FindDir(), game->dTime * cat->homeSpeed);
			cat->MoveDir();

			cat->Update(UPDATE::POUNCER);
		}

		void CataclysmU(Entity* entity)
		{
			Cataclysm* cat = static_cast<Cataclysm*>(entity);

			/*if (tTime - cat->lastShoot > cat->timePerShot && tTime - cat->lastStartedCircle >= cat->circleTime && tTime - cat->lastStartedCircle < cat->circleTime / difficultyGrowthModifier[game->settings.difficulty])
			{
				game->entities->push_back(cat->projectile2->Clone(cat->pos, glm::rotateZ(cat->FindDir() * cat->projectile2->range +
					Vec3(0, 0, cat->projectile2->radius), PI_F * 0.125f), cat));
				game->entities->push_back(cat->projectile2->Clone(cat->pos, cat->FindDir() * cat->projectile2->range +
					Vec3(0, 0, cat->projectile2->radius), cat));
				game->entities->push_back(cat->projectile2->Clone(cat->pos, glm::rotateZ(cat->FindDir() * cat->projectile2->range +
					Vec3(0, 0, cat->projectile2->radius), PI_F * -0.125f), cat));
			}
			
			//cat->corporeal = true;
			if (tTime - cat->lastStartedCircle < cat->circleTime)
			{
				//cat->corporeal = false;
				float rotation = (tTime - cat->lastStartedCircle) * cat->spinSpeed;
				cat->SetPos(game->PlayerPos() + Vec3(sinf(rotation) * cat->circleRadius, cosf(rotation) * cat->circleRadius, 0));
			
				if (tTime - cat->lastShoot > cat->timePerShot)
				{
					cat->lastShoot = tTime;
					int count = cat->maxHealth - cat->health + 1;
					Vec3 pos = cat->pos;

					for (int i = 1; i < count; i++)
					{
						pos = Vec3(glm::rotate(Vec2(pos - game->PlayerPos()), PI_F * 2 / count) + Vec2(game->PlayerPos()), pos.z);
						game->entities->push_back(cat->projectile->Clone(pos, game->PlayerPos() - pos +
							Vec3(0, 0, cat->projectile->radius), cat));
					}

					game->entities->push_back(cat->projectile->Clone(cat->pos, game->PlayerPos() - cat->pos, cat));
				}

				cat->dir = Normalized(game->PlayerPos() - cat->pos);
				cat->vel = cat->dir * cat->speed;
			}
			else */cat->Update(UPDATE::CAT);
		}
	}

	namespace VUpdates
	{
		void SnakeVU(Entity* entity)
		{
			Snake* snake = static_cast<Snake*>(entity);

			if (snake->front == nullptr && snake->back)
			{
				Snake* currentBack;
				for (int i = 0; i < 5; i++)
				{
					snake->back->SetPos(snake->pos + Normalized(snake->back->pos - snake->pos) * (snake->radius + snake->back->radius));
					currentBack = snake->back->back;
					while (currentBack)
					{
						Entity* front = currentBack->front;
						float dist = glm::distance(front->pos, currentBack->pos);
						Vec3 multiplier = (front->pos - currentBack->pos) * (1.01f * (dist - currentBack->radius - front->radius) / (dist * (currentBack->mass + front->mass)));
						currentBack->SetPos(currentBack->pos + multiplier * front->mass);
						front->SetPos(front->pos - multiplier * currentBack->mass);
						currentBack = currentBack->back;
					}
				}
			}
			snake->VUpdate(VUPDATE::FRICTION);
		}

		void SnakeConnectedVU(Entity* entity)
		{
			SnakeConnected* snake = static_cast<SnakeConnected*>(entity);

			if (snake->next == nullptr)
			{
				for (int i = 0; i < 5; i++)
				{
					SnakeConnected* currentFarthest = snake->back;
					while (currentFarthest)
					{
						Entity* front = currentFarthest->next;
						float dist = glm::distance(front->pos, currentFarthest->pos);
						Vec3 multiplier = (front->pos - currentFarthest->pos) * (1.01f * (dist - currentFarthest->radius - front->radius) / (dist * (currentFarthest->mass + front->mass)));
						currentFarthest->SetPos(currentFarthest->pos + multiplier * front->mass);
						front->SetPos(front->pos - multiplier * currentFarthest->mass);

						currentFarthest = currentFarthest->back;
					}
				}
			}
			snake->VUpdate(VUPDATE::FRICTION);
		}
	}

	namespace DUpdates
	{
		void DeceiverDU(Entity* entity)
		{
			Deceiver* deceiver = static_cast<Deceiver*>(entity);

			deceiver->DUpdate(DUPDATE::DTOCOL);

			RGBA tempColor = deceiver->color;
			Vec3 tempPos = deceiver->pos;

			deceiver->color.r = static_cast<byte>(deceiver->color.r / 255.f * deceiver->color3.r);
			deceiver->color.g = static_cast<byte>(deceiver->color.g / 255.f * deceiver->color3.g);
			deceiver->color.b = static_cast<byte>(deceiver->color.b / 255.f * deceiver->color3.b);
			deceiver->color.a = static_cast<byte>(deceiver->color.a / 255.f * deceiver->color3.a);
			
			Vec3 playerPos = game->PlayerPos();
			deceiver->pos = playerPos + glm::rotateZ(deceiver->pos - playerPos, glm::radians(90.f));
			deceiver->DUpdate(DUPDATE::DTOCOL);
			deceiver->pos = playerPos + glm::rotateZ(deceiver->pos - playerPos, glm::radians(90.f));
			deceiver->DUpdate(DUPDATE::DTOCOL);
			deceiver->pos = playerPos + glm::rotateZ(deceiver->pos - playerPos, glm::radians(90.f));
			deceiver->DUpdate(DUPDATE::DTOCOL);

			deceiver->color = tempColor;
			deceiver->pos = tempPos;
		}

		void ParentDU(Entity* entity)
		{
			Parent* parent = static_cast<Parent*>(entity);
			parent->DUpdate(DUPDATE::DTOCOL); // Modify to actually draw now that rendering's been changed.
			float drawHeight = parent->radius * sqrtf(0.75f), drawDist = parent->radius * 0.5f;
			parent->child->Draw(parent->pos + Vec3(0, drawDist, drawHeight));
			parent->child->Draw(parent->pos + Vec3(-drawDist, 0, drawHeight));
			parent->child->Draw(parent->pos + Vec3(0, -drawDist, drawHeight));
			parent->child->Draw(parent->pos + Vec3(drawDist, 0, drawHeight));
		}

		void ExploderDU(Entity* entity)
		{
			Exploder* exploder = static_cast<Exploder*>(entity);
			float tempRadius = exploder->radius;
			Vec3 tempPos = exploder->pos;
			exploder->pos = exploder->ExplosionPos();
			exploder->radius = exploder->explosionRadius;
			byte tempAlpha = exploder->color.a;
			exploder->color.a /= 5;
			exploder->DUpdate(DUPDATE::DTOCOL);
			exploder->pos = tempPos;
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
			game->DrawCone(pouncer->pos + pouncer->dir * ratio,
				pouncer->pos + pouncer->dir * (ratio * 2), pouncer->color, ratio);

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
				cat->pos = Vec3(glm::rotate(Vec2(cat->pos) - Vec2(game->PlayerPos()), PI_F * 2 / count) + Vec2(game->PlayerPos()), cat->pos.z);
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
			if (tTime - cat->lastStartedCircle < cat->circleTime / difficultyGrowthModifier[game->settings.difficulty])
				cat->color = cat->color4.CLerp(cat->color, sinf(tTime * 4 * PI_F) * 0.5f + 0.5f);
			cat->DUpdate(DUPDATE::CAT);
			cat->color = tempColor;
		}

		void TankDU(Entity* entity)
		{
			Tank* tank = static_cast<Tank*>(entity);
			Vec3 rightDir = RotateRight(tank->dir) * tank->radius, upDir = tank->dir * tank->radius, forwardDir = tank->dir * tank->radius;
			float treadRadius = tank->radius * 0.33333f;
			float drawHeight = tank->radius * sqrtf(0.75f), drawDist = tank->radius * 0.5f;
			Vec3 dir = glm::normalize(glm::cross(up, tank->dir));
			//Vec3 a = drawHeight * dir + drawDist * tank->dir;
			game->DrawCircle(tank->pos + rightDir + forwardDir * -0.33333f, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos + rightDir + forwardDir * -0.16666f, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos + rightDir, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos + rightDir + forwardDir * 0.16666f, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos + rightDir + forwardDir * 0.33333f, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos - rightDir + forwardDir * -0.33333f, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos - rightDir + forwardDir * -0.16666f, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos - rightDir, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos - rightDir + forwardDir * 0.16666f, tank->projectile->color, treadRadius);
			game->DrawCircle(tank->pos - rightDir + forwardDir * 0.33333f, tank->projectile->color, treadRadius);

			game->DrawCylinder(tank->pos, tank->pos + upDir * 1.5f, tank->projectile->color, treadRadius);

			tank->DUpdate(DUPDATE::DTOCOL);
		}
	}

	namespace EDUpdates
	{
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

			game->entities->push_back(parent->child->Clone(parent->pos + north));
			game->entities->push_back(parent->child->Clone(parent->pos + west));
			game->entities->push_back(parent->child->Clone(parent->pos + south));
			game->entities->push_back(parent->child->Clone(parent->pos + east));
		}

		void ExploderOD(Entity* entity, Entity* damageDealer)
		{
			Exploder* exploder = static_cast<Exploder*>(entity);
			exploder->OnDeath(ONDEATH::ENEMY, damageDealer);
			// Will break tiles downwards.
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
			for (ItemInstance item : vacuumer->items)
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
			enemy->dir = enemy->FindDir();
			enemy->MoveDir();
			enemy->ShouldTryJump();
			return false;
		}
	
		bool SnakeMU(Enemy* enemy)
		{
			Snake* snake = static_cast<Snake*>(enemy);
			if (snake->front == nullptr)
			{
				snake->dir = snake->FindDir();
				snake->ShouldTryJump();
			}
			else
				snake->dir = Normalized(snake->front->pos - snake->pos);
			snake->MoveDir();
			return false;
		}

		bool PouncerSnakeMU(Enemy* enemy)
		{
			PouncerSnake* pSnake = static_cast<PouncerSnake*>(enemy);
			pSnake->dir = pSnake->FindDir();
			if (pSnake->front == nullptr)
				pSnake->vel = pSnake->dir * pSnake->speed + up * 12.f;
			return true;
		}

		bool SnakeConnectedMU(Enemy* enemy)
		{
			SnakeConnected* snake = static_cast<SnakeConnected*>(enemy);
			if (snake->next == nullptr)
			{
				snake->dir = snake->FindDir();
				snake->ShouldTryJump();
			}
			else
				snake->dir = Normalized(snake->next->pos - snake->pos);
			snake->MoveDir();
			return false;
		}

		bool VacuumerMU(Enemy* enemy)
		{
			Vacuumer* vacuumer = static_cast<Vacuumer*>(enemy);
			//float distance = glm::distance(game->PlayerPos(), vacuumer->pos);
			vacuumer->dir = vacuumer->FindDir();//Normalized(game->PlayerPos() - vacuumer->pos) * (distance > vacuumer->desiredDistance ? 1.f : -1.f);
			vacuumer->MoveDir();
			vacuumer->ShouldTryJump();
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
			pouncer->dir = pouncer->FindDir();
			pouncer->vel = pouncer->dir * pouncer->speed;
			return true;
		}

		bool CatMU(Enemy* enemy)
		{
			Cat* cat = static_cast<Cat*>(enemy);
			cat->vel = cat->dir * cat->speed;
			return true;
		}

		bool TankMU(Enemy* enemy)
		{
			Tank* tank = static_cast<Tank*>(enemy);

			Vec3 dDir = tank->FindDir();
			tank->dir = Vec3(RotateTowardsNorm2(tank->dir, dDir, tank->turnSpeed * game->dTime), 0);
			if (glm::dot(tank->dir, dDir) > 0.75f)
				tank->MoveDir();
			tank->ShouldTryJump();
			return false;
		}
	}

	namespace AUpdates
	{
		bool DefaultAU(Enemy* enemy)
		{
			return game->entities->TryDealDamage(enemy->damage, enemy->BitePos(), enemy->BiteRad(), MaskF::IsNonAlly, enemy);
		}

		bool ExploderAU(Enemy* enemy)
		{
			Exploder* exploder = static_cast<Exploder*>(enemy);
			if (!game->entities->DoesOverlap(exploder->ExplosionPos(), exploder->explosionRadius, MaskF::IsNonAlly, exploder))
				return false;
			CreateExplosion(exploder->ExplosionPos(), exploder->explosionRadius, exploder->color, exploder->name, 0, exploder->damage, exploder);
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
			game->entities->push_back(tank->projectile->Clone(tank->pos + tank->dir * tank->radius * 1.5f, tank->dir * tank->projectile->range, tank));
			return true;
		}
	}
#pragma endregion


#pragma region Enemies
#pragma region Wild enemies
	// Prerequisites
	LegParticle spiderLeg = LegParticle(vZero, nullptr, RGBA(0, 0, 0, 255), 32.0f, 0.25f);
	LegParticle miniSpiderLeg = LegParticle(vZero, nullptr, RGBA(0, 0, 0, 255), 32.0f, 0.125f);
	Centicrawler centicrawler = Centicrawler(&centicrawlerData, 0.1f, 1, spiderLeg, 3, 3.0f, 0.25f, 1.0f, ENEMY1_A, 0.5f, 4, 4, 0.5f, 0, 0, 10, 0.5f, RGBA(186, 7, 66), RGBA(), 1, 0, 60, 60, "Centicrawler");
	Centicrawler miniCenticrawler = Centicrawler(&centicrawlerData, 0.1f, 1, miniSpiderLeg, 3, 3.0f, 0.25f, 1.0f, ENEMY1_A, 0.5f, 2, 4, 0.5f, 0, 0, 10, 0.25f, RGBA(186, 7, 66), RGBA(), 0.5f, 0, 20, 20, "Mini Centicrawler");
	// Earlies - 1
	Spider spider = Spider(&spiderData, spiderLeg, 6, 3.0f, 0.25f, 1.0f, ENEMY1_A, 0.5f, 4, 4, 0.5f, 2, 1, 10, 0.5f, RGBA(79, 0, 26), RGBA(), 1, 0, 20, 20, "Spider");
	// Mids - 4
	Pouncer frog = Pouncer(&pouncerData, 2, 16, 0.5f, ENEMY1_A, 1, 4, 3, 4, 10, 0.5f, RGBA(107, 212, 91), RGBA(), 3, 0.5f, 30, 30, "Frog");
	Parent miniSpiderParent = Parent(&parentData, &miniCenticrawler, ENEMY1_A, 1, 1, 1, 0.5f, 4, 4, 10, 1, RGBA(140, 35, 70), RGBA(), 3, 0, 30, 30, "Mini Spider Parent");
	// Mid-lates - 6
	Parent spiderParent = Parent(&parentData, &centicrawler, ENEMY1_A, 1, 1, 1, 0.5f, 5, 6, 10, 2.5f, RGBA(140, 35, 70), RGBA(), 5, 0, 100, 100, "Spider Parent");
	SnakeConnected snake = SnakeConnected(&snakeConnectedData, 15, 0.1f, 1, ENEMY1_A, 0.5f, 4, 4, 0.5f, 8, 6, 10, 0.5f, RGBA(0, 255), RGBA(), RGBA(255, 255), RGBA(0, 127), 2, 0, 300, 300, "Snake");
	// Lates - 8
	SnakeConnected bigSnake = SnakeConnected(&snakeConnectedData, 30, 0.3f, 1, ENEMY1_A, 0.5f, 4, 4, 0.5f, 30, 8, 10, 1.f, RGBA(0, 255), RGBA(), RGBA(255, 255), RGBA(0, 127), 5, 0, 900, 900, "Big Snake");
	PouncerSnake pouncerSnake = PouncerSnake(&pouncerSnakeData, 3.0f, 24.0f, 0.5f, 30, 0.1f, 1, ENEMY1_A, 0.5f, 8.0f, 60, 8, 10, 0.5f, RGBA(0, 0, 255), RGBA(), RGBA(0, 255, 255), RGBA(0, 0, 127), 2, 0, 30, 30, "Pouncer Snake");
	// Very lates - 12
	Cat cat = Cat(&catData, 1.5f, 2.0f, 16.0f, 0.5f, ENEMY1_A, 0.25f, 3.0f, 45, 12, 10, 0.5f, RGBA(209, 96, 36), RGBA(), RGBA(186, 118, 82), 1, 0, 18, 18, "Cat");
	BoomCat boomCat = BoomCat(&boomCatData, 4.5f, 1.5f, 2.0f, 12.0f, 0.5f, ENEMY1_A, 1.0f, 4.0f, 45, 12, 10, 1.5f, RGBA(255, 120, 97), RGBA(), RGBA(158, 104, 95), 9, 0, 18, 18, "Boom Cat");
	Spoobderb spoobderb = Spoobderb(&spiderData, &centicrawler, spiderLeg, 30, 25.0f, 3.0f, 2.5f, ENEMY1_A, 0.5f, 2, 2, 0.5f, 50, 12, 10, 3.5f, RGBA(77, 14, 35), RGBA(), 50, 0, 1000, 1000, "Spoobderb - The 30 footed beast");
#pragma endregion
#pragma region Faction 1
	//Predefinitions - Special
	Projectile tinyTankProjectile = Projectile(&projectileData, 15.0f, 10, 8.0f, 0.5f, RGBA(51, 51, 51), 1, 0, 1, 1, "Tiny Tank Projectile");
	ExplodeOnLanding gigaTankItem = ExplodeOnLanding(ITEMTYPE::GIGA_TANK_ITEM, 7.5f, 10, "Giga Tank Item", "Ammo", 1, RGBA(65, 224, 150), 0, 30, 0.25f, 30.f, 0.4f);
	ShotItem gigaTankProjectile = ShotItem(&shotItemData, gigaTankItem.Clone(), "Giga Tank Projectile");
	Enemy child = Enemy(&enemyData, ENEMY2_A, 1.0f, 12, 12, 0.5f, 0, 0, 10, 0.5f, RGBA(255, 0, 255), RGBA(), 1, 0, 5, 5, "Child");
	
	// Earlies - 1
	Enemy walker = Enemy(&enemyData, ENEMY2_A, 0.75f, 2, 2, 0.5f, 1, 1, 10, 0.5f, RGBA(0, 255, 255), RGBA(), 1, 0, 30, 30, "Walker");
	Enemy tanker = Enemy(&enemyData, ENEMY2_A, 1.0f, 1.5f, 1.5f, 0.5f, 2, 1, 10, 1.5f, RGBA(255), RGBA(), 5, 0, 120, 120, "Tanker");
	Tank tinyTank = Tank(&tankData, &tinyTankProjectile, 0.78f, ENEMY2_A, 1.0f, 4, 4, 0.5f, 3, 10, 1, 0.5f, RGBA(127, 127), RGBA(), 1, 0, 30, 30, "Tiny Tank");

	// Mids - 4
	Deceiver deceiver = Deceiver(&deceiverData, ENEMY2_A, 0.5f, 4, 4, 0.5f, 4, 4, 10, 0.5f, RGBA(255, 255, 255), RGBA(), RGBA(192, 192, 192, 153), 1, 0, 30, 30, "Deceiver");
	Exploder exploder = Exploder(&exploderData, 2.5f, ENEMY2_A, 1.0f, 3, 3, 0.5f, 4, 4, 10, 0.5f, RGBA(153, 255, 0), RGBA(), 1, 0, 30, 30, "Exploder");
	Vacuumer vacuumer = Vacuumer(&vacuumerData, 4, 16, 2, 8, ENEMY2_A, 0.125f, 8, 8, 0.5f, 3, 4, 0, 0.5f, RGBA(255, 255, 255), RGBA(), 1, 0, 30, 30, "Vacuumer");
	Vacuumer pusher = Vacuumer(&vacuumerData, 6, -32, 2, 2, ENEMY2_A, 0.125f, 8, 8, 0.5f, 3, 4, 0, 0.5f, RGBA(255, 153, 255), RGBA(), 1, 0, 30, 30, "Pusher");
	
	// Mid-lates - 6
	Parent parent = Parent(&parentData, &child, ENEMY2_A, 1, 1, 1, 0.5f, 4, 6, 10, 2.5f, RGBA(127, 0, 127), RGBA(), 1, 0, 100, 100, "Parent");
	Enemy megaTanker = Enemy(&enemyData, ENEMY2_A, 1, 1, 1, 0.5f, 20, 6, 10, 2.5f, RGBA(174, 0, 255), RGBA(), 10, 0, 480, 480, "Mega Tanker");
	
	// Lates - 8
	ColorCycler hyperSpeedster = ColorCycler(&colorCyclerData, { RGBA(255), RGBA(255, 255), RGBA(0, 0, 255) }, 2.0f, ENEMY2_A, 0.5f, 3, 6, 0.5f, 8, 8, 10, 0.5f, RGBA(), 1, 0, 240, 240, "Hyper Speedster");
	Exploder gigaExploder = Exploder(&exploderData, 7.5f, ENEMY2_A, 1.0f, 4, 4, 0.5f, 8, 8, 10, 1.5f, RGBA(153, 255), RGBA(), 1, 0, 30, 30, "Giga Exploder");
	
	// Very lates - 12
	Tank gigaTank = Tank(&tankData, &gigaTankProjectile, 0.78f, ENEMY2_A, 2, 2, 16, 0.5f, 120, 1, 0, 3, RGBA(201, 193, 119), RGBA(), 15, 0, 600, 600, "Giga Tank");
#pragma endregion

	// Bosses - Special
	Projectile* catProjectile = new Projectile(&projectileData, 25.0f, 10, cat.speed, 0.4f, cat.color, 1, 0, 1, 1, "Cataclysmic Bullet", false, true);
	Projectile* catProjectile2 = new Projectile(&projectileData, 25.0f, 10, cat.speed * 2, 0.4f, cat.color, 1, 0, 1, 1, "Cataclysmic Bullet", false, true);
	Cataclysm* cataclysm = new Cataclysm(&cataclysmData, 10.0f, 25.0f, PI_F / 5, catProjectile, catProjectile2, 0.0625f, 6.5f, 0.5f, 4.0f, 12.0f, 0.5f, ENEMY1_A, 0.5f, 5.0f, 1000, 0, 10, 3.5f, RGBA(), RGBA(), RGBA(158, 104, 95), RGBA(127), 50, 0, 9, 9, "Cataclysm - The nine lived feind");
#pragma endregion


#pragma region Spawning
	class Instance;
	class Types : public vector<vector<Enemy*>>
	{
	public:
		using vector<vector<Enemy*>>::vector;

		static int GetRoundPoints()
		{
			if (game->settings.difficulty == DIFFICULTY::EASY)
				return static_cast<int>(pow(1.25f, waveCount)) + waveCount * 2 - 1;
			else if (game->settings.difficulty == DIFFICULTY::MEDIUM)
				return static_cast<int>(pow(1.37f, waveCount)) + waveCount * 3 - 1;
			else // difficulty == hard
				return static_cast<int>(pow(1.45f, waveCount)) + waveCount * 5 + 3;
		}

		static float GetPoints()
		{
			float time = tTime2 / 60;
			if (game->settings.difficulty == DIFFICULTY::EASY)
				return pow(1.25f, time) + time * 2 - 1;
			else if (game->settings.difficulty == DIFFICULTY::MEDIUM)
				return pow(1.37f, time) + time * 3 - 1;
			else // difficulty == hard
				return pow(1.45f, time) + time * 5 + 3;
		}

		Instance RandomClone();
	};

	int superWaveModifiers[] = { 3, 4, 5 }, difficultySeedSuperWaveCount[] = { 5, 4, 3 };

	class Instance : public vector<Enemy*>
	{
	public:
		using vector<Enemy*>::vector;
		bool superWave = false;

		void SpawnEnemy(int index)
		{
			float randomValue = RandFloat() * 6.283184f;
			game->entities->push_back((*this)[index]->Clone(Vec3(RandCircPoint2(), 0) * 30.f * (RandFloat() * 0.5f + 1.5f)
				+ game->PlayerPos() + up * 30.f));
		}

		void SpawnRandomEnemies(float multiplier = 1)
		{
			((Entity*)game->player)->ApplyHit(-3, nullptr);
			if (superWave)
				waveCount += superWaveModifiers[game->settings.difficulty];

			SpawnRandomEnemiesCost(static_cast<int>(multiplier * Types::GetRoundPoints()));



			if (superWave)
			{
				waveCount -= superWaveModifiers[game->settings.difficulty];
				for (int i = 0; i < difficultySeedSuperWaveCount[game->settings.difficulty]; i++)
					game->entities->push_back(Collectibles::Seeds::plantSeeds[rand() % Collectibles::Seeds::plantSeeds.size()]->Clone(game->PlayerPos()));
				superWave = false;
			}
		}

		void SpawnOneRandom()
		{
			SpawnEnemy(rand() % size());
		}

		void SpawnRandomEnemiesCost(int cost)
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

	class OvertimeInstance : public Instance
	{
	public:
		int usedPoints = 0;
		uint nextSpawnIndex = 0, nextSpawnCount = 1;
		uint groupSpawnMin = 3, groupSpawnMax = 10;

		void Randomize()
		{
			int currentlySpawnableEnemyCount = 1;
			for (int i = 1; i < (*this).size(); i++)
				currentlySpawnableEnemyCount += int((*this)[i]->firstWave <= waveCount);
			vector<uint> currentlySpawnableEnemies(currentlySpawnableEnemyCount);
			currentlySpawnableEnemies[0] = 0;
			for (int i = 1, j = 1; j < currentlySpawnableEnemyCount; i++)
				if ((*this)[i]->firstWave <= waveCount)
					currentlySpawnableEnemies[j++] = i;
			nextSpawnIndex = currentlySpawnableEnemies[rand() % currentlySpawnableEnemies.size()];
			nextSpawnCount = RandRangeInt(groupSpawnMin, groupSpawnMax);
		}

		void Update()
		{
			if (Types::GetPoints() - usedPoints > (*this)[nextSpawnIndex]->Cost() * nextSpawnCount)
			{
				for (int i = 0; i < nextSpawnCount; i++)
					SpawnEnemy(nextSpawnIndex);
				usedPoints += (*this)[nextSpawnIndex]->Cost() * nextSpawnCount;
				Randomize();
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

	Types wildSpawns
	{
		{&spider},
		{&frog, &miniSpiderParent},
		{&spiderParent, &snake},
		{&bigSnake, &pouncerSnake},
		{&cat, &boomCat, &spoobderb}
	};

	Types faction1Spawns
	{
		{&walker, &tanker, &tinyTank},
		{&deceiver, &exploder, &vacuumer, &pusher},
		{&parent, &megaTanker},
		{&hyperSpeedster, &gigaExploder},
		{&gigaTank}
	};

	Types spawnableBosses
	{
		{cataclysm}
	};
#pragma endregion
}