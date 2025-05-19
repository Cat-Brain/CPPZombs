#include "BuildingBlocks.h"

namespace Enemies
{
#pragma region Enemy types
#pragma region Type Definitions
	// The base class of all enemies.
	class Enemy;
	typedef function<bool(Enemy*)> MUpdate, AUpdate;

	class EnemyData : public EntityData
	{
	public:
		MUpdate mUpdate;
		AUpdate aUpdate;

		EnemyData(MUpdate mUpdate, AUpdate aUpdate, Update update = ::Updates::EntityU, VUpdate vUpdate = ::VUpdates::EntityVU,
			DUpdate dUpdate = ::DUpdates::EntityDU, UIUpdate uiUpdate = ::UIUpdates::EntityUIU, OnDeath onDeath = ::OnDeaths::EntityOD) :
			EntityData(update, vUpdate, dUpdate, uiUpdate, onDeath),
			mUpdate(mUpdate), aUpdate(aUpdate) {}
	};

	constexpr float ENEMY_SIGHT_DIST = 30.f;
	class Enemy : public DToCol
	{
	public:
		Entity* defaultTarget = nullptr, *currentTarget = nullptr;
		Mesh2* mesh;
		float timePer, lastTime, speed, maxSpeed, timePerMove, lastMove, jumpTime, lastJump;

		int points, firstWave, lastWave;
		int damage;

		Enemy(EntityData* data, Allegiance allegiance = 0, Mesh2* mesh = nullptr, float timePer = 0.5f, float speed = 0.25f, float maxSpeed = 0.25f,
			float jumpTime = 0.5f, int points = 1, int firstWave = -1, int lastWave = -1, int damage = 1, float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			DToCol(data, vZero, radius, color, color2, mass, bounciness, maxHealth, health, name, allegiance),
			mesh(mesh), timePer(timePer), lastTime(0.0f), speed(speed), maxSpeed(maxSpeed), timePerMove(0.0f), lastMove(0.0f), points(points),
			firstWave(firstWave), lastWave(lastWave), damage(damage), jumpTime(jumpTime), lastJump(0)
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

		inline bool MUpd()
		{
			return static_cast<EnemyData*>(data)->mUpdate(this);
		}
		bool MUpd(MUpdate tempMUpdate)
		{
			return tempMUpdate(this);
		}

		inline bool AUpd()
		{
			return static_cast<EnemyData*>(data)->aUpdate(this);
		}
		bool AUpd(AUpdate tempAUpdate)
		{
			return tempAUpdate(this);
		}

		virtual int Cost()
		{
			return points;
		}

		bool ValidWave(int wave)
		{
			return (firstWave == -1 || wave >= firstWave) && (lastWave == -1 || wave <= lastWave);
		}

		virtual Vec3 BitePos()
		{
			return pos + dir * radius * 0.2f;
		}

		virtual Vec3 BitePos2()
		{
			return pos + Vec3(Normalized2((Vec2)dir) * radius * 0.2f, 0);
		}

		virtual float BiteRad()
		{
			return radius * 0.9f;
		}

		virtual void ShouldTryJump()
		{
			if (tTime - lastJump > jumpTime && game->entities->OverlapsAny(BitePos2(), BiteRad(), MaskF::IsAlly, this))
			{
				vel += Vec3(0, 0, 6);
				lastJump = tTime;
			}
		}

		virtual void ShouldTryJumpBoth()
		{
			if (tTime - lastJump > jumpTime && game->entities->OverlapsAny(BitePos(), BiteRad(), MaskF::IsCorporealNotCollectible, this))
			{
				Vec3 jumpForce = Vec3(0, 0, 6);
				vel += jumpForce;
				Entity* overlap = game->entities->FirstOverlap(BitePos(), BiteRad(), MaskF::IsCorporealNotCollectible, this);
				if (overlap != nullptr)
					overlap->vel -= jumpForce * (mass / overlap->mass);
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
			std::pair<Entity*, float> result = game->entities->ExtremestOverlap(pos, ENEMY_SIGHT_DIST, MaskF::IsNonAlly, ExtrF::DistMin, this);
			if (defaultTarget == nullptr && result.first == nullptr) return up;
			currentTarget = result.first == nullptr ? defaultTarget : result.first;
			return Normalized(currentTarget->pos - pos);
		}
	};


	class Deceiver : public Enemy
	{
	public:
		RGBA color3;

		Deceiver(EntityData* data, Allegiance allegiance = 0, Mesh2* mesh = nullptr, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f,
			float jumpTime = 0.5f, int points = 1, int firstWave = -1, int lastWave = -1, int damage = 1, float radius = 0.5f,
			RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, mesh, timePer, speed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass, bounciness,
				maxHealth, health, name), color3(color3)
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

	class Parent : public Enemy
	{
	public:
		Enemy* child;

		Parent(EntityData* data, Enemy* child, Allegiance allegiance = 0, Mesh2* mesh = nullptr, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f,
			float jumpTime = 0.5f, int points = 1, int firstWave = -1, int lastWave = -1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, mesh, timePer, speed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name), child(child)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<Parent> newEnemy = make_unique<Parent>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
		}

		int Cost() override
		{
			return points + child->points * 4;
		}
	};

	class Exploder : public Enemy
	{
	public:
		float explosionRadius;

		Exploder(EntityData* data, float explosionRadius, Allegiance allegiance = 0, Mesh2* mesh = nullptr, float timePer = 0.5f, float speed = 2,
			float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1, int firstWave = -1, int lastWave = -1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, mesh, timePer, speed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass, bounciness,
				maxHealth, health, name), explosionRadius(explosionRadius)
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

	class Snake : public Enemy
	{
	public:
		RGBA color3, color4;
		Snake* back = nullptr, * front = nullptr;
		int length;
		float segmentWobbleForce, segmentWobbleFrequency;

		Snake(EntityData* data, int length, float segmentWobbleForce, float segmentWobbleFrequency, Allegiance allegiance = 0, Mesh2* mesh = nullptr,
			float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1, int firstWave = -1, int lastWave = -1,
			int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, mesh, timePer, speed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name), length(length), color3(color3),
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
			return points * length;
		}
	};

	class PouncerSnake : public Snake
	{
	public:
		float pounceTime;

		PouncerSnake(EntityData* data, float pounceTime, float speed, float jumpTime, int length, float segmentWobbleForce, float segmentWobbleFrequency,
			Allegiance allegiance = 0, Mesh2* mesh = nullptr, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = -1, int lastWave = -1,
			int damage = 1, float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Snake(data, length, segmentWobbleForce, segmentWobbleFrequency, allegiance, mesh, timePer, speed, speed, jumpTime, points, firstWave, lastWave,
				damage, radius, color, color2, color3, color4, mass, bounciness, maxHealth, health, name),
			pounceTime(pounceTime)
		{
			this->timePerMove = timePerMove;
		}

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

	class SnakeConnected : public Enemy
	{
	public:
		RGBA color3, color4;
		SnakeConnected* back = nullptr, *front = nullptr, *next = nullptr;
		int length;
		float segmentWobbleForce, segmentWobbleFrequency;

		SnakeConnected(EntityData* data, int length, float segmentWobbleForce, float segmentWobbleFrequency, Allegiance allegiance = 0,
			Mesh2* mesh = nullptr, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1,
			int firstWave = -1, int lastWave = -1, int damage = 1, float radius = 0.5f,
			RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, mesh, timePer, speed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name), length(length), color3(color3),
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

		HitResult ApplyHit(int damage, Entity* damageDealer) override
		{
			if (health <= 0) return HitResult::DIED;
			if (next != nullptr)
				return front->ApplyHit(damage, damageDealer);

			HitResult result = Enemy::ApplyHitHarmless(damage, damageDealer);
			if (result == HitResult::DIED)
			{
				health = 0;
				OnDeath(damageDealer);
				DelayedDestroySelf();
			}
			return result;
		}
	};

	class ColorCycler : public Enemy
	{
	public:
		vector<RGBA> colorsToCycle;
		float colorOffset, colorCycleSpeed;

		ColorCycler(EntityData* data, vector<RGBA> colorsToCycle, float colorCycleSpeed = 1.0f, Allegiance allegiance = 0, Mesh2* mesh = nullptr,
			float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1,
			int firstWave = -1, int lastWave = -1, int damage = 1, float radius = 0.5f, RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, mesh, timePer, speed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, colorsToCycle[0], color2, mass, bounciness, maxHealth, health, name),
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

	class Vacuumer : public Enemy
	{
	public:
		float vacDist, vacSpeed, maxVacSpeed, desiredDistance;
		Items items;

		Vacuumer(EntityData* data, float vacDist, float vacSpeed, float maxVacSpeed, float desiredDistance, Allegiance allegiance = 0,
			Mesh2* mesh = nullptr, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1,
			int firstWave = -1, int lastWave = -1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, mesh, timePer, speed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name),
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
			Allegiance allegiance = 0, Mesh2* mesh = nullptr, float timePer = 0.5f, float moveSpeed = 2.0f, float maxSpeed = 0.25f,
			float jumpTime = 0.5f, int points = 1, int firstWave = -1, int lastWave = -1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, mesh, timePer, moveSpeed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name),
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
				unique_ptr<LegParticle> newLeg = make_unique<LegParticle>(baseLeg);
				newLeg->parent = this;
				newLeg->desiredPos = LegPos(i);
				newLeg->pos = newLeg->desiredPos;
				lastLegUpdates[i] = offsettedTTime + i * legCycleSpeed / legCount;
				legs[i] = newLeg.get();
				game->entities->particles.push_back(std::move(newLeg));
			}
		}

		Vec3 LegPos(int index)
		{
			float rotation = (float(index) / legCount + legRotation) * 2 * PI_F;
			return pos + down * radius + Vec3(CircPoint2(rotation), 0) * legLength;
		}

		void UpdateLegs()
		{
			for (int i = 0; i < legCount; i++)
				if (tTime - lastLegUpdates[i] > legCycleSpeed)
				{
					lastLegUpdates[i] += legCycleSpeed;
					Vec3 desiredPos = LegPos(i) + dir * legTolerance * 0.5f;
					if (glm::distance2(legs[i]->desiredPos, desiredPos) > legTolerance * legTolerance)
						legs[i]->desiredPos = desiredPos;
				}
		}
	};

	class Spoobderb : public Spider
	{
	public:
		Entity* baseChild;

		Spoobderb(EntityData* data, Entity* baseChild, LegParticle baseLeg, int legCount = 8, float legLength = 5.0f, float legTolerance = 3.0f,
			float legCycleSpeed = 1.0f,
			Allegiance allegiance = 0, Mesh2* mesh = nullptr, float timePer = 0.5f, float moveSpeed = 2.0f, float maxSpeed = 0.25f, float jumpTime = 0.5f,
			int points = 1, int firstWave = -1, int lastWave = -1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			baseChild(baseChild), Spider(data, baseLeg, legCount, legLength, legTolerance, legCycleSpeed,
				allegiance, mesh, timePer, moveSpeed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass, bounciness, maxHealth,
				health, name) { }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<Spoobderb> newEnemy = make_unique<Spoobderb>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
		}

		HitResult ApplyHit(int damage, Entity* damageDealer) override
		{
			for (int i = 0; i < damage; i++)
				if ((health - i) % 10 == 0)
				{
					unique_ptr<Entity> spider = baseChild->Clone(pos - Vec3(radius + 1) + Vec3(RandFloat() * ((radius + 1) * 2), RandFloat() * ((radius + 1) * 2), RandFloat() * ((radius + 1) * 2)), north, this);
					((Enemy*)spider.get())->defaultTarget = defaultTarget;
					game->entities->push_back(std::move(spider));
				}
			return Spider::ApplyHit(damage, damageDealer);
		}
	};

	class Centicrawler : public Spider
	{
	public:
		Centicrawler* back = nullptr, * front = nullptr;
		float segmentWobbleForce, segmentWobbleFrequency;

		Centicrawler(EntityData* data, float segmentWobbleForce, float segmentWobbleFrequency, LegParticle baseLeg, int legCount = 8, float legLength = 5.0f,
			float legTolerance = 3.0f, float legCycleSpeed = 1.0f, Allegiance allegiance = 0, Mesh2* mesh = nullptr, float timePer = 0.5f, float moveSpeed = 2.0f,
			float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1, int firstWave = -1, int lastWave = -1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Spider(data, baseLeg, legCount, legLength, legTolerance, legCycleSpeed, allegiance, mesh, timePer, moveSpeed, maxSpeed, jumpTime, points,
				firstWave, lastWave, damage, radius, color, color2,
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

	class Pouncer : public Enemy
	{
	public:
		float pounceTime;

		Pouncer(EntityData* data, float pounceTime, float moveSpeed = 2, float jumpTime = 0.5f, Allegiance allegiance = 0, Mesh2* mesh = nullptr,
			float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = -1, int lastWave = -1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, mesh, timePer, moveSpeed, moveSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass, bounciness,
				maxHealth, health, name),
			pounceTime(pounceTime)
		{
			this->timePerMove = timePerMove;
		}

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

	class Cat : public Pouncer
	{
	public:
		float homeSpeed;
		RGBA color3;

		Cat(EntityData* data, float homeSpeed, float pounceTime, float moveSpeed = 2, float jumpTime = 0.5f, Allegiance allegiance = 0,
			Mesh2* mesh = nullptr, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = -1, int lastWave = -1, int damage = 1,
			float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Pouncer(data, pounceTime, moveSpeed, jumpTime, allegiance, mesh, timePer, timePerMove, points, firstWave, lastWave, damage, radius, color, color2,
				mass, bounciness, maxHealth, health, name), homeSpeed(homeSpeed), color3(color3)
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

		HitResult ApplyHit(int damage, Entity* damageDealer) override
		{
			return Enemy::ApplyHit(Clamp(damage, -1, 1), damageDealer);
		}
	};

	class BoomCat : public Cat
	{
	public:
		float explosionRadius;

		BoomCat(EntityData* data, float explosionRadius, float homeSpeed, float pounceTime, float moveSpeed = 2, float jumpTime = 0.5f,
			Allegiance allegiance = 0, Mesh2* mesh = nullptr, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = -1, int lastWave = -1,
			int damage = 1, float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Cat(data, homeSpeed, pounceTime, moveSpeed, jumpTime, allegiance, mesh, timePer, timePerMove, points, firstWave, lastWave, damage, radius,
				color, color2, color3, mass, bounciness, maxHealth, health, name), explosionRadius(explosionRadius)
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

	class Cataclysm : public BoomCat
	{
	public:
		float lastStartedCircle = -100, circleTime, circleRadius, spinSpeed;
		float lastShoot = -100, timePerShot;
		Projectile* projectile, *projectile2;
		RGBA color4;

		Cataclysm(EntityData* data, float circleTime, float circleRadius, float spinSpeed, Projectile* projectile, Projectile* projectile2,
			float timePerShot, float explosionRadius, float homeSpeed, float pounceTime, float moveSpeed, float jumpTime = 0.5f,
			Allegiance allegiance = 0, Mesh2* mesh = nullptr, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = -1, int lastWave = -1,
			int damage = 1, float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			BoomCat(data, explosionRadius, homeSpeed, pounceTime, moveSpeed, jumpTime, allegiance, mesh, timePer, timePerMove, points, firstWave, lastWave, damage, radius, color, color2, color3,
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

		HitResult ApplyHit(int damage, Entity* damageDealer) override
		{
			return HitResult::LIVED;
		}
	};

	class BaseTank : public Enemy
	{
	public:
		float turnSpeed, treadRot = 0, foeDist = 0;

		BaseTank(EntityData* data, float turnSpeed, Allegiance allegiance = 0, Mesh2* mesh = nullptr, float timePer = 0.5f, float moveSpeed = 0.5f, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1,
			int firstWave = -1, int lastWave = -1, int damage = 1, float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, mesh, timePer, moveSpeed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name),
			turnSpeed(turnSpeed)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<BaseTank> newEnemy = make_unique<BaseTank>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = FindDir();
			newEnemy->Start();
			return newEnemy;
		}
	};

	class Tank : public BaseTank
	{
	public:
		Projectile* projectile;

		Tank(EntityData* data, Projectile* projectile, float turnSpeed, Allegiance allegiance = 0, Mesh2* mesh = nullptr, float timePer = 0.5f,
			float moveSpeed = 0.5f, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1,
			int firstWave = -1, int lastWave = -1, int damage = 1, float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			BaseTank(data, turnSpeed, allegiance, mesh, timePer, moveSpeed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name),
			projectile(projectile)
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

	class GenericTank : public BaseTank
	{
	public:
		Entity* projectile;
		float projectileSpeed;

		GenericTank(EntityData* data, Entity* projectile, float projectileSpeed, float turnSpeed, Allegiance allegiance = 0, Mesh2* mesh = nullptr,
			float timePer = 0.5f, float moveSpeed = 0.5f, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1,
			int firstWave = -1, int lastWave = -1, int damage = 1, float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			BaseTank(data, turnSpeed, allegiance, mesh, timePer, moveSpeed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass,
				bounciness, maxHealth, health, name),
			projectile(projectile), projectileSpeed(projectileSpeed)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<GenericTank> newEnemy = make_unique<GenericTank>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = FindDir();
			newEnemy->Start();
			return newEnemy;
		}
	};

	class LaserTank : public BaseTank
	{
	public:
		float maxDist;
		RGBA color3;

		LaserTank(EntityData* data, float maxDist, float turnSpeed, Allegiance allegiance = 0, Mesh2* mesh = nullptr,
			float timePer = 0.5f, float moveSpeed = 0.5f, float maxSpeed = 0.25f, float jumpTime = 0.5f, int points = 1,
			int firstWave = -1, int lastWave = -1, int damage = 1, float radius = 0.5f, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			BaseTank(data, turnSpeed, allegiance, mesh, timePer, moveSpeed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass, bounciness, maxHealth, health, name),
			maxDist(maxDist), color3(color3)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<LaserTank> newEnemy = make_unique<LaserTank>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->dir = FindDir();
			newEnemy->Start();
			return newEnemy;
		}
	};
	
	class Thief : public Enemy
	{
	public:
		Entity* grabbed = nullptr;

		Thief(EntityData* data, Allegiance allegiance = 0, Mesh2* mesh = nullptr, float timePer = 0.5f, float speed = 2, float maxSpeed = 0.25f,
			float jumpTime = 0.5f, int points = 1, int firstWave = -1, int lastWave = -1, int damage = 1, float radius = 0.5f,
			RGBA color = RGBA(), RGBA color2 = RGBA(),
			float mass = 1, float bounciness = 0, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(data, allegiance, mesh, timePer, speed, maxSpeed, jumpTime, points, firstWave, lastWave, damage, radius, color, color2, mass, bounciness,
				maxHealth, health, name)
		{ }

		unique_ptr<Entity> Clone(Vec3 pos, Vec3 dir = north, Entity* creator = nullptr) override
		{
			unique_ptr<Thief> newEnemy = make_unique<Thief>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
		}

		void UnAttach(Entity* entity) override
		{
			if (entity == grabbed)
				grabbed = nullptr;
		}
	};
#pragma endregion
#pragma region Update Functions
	namespace Updates
	{
		void EnemyU(Entity* entity)
		{
			Enemy* enemy = static_cast<Enemy*>(entity);
			if (tTime - enemy->lastMove >= enemy->timePerMove)
				if (enemy->MUpd())
					enemy->lastMove = tTime;

			if (tTime - enemy->lastTime >= enemy->timePer)
				if (enemy->AUpd())
					enemy->lastTime = tTime;
		}

		void VacuumerU(Entity* entity)
		{
			Vacuumer* vacuumer = static_cast<Vacuumer*>(entity);
			EnemyU(entity);

			vacuumer->vel -= game->entities->Vacuum(vacuumer->pos, vacuumer->vacDist, vacuumer->vacSpeed, vacuumer->maxVacSpeed, MaskF::IsDifferent, vacuumer) / vacuumer->mass;

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
			EnemyU(entity);
		}

		void CenticrawlerU(Entity* entity)
		{
			Centicrawler* centicrawler = static_cast<Centicrawler*>(entity);
			SpiderU(entity);

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

			EnemyU(entity);
			pouncer->ShouldTryJump();
		}

		void CatU(Entity* entity)
		{
			Cat* cat = static_cast<Cat*>(entity);

			cat->dir = RotateTowardsNorm(cat->dir, cat->FindDir(), game->dTime * cat->homeSpeed);
			cat->MoveDir();

			PouncerU(entity);
		}

		void CataclysmU(Entity* entity)
		{
			Cataclysm* cat = static_cast<Cataclysm*>(entity);

			/*if (tTime - cat->lastShoot > cat->timePerShot && tTime - cat->lastStartedCircle >= cat->circleTime && tTime - cat->lastStartedCircle < cat->circleTime / difficultyGrowthModifier[game->difficulty])
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
			else */CatU(entity);
		}
	}

	namespace VUpdates
	{
		void SnakeVU(Entity* entity)
		{
			Snake* snake = static_cast<Snake*>(entity);

			if (snake->front == nullptr && snake->back != nullptr)
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
			::VUpdates::FrictionVU(entity);
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
			::VUpdates::FrictionVU(entity);
		}

		void SpiderVU(Entity* entity)
		{
			Spider* spider = static_cast<Spider*>(entity);
			spider->UpdateLegs();
			::VUpdates::FrictionVU(entity);
		}

		void CenticrawlerVU(Entity* entity)
		{
			Centicrawler* centicrawler = static_cast<Centicrawler*>(entity);

			if (centicrawler->front == nullptr)
			{
				for (int i = 0; i < 5; i++)
				{
					Centicrawler* currentFarthest = centicrawler->back;
					while (currentFarthest)
					{
						Entity* front = currentFarthest->front;
						float dist = glm::distance(front->pos, currentFarthest->pos);
						Vec3 multiplier = (front->pos - currentFarthest->pos) * (1.01f * (dist - currentFarthest->radius - front->radius) / (dist * (currentFarthest->mass + front->mass)));
						currentFarthest->SetPos(currentFarthest->pos + multiplier * front->mass);
						front->SetPos(front->pos - multiplier * currentFarthest->mass);

						currentFarthest = currentFarthest->back;
					}
				}
			}
			SpiderVU(entity);
		}

		void ThiefVU(Entity* entity)
		{
			Thief* thief = static_cast<Thief*>(entity);

			if (thief->grabbed != nullptr)
			{
				float dist = glm::distance(thief->grabbed->pos, thief->pos);
				Vec3 multiplier = (thief->grabbed->pos - thief->pos) * (1.01f * (dist - thief->radius - thief->grabbed->radius) / (dist * (thief->mass + thief->grabbed->mass)));
				//thief->vel += multiplier * thief->grabbed->mass;
				//thief->grabbed->vel -= multiplier * thief->mass;
				thief->SetPos(thief->pos + multiplier * thief->grabbed->mass);
				thief->grabbed->SetPos(thief->grabbed->pos - multiplier * thief->mass);

			}
			::VUpdates::FrictionVU(entity);
		}
	}

	namespace DUpdates
	{
		void EnemyDU(Entity* entity)
		{
			Enemy* enemy = static_cast<Enemy*>(entity);
			if (enemy->mesh == nullptr)
				return ::DUpdates::DToColDU(entity);
			game->DrawMesh(enemy->mesh, PosScaleRotMat(enemy->pos, Vec3(enemy->radius), glm::vec4(up, atan2f(enemy->dir.y, enemy->dir.x))), enemy->Color());
		}

		void DeceiverDU(Entity* entity)
		{
			Deceiver* deceiver = static_cast<Deceiver*>(entity);

			EnemyDU(entity);

			RGBA tempColor = deceiver->color;
			Vec3 tempPos = deceiver->pos;

			deceiver->color.r = static_cast<byte>(deceiver->color.r / 255.f * deceiver->color3.r);
			deceiver->color.g = static_cast<byte>(deceiver->color.g / 255.f * deceiver->color3.g);
			deceiver->color.b = static_cast<byte>(deceiver->color.b / 255.f * deceiver->color3.b);
			deceiver->color.a = static_cast<byte>(deceiver->color.a / 255.f * deceiver->color3.a);
			
			Vec3 playerPos = game->PlayerPos();
			deceiver->pos = playerPos + glm::rotateZ(deceiver->pos - playerPos, glm::radians(90.f));
			::DUpdates::DToColDU(entity);
			deceiver->pos = playerPos + glm::rotateZ(deceiver->pos - playerPos, glm::radians(90.f));
			::DUpdates::DToColDU(entity);
			deceiver->pos = playerPos + glm::rotateZ(deceiver->pos - playerPos, glm::radians(90.f));
			::DUpdates::DToColDU(entity);

			deceiver->color = tempColor;
			deceiver->pos = tempPos;
		}

		void ParentDU(Entity* entity)
		{
			Parent* parent = static_cast<Parent*>(entity);
			EnemyDU(entity);
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
			EnemyDU(entity);
			exploder->pos = tempPos;
			exploder->radius = tempRadius;
			exploder->color.a = tempAlpha;
			::DUpdates::DToColDU(entity);
		}

		void SnakeConnectedDU(Entity* entity)
		{
			SnakeConnected* snake = static_cast<SnakeConnected*>(entity);
			int tempHealth = snake->health;
			snake->health = snake->front->health;
			EnemyDU(entity);
			snake->health = tempHealth;
		}

		void ColorCyclerDU(Entity* entity)
		{
			ColorCycler* colorCycler = static_cast<ColorCycler*>(entity);
			float currentPlace = (tTime * colorCycler->colorCycleSpeed + colorCycler->colorOffset);
			int intCurrentPlace = static_cast<int>(currentPlace);
			colorCycler->color = colorCycler->colorsToCycle[intCurrentPlace % colorCycler->colorsToCycle.size()].CLerp(
				colorCycler->colorsToCycle[(static_cast<size_t>(intCurrentPlace) + 1) % colorCycler->colorsToCycle.size()], currentPlace - floorf(currentPlace));
			EnemyDU(entity);
		}

		void PouncerDU(Entity* entity)
		{
			Pouncer* pouncer = static_cast<Pouncer*>(entity);

			float ratio = pouncer->radius / SQRTTWO_F;
			game->DrawCone(pouncer->pos + pouncer->dir * ratio,
				pouncer->pos + pouncer->dir * (ratio * 2), pouncer->Color(), ratio);
			EnemyDU(entity);
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
				::DUpdates::DToColDU(entity);
			}

			cat->color = tempColor;
			cat->pos = tempPos;
			EnemyDU(entity);
		}

		void CataclysmDU(Entity* entity)
		{
			Cataclysm* cat = static_cast<Cataclysm*>(entity);
			RGBA tempColor = cat->color;
			EnemyDU(entity);
			cat->color = tempColor;
		}

		void TankDU(Entity* entity)
		{
			Tank* tank = static_cast<Tank*>(entity);
			Vec3 rightDir = glm::normalize(glm::cross(tank->dir, up)), forwardDir = glm::cross(rightDir, up);
			float treadRadius = tank->radius * 0.33333f;
			game->DrawCylinder(tank->pos, tank->pos + tank->dir * tank->radius * 1.5f, tank->projectile->color, treadRadius);

			float rotation = atan2f(rightDir.y, rightDir.x);
			float dist = tank->radius * 0.555555f;
			Vec3 baseTreadPos = Vec3(dist, tank->radius * 0.666666f, 0.f);
			float mul = PI_F * 0.33333f;
			for (int i = 0; i < 6; i++)
				game->DrawCircle(tank->pos + glm::rotateZ(glm::rotateX(baseTreadPos, (i - tank->treadRot) * mul), rotation), tank->projectile->color, treadRadius);
			baseTreadPos.x *= -1;
			for (int i = 0; i < 6; i++)
				game->DrawCircle(tank->pos + glm::rotateZ(glm::rotateX(baseTreadPos, (i - tank->treadRot) * mul), rotation), tank->projectile->color, treadRadius);
			EnemyDU(entity);
		}

		void LaserTankDU(Entity* entity)
		{
			LaserTank* tank = static_cast<LaserTank*>(entity);
			Vec3 rightDir = glm::normalize(glm::cross(tank->dir, up)), forwardDir = glm::cross(rightDir, up);
			float treadRadius = tank->radius * 0.33333f;
			game->DrawCylinder(tank->pos, tank->pos + tank->dir * tank->maxDist, tank->color3, treadRadius);

			float rotation = atan2f(rightDir.y, rightDir.x);
			float dist = tank->radius * 0.555555f;
			Vec3 baseTreadPos = Vec3(dist, tank->radius * 0.666666f, 0.f);
			float mul = PI_F * 0.33333f;
			for (int i = 0; i < 6; i++)
				game->DrawCircle(tank->pos + glm::rotateZ(glm::rotateX(baseTreadPos, (i - tank->treadRot) * mul), rotation), tank->color3, treadRadius);
			baseTreadPos.x *= -1;
			for (int i = 0; i < 6; i++)
				game->DrawCircle(tank->pos + glm::rotateZ(glm::rotateX(baseTreadPos, (i - tank->treadRot) * mul), rotation), tank->color3, treadRadius);
			EnemyDU(entity);
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
			EnemyUIU(entity);
		}
	}

	namespace OnDeaths
	{
		void EnemyOD(Entity* entity, Entity* damageDealer)
		{
			Enemy* enemy = static_cast<Enemy*>(entity);
			totalGamePoints += enemy->points;

			game->entities->push_back(make_unique<FadeOut>(&fadeOutData, 0.5f, entity->pos, entity->radius, enemy->color2));
		}

		void ParentOD(Entity* entity, Entity* damageDealer)
		{
			Parent* parent = static_cast<Parent*>(entity);
			EnemyOD(entity, damageDealer);

			float drawHeight = parent->radius * sqrtf(0.75f), drawDist = parent->radius * 0.5f;
			unique_ptr<Entity> child = parent->child->Clone(parent->pos + Vec3(0, drawDist, drawHeight));
			((Enemy*)child.get())->defaultTarget = parent->defaultTarget;
			game->entities->push_back(std::move(child));
			child = parent->child->Clone(parent->pos + Vec3(-drawDist, 0, drawHeight));
			((Enemy*)child.get())->defaultTarget = parent->defaultTarget;
			game->entities->push_back(std::move(child));
			child = parent->child->Clone(parent->pos + Vec3(0, -drawDist, drawHeight));
			((Enemy*)child.get())->defaultTarget = parent->defaultTarget;
			game->entities->push_back(std::move(child));
			child = parent->child->Clone(parent->pos + Vec3(drawDist, 0, drawHeight));
			((Enemy*)child.get())->defaultTarget = parent->defaultTarget;
			game->entities->push_back(std::move(child));
		}

		void ExploderOD(Entity* entity, Entity* damageDealer)
		{
			Exploder* exploder = static_cast<Exploder*>(entity);
			EnemyOD(entity, damageDealer);
			CreateExplosion(exploder->pos, exploder->explosionRadius, exploder->color, exploder->name, 0, exploder->damage, exploder);
		}

		void SnakeOD(Entity* entity, Entity* damageDealer)
		{
			Snake* snake = static_cast<Snake*>(entity);
			EnemyOD(entity, damageDealer);

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
			EnemyOD(entity, damageDealer);

			if (pSnake->back != nullptr)
				((PouncerSnake*)pSnake->back)->dir = pSnake->dir;
		}

		void SnakeConnectedOD(Entity* entity, Entity* damageDealer)
		{
			SnakeConnected* snake = static_cast<SnakeConnected*>(entity);
			if (snake->next == nullptr)
				EnemyOD(entity, damageDealer);
		}

		void VacuumerOD(Entity* entity, Entity* damageDealer)
		{
			Vacuumer* vacuumer = static_cast<Vacuumer*>(entity);
			EnemyOD(entity, damageDealer);
			for (ItemInstance item : vacuumer->items)
			{
				game->entities->push_back(make_unique<Collectible>(item, vacuumer->pos));
			}
		}

		void SpiderOD(Entity* entity, Entity* damageDealer)
		{
			Spider* spider = static_cast<Spider*>(entity);
			EnemyOD(entity, damageDealer);
			for (LegParticle* leg : spider->legs)
				leg->parent = nullptr;
		}

		void CenticrawlerOD(Entity* entity, Entity* damageDealer)
		{
			Centicrawler* centicrawler = static_cast<Centicrawler*>(entity);
			SpiderOD(entity, damageDealer);
			if (centicrawler->back != nullptr)
				centicrawler->back->front = nullptr;
			if (centicrawler->front != nullptr)
				centicrawler->front->back = nullptr;
		}

		void ThiefOD(Entity* entity, Entity* damageDealer)
		{
			Thief* thief = static_cast<Thief*>(entity);
			
			if (thief->grabbed != nullptr)
				thief->grabbed->RemoveObserver(thief);
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
			{
				centicrawler->dir = centicrawler->FindDir();
				centicrawler->ShouldTryJump();
			}
			else
				centicrawler->dir = Normalized(centicrawler->front->pos - centicrawler->pos);
			centicrawler->MoveDir();
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

		bool BaseTankMU(Enemy* enemy)
		{
			BaseTank* tank = static_cast<BaseTank*>(enemy);

			Vec3 dDir = tank->FindDir();
			tank->dir = Vec3(RotateTowardsNorm2(tank->dir, dDir, tank->turnSpeed * game->dTime) * glm::length(Vec2(dDir)), dDir.z);
			if (glm::dot(tank->dir, dDir) > 0.75f)
				tank->MoveDir();
			tank->ShouldTryJump();

			tank->foeDist = glm::distance(tank->pos, tank->currentTarget->pos);

			tank->treadRot += glm::dot(tank->vel * game->dTime, tank->dir);
			return false;
		}

		bool ThiefMU(Enemy* enemy)
		{
			Thief* thief = static_cast<Thief*>(enemy);

			thief->dir = thief->FindDir();
			if (thief->grabbed != nullptr)
			{
				thief->dir *= -1;
				thief->MoveDir();
				thief->dir *= -1;
				return false;
			}
			thief->MoveDir();
			thief->ShouldTryJump();
			return false;
		}
	}

	namespace AUpdates
	{
		bool DefaultAU(Enemy* enemy)
		{
			return game->entities->TryDealDamage(enemy->damage, enemy->BitePos(), enemy->BiteRad(), MaskF::IsNonAlly, enemy) != HitResult::MISSED;
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
			game->entities->push_back(tank->projectile->Clone(tank->pos + tank->dir * (tank->radius + tank->projectile->radius), tank->dir * tank->projectile->range, tank));
			return true;
		}

		bool MortarTankAU(Enemy* enemy)
		{
			Tank* tank = static_cast<Tank*>(enemy);
			if (tank->foeDist > tank->projectile->range || (tank->dir.x == 0 && tank->dir.y == 0)) return false;
			unique_ptr<Entity> projectile = tank->projectile->Clone(tank->pos + tank->dir * (tank->radius + tank->projectile->radius), tank->dir * tank->projectile->range, tank);
			Vec2 disp = Vec2(glm::length((Vec2)tank->dir) * tank->foeDist, tank->dir.z * tank->foeDist);
			projectile->vel = Vec3(glm::normalize((Vec2)tank->dir) * tank->projectile->speed,
				disp.y * tank->projectile->speed / disp.x + game->planet->gravity * 0.5f * disp.x / tank->projectile->speed);
			game->entities->push_back(std::move(projectile));
			return true;
		}

		bool GenericTankAU(Enemy* enemy)
		{
			GenericTank* tank = static_cast<GenericTank*>(enemy);
			unique_ptr<Entity> newProj = tank->projectile->Clone(tank->pos + tank->dir * (tank->radius + tank->projectile->radius), tank->dir, tank);
			newProj->vel = tank->dir * tank->projectileSpeed;
			game->entities->push_back(std::move(newProj));
			return true;
		}

		bool LaserTankAU(Enemy* enemy)
		{
			LaserTank* tank = static_cast<LaserTank*>(enemy);
			RaycastHit hit = game->entities->RaycastEnt(tank->pos, tank->dir, tank->maxDist, MaskF::IsNonAlly, tank);
			if (hit.dist < tank->maxDist)
				(*game->entities)[hit.index]->ApplyHit(tank->damage, tank);
			return true;
		}

		bool ThiefAU(Enemy* enemy)
		{
			Thief* thief = static_cast<Thief*>(enemy);
			if (thief->grabbed != nullptr)
				return game->entities->TryDealDamage(enemy->damage, enemy->BitePos(), enemy->BiteRad(), MaskF::IsNonAlly, enemy) != HitResult::MISSED;
			thief->grabbed = game->entities->FirstOverlap(enemy->BitePos(), enemy->BiteRad(), MaskF::IsNonAlly, enemy);
			if (thief->grabbed != nullptr)
				thief->grabbed->observers.push_back(thief);
			return false;
		}
	}
#pragma endregion
#pragma region Enemy Datas
	EnemyData enemyData = EnemyData(MUpdates::DefaultMU, AUpdates::DefaultAU, Updates::EnemyU, ::VUpdates::FrictionVU, ::DUpdates::DToColDU, UIUpdates::EnemyUIU, OnDeaths::EnemyOD);
	EnemyData deceiverData = EnemyData(MUpdates::DefaultMU, AUpdates::DefaultAU, Updates::EnemyU, ::VUpdates::FrictionVU, DUpdates::DeceiverDU, UIUpdates::EnemyUIU, OnDeaths::EnemyOD);
	EnemyData parentData = EnemyData(MUpdates::DefaultMU, AUpdates::DefaultAU, Updates::EnemyU, ::VUpdates::FrictionVU, DUpdates::ParentDU, UIUpdates::EnemyUIU, OnDeaths::ParentOD);
	EnemyData exploderData = EnemyData(MUpdates::DefaultMU, AUpdates::ExploderAU, Updates::EnemyU, ::VUpdates::FrictionVU, DUpdates::ExploderDU, UIUpdates::EnemyUIU, OnDeaths::ExploderOD);
	EnemyData snakeData = EnemyData(MUpdates::SnakeMU, AUpdates::DefaultAU, Updates::EnemyU, VUpdates::SnakeVU, ::DUpdates::DToColDU, UIUpdates::EnemyUIU, OnDeaths::SnakeOD);
	EnemyData pouncerSnakeData = EnemyData(MUpdates::PouncerSnakeMU, AUpdates::DefaultAU, Updates::EnemyU, VUpdates::SnakeVU, ::DUpdates::DToColDU, UIUpdates::EnemyUIU, OnDeaths::PouncerSnakeOD);
	EnemyData snakeConnectedData = EnemyData(MUpdates::SnakeConnectedMU, AUpdates::DefaultAU, Updates::EnemyU, VUpdates::SnakeConnectedVU, DUpdates::SnakeConnectedDU, UIUpdates::SnakeConnectedUIU, OnDeaths::SnakeConnectedOD);
	EnemyData colorCyclerData = EnemyData(MUpdates::DefaultMU, AUpdates::DefaultAU, Updates::EnemyU, ::VUpdates::FrictionVU, DUpdates::ColorCyclerDU, UIUpdates::EnemyUIU, OnDeaths::EnemyOD);
	EnemyData vacuumerData = EnemyData(MUpdates::VacuumerMU, AUpdates::DefaultAU, Updates::VacuumerU, ::VUpdates::FrictionVU, ::DUpdates::DToColDU, UIUpdates::EnemyUIU, OnDeaths::VacuumerOD);
	EnemyData spiderData = EnemyData(MUpdates::DefaultMU, AUpdates::DefaultAU, Updates::SpiderU, VUpdates::SpiderVU, ::DUpdates::DToColDU, UIUpdates::EnemyUIU, OnDeaths::SpiderOD);
	EnemyData centicrawlerData = EnemyData(MUpdates::CenticrawlerMU, AUpdates::DefaultAU, Updates::CenticrawlerU, VUpdates::CenticrawlerVU, ::DUpdates::DToColDU, UIUpdates::EnemyUIU, OnDeaths::CenticrawlerOD);
	EnemyData pouncerData = EnemyData(MUpdates::PouncerMU, AUpdates::DefaultAU, Updates::PouncerU, ::VUpdates::FrictionVU, DUpdates::PouncerDU, UIUpdates::EnemyUIU, OnDeaths::EnemyOD);
	EnemyData catData = EnemyData(MUpdates::CatMU, AUpdates::DefaultAU, Updates::CatU, ::VUpdates::FrictionVU, DUpdates::CatDU, UIUpdates::EnemyUIU, OnDeaths::EnemyOD);
	EnemyData boomCatData = EnemyData(MUpdates::CatMU, AUpdates::BoomcatAU, Updates::CatU, ::VUpdates::FrictionVU, DUpdates::CatDU, UIUpdates::EnemyUIU, OnDeaths::EnemyOD);
	EnemyData cataclysmData = EnemyData(MUpdates::CatMU, AUpdates::BoomcatAU, Updates::CataclysmU, ::VUpdates::FrictionVU, DUpdates::CataclysmDU, UIUpdates::EnemyUIU, OnDeaths::EnemyOD);
	EnemyData tankData = EnemyData(MUpdates::BaseTankMU, AUpdates::TankAU, Updates::EnemyU, ::VUpdates::FrictionVU, DUpdates::TankDU, UIUpdates::EnemyUIU, OnDeaths::EnemyOD);
	EnemyData mortarTankData = EnemyData(MUpdates::BaseTankMU, AUpdates::MortarTankAU, Updates::EnemyU, ::VUpdates::FrictionVU, DUpdates::TankDU, UIUpdates::EnemyUIU, OnDeaths::EnemyOD);
	EnemyData genericTankData = EnemyData(MUpdates::BaseTankMU, AUpdates::GenericTankAU, Updates::EnemyU, ::VUpdates::FrictionVU, DUpdates::TankDU, UIUpdates::EnemyUIU, OnDeaths::EnemyOD);
	EnemyData laserTankData = EnemyData(MUpdates::BaseTankMU, AUpdates::LaserTankAU, Updates::EnemyU, ::VUpdates::FrictionVU, DUpdates::LaserTankDU, UIUpdates::EnemyUIU, OnDeaths::EnemyOD);
	EnemyData thiefData = EnemyData(MUpdates::ThiefMU, AUpdates::ThiefAU, Updates::EnemyU, VUpdates::ThiefVU, ::DUpdates::DToColDU, UIUpdates::EnemyUIU, OnDeaths::ThiefOD);
#pragma endregion
#pragma endregion


#pragma region Spawning
	namespace Types
	{
		int GetRoundPoints()
		{
			if (game->difficulty == DIFFICULTY::EASY)
				return static_cast<int>(pow(1.2f, waveCount)) + waveCount * 2 - 2;
			else if (game->difficulty == DIFFICULTY::MEDIUM)
				return static_cast<int>(pow(1.25f, waveCount)) + waveCount * 3 - 2;
			else // difficulty == hard
				return static_cast<int>(pow(1.3f, waveCount)) + waveCount * 7 - 2;
		}

		float GetPoints(float time)
		{
			float time2 = time / 60;
			/*if (game->difficulty == DIFFICULTY::EASY)
				return pow(1.2f, time2) + time2 * 2 + 2;
			else if (game->difficulty == DIFFICULTY::MEDIUM)
				return pow(1.25f, time2) + time2 * 3 + 2;
			else // difficulty == hard
				return pow(1.3f, time2) + time2 * 7 + 2;*/
			if (game->difficulty == DIFFICULTY::EASY)
				return 0.0001f * time2 * time2 * time2 * time2 * time2 + 0.0005f * time2 * time2 * time2 * time2 + 0.01f * time2 * time2 * time2 + time2 * 2 + 2;
			else if (game->difficulty == DIFFICULTY::MEDIUM)
				return 0.00015f * time2 * time2 * time2 * time2 * time2 + 0.00075f * time2 * time2 * time2 * time2 + 0.015f * time2 * time2 * time2 + time2 * 3 + 2;
			else // difficulty == hard
				return 0.00025f * time2 * time2 * time2 * time2 * time2 + 0.001f * time2 * time2 * time2 * time2 + 0.025f * time2 * time2 * time2 + time2 * 7 + 2;
		}
	};

	int superWaveModifiers[] = { 3, 4, 5 }, difficultySeedSuperWaveCount[] = { 5, 4, 3 };

	constexpr float ENEMY_SPAWN_DIST = 50.f;
	constexpr float ENEMY_SPAWN_HEIGHT = 50.f;
	constexpr float SECONDS_PER_WAVE = 60.f;

	class Instance : public vector<Enemy*>
	{
	public:
		int currentWave;
		bool superWave = false;
		Entity* defaultTarget = nullptr;

		Instance(vector<Enemy*> enemies, int currentWave = 0):
			vector<Enemy*>(enemies), currentWave(currentWave) {}

		bool CanSpawn(int index, int cost)
		{
			return (*this)[index]->ValidWave(currentWave) && (*this)[index]->Cost() <= cost;
		}

		void SpawnEnemy(int index)
		{
			float randomValue = RandFloat() * 6.283184f;
			unique_ptr<Entity> newEnemy = (*this)[index]->Clone(Vec3(RandCircPoint2(), 0) * ENEMY_SPAWN_DIST * (RandFloat() * 0.5f + 1.5f)
				+ defaultTarget->pos + up * ENEMY_SPAWN_HEIGHT);
			static_cast<Enemy*>(newEnemy.get())->defaultTarget = defaultTarget;
			game->entities->push_back(std::move(newEnemy));
		}

		void SpawnRandomEnemies(float multiplier = 1)
		{
			((Entity*)game->player)->ApplyHit(-3, nullptr);
			if (superWave)
				waveCount += superWaveModifiers[game->difficulty];

			SpawnRandomEnemiesCost(static_cast<int>(multiplier * Types::GetRoundPoints()));
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
				currentlySpawnableEnemyCount += int(CanSpawn(i, cost));
			vector<Enemy*> currentlySpawnableEnemies(currentlySpawnableEnemyCount);
			currentlySpawnableEnemies[0] = (*this)[0];
			for (int i = 1, j = 1; j < currentlySpawnableEnemyCount; i++)
				if (CanSpawn(i, cost))
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
		float aggro = 0, speedMul = 1;
		float waitTime = 0;
		float timeTillNextWave = 0;
		float timeTillWaveCountIncrease = SECONDS_PER_WAVE;

		float minTime, maxTime;

		OvertimeInstance(vector<Enemy*> enemies, float minTime, float maxTime) :
			Instance(enemies), minTime(minTime), maxTime(maxTime) {}

		void Randomize()
		{
			timeTillNextWave += minTime + RandFloat() * (maxTime - minTime);
		}

		void Update()
		{
			if (waitTime > 0)
			{
				waitTime = max(0.f, waitTime - game->dTime);
				return;
			}
			aggro += game->dTime * speedMul;
			timeTillWaveCountIncrease -= game->dTime * speedMul;
			timeTillNextWave -= game->dTime * speedMul;

			if (timeTillWaveCountIncrease <= 0)
			{
				currentWave++;
				timeTillWaveCountIncrease += SECONDS_PER_WAVE;
			}

			if (timeTillNextWave <= 0)
			{
				int totalPoints = Types::GetPoints(aggro);
				int totalCost = 0, cost = totalPoints - usedPoints;
				if (cost == 0) return;
				int currentlySpawnableEnemyCount = 0;
				for (int i = 0; i < (*this).size(); i++)
					currentlySpawnableEnemyCount += int(CanSpawn(i, cost));

				vector<int> currentlySpawnableEnemies;
				if (currentlySpawnableEnemyCount == 0)
				{
					currentlySpawnableEnemies = vector<int>{ 0 };
					std::cout << "Uh oh! There aren't any good spawnable enemies!\n";
				}
				else
				{
					currentlySpawnableEnemies = vector<int>(currentlySpawnableEnemyCount);
					for (int i = 0, j = 0; j < currentlySpawnableEnemyCount; i++)
						if (CanSpawn(i, cost))
							currentlySpawnableEnemies[j++] = i;
				}
				vector<int> remainingSpawnableEnemies(currentlySpawnableEnemies);
				for (int i = 0; i < currentlySpawnableEnemyCount; i++)
				{
					int currentIndex = rand() % (currentlySpawnableEnemyCount - i);
					int costPer = (*this)[remainingSpawnableEnemies[currentIndex]]->Cost();
					int canSpawnTotal = (cost - totalCost) / costPer;
					if (canSpawnTotal <= 0) continue;
					int randomCount = min(100, rand() % canSpawnTotal);
					for (int j = 0; j < randomCount; j++)
						SpawnEnemy(remainingSpawnableEnemies[currentIndex]);
					totalCost += costPer * randomCount;
					remainingSpawnableEnemies.erase(remainingSpawnableEnemies.begin() + currentIndex);
				}
				if (totalCost == 0) return;
				usedPoints += totalCost;
				Randomize();
			}
		}
	};
#pragma endregion


#pragma region Enemies
#pragma region Wild enemies
	// Prerequisites: Never Spawns Naturally
	LegParticle spiderLeg = LegParticle(vZero, nullptr, RGBA(0, 0, 0, 255), 32.0f, 0.25f);
	LegParticle miniSpiderLeg = LegParticle(vZero, nullptr, RGBA(0, 0, 0, 255), 32.0f, 0.125f);
	Centicrawler centicrawler = Centicrawler(&centicrawlerData, 0.1f, 1, spiderLeg, 3, 3.0f, 0.25f, 1.0f, WILD_A, nullptr, 0.5f, 4, 4, 0.5f, 0, -1, -1, 10, 0.5f, RGBA(186, 7, 66), RGBA(), 1, 0, 60, 60, "Centicrawler");
	Centicrawler miniCenticrawler = Centicrawler(&centicrawlerData, 0.1f, 1, miniSpiderLeg, 3, 3.0f, 0.25f, 1.0f, WILD_A, nullptr, 0.5f, 2, 4, 0.5f, 1, -1, -1, 10, 0.25f, RGBA(186, 7, 66), RGBA(), 0.5f, 0, 20, 20, "Mini Centicrawler");
	// Earlies: 0-10
	Spider spider = Spider(&spiderData, spiderLeg, 6, 3.0f, 0.25f, 1.0f, WILD_A, nullptr, 0.5f, 4, 4, 0.5f, 1, 0, 10, 10, 0.5f, RGBA(79, 0, 26), RGBA(), 1, 0, 20, 20, "Spider");
	SnakeConnected babySnake = SnakeConnected(&snakeConnectedData, 5, 0.1f, 1, WILD_A, nullptr, 0.5f, 3, 3, 0.5f, 2, 0, 10, 10, 0.25f, RGBA(0, 255), RGBA(), RGBA(255, 255), RGBA(0, 127), 0.5f, 0.25f, 120, 120, "Baby Snake");
	Pouncer frog = Pouncer(&pouncerData, 2, 16, 0.5f, WILD_A, nullptr, 1, 4, 2, 0, 10, 10, 0.5f, RGBA(107, 212, 91), RGBA(), 3, 0.5f, 30, 30, "Frog");
	// Mids: 5-15
	Parent miniSpiderParent = Parent(&parentData, &miniCenticrawler, WILD_A, nullptr, 1, 1, 1, 0.5f, 0, 5, 15, 10, 1, RGBA(140, 35, 70), RGBA(), 3, 0, 30, 30, "Mini Spider Parent");
	// Mid-lates: 10-20
	Parent spiderParent = Parent(&parentData, &centicrawler, WILD_A, nullptr, 1, 1, 1, 0.5f, 5, 10, 20, 10, 2.5f, RGBA(140, 35, 70), RGBA(), 5, 0, 100, 100, "Spider Parent");
	SnakeConnected snake = SnakeConnected(&snakeConnectedData, 15, 0.1f, 1, WILD_A, nullptr, 0.5f, 4, 4, 0.5f, 8, 10, 20, 10, 0.5f, RGBA(0, 255), RGBA(), RGBA(255, 255), RGBA(0, 127), 2, 0, 300, 300, "Snake");
	// Lates: 15+
	SnakeConnected bigSnake = SnakeConnected(&snakeConnectedData, 30, 0.3f, 1, WILD_A, nullptr, 0.5f, 4, 4, 0.5f, 30, 15, -1, 10, 1.f, RGBA(0, 255), RGBA(), RGBA(255, 255), RGBA(0, 127), 5, 0, 900, 900, "Big Snake");
	PouncerSnake pouncerSnake = PouncerSnake(&pouncerSnakeData, 3.0f, 24.0f, 0.5f, 5, 0.1f, 1, WILD_A, nullptr, 0.5f, 8.0f, 1, 15, -1, 10, 0.5f, RGBA(0, 0, 255), RGBA(), RGBA(0, 255, 255), RGBA(0, 0, 127), 2, 0, 30, 30, "Pouncer Snake");
	// Very lates: 20+
	Cat cat = Cat(&catData, 1.5f, 2.0f, 16.0f, 0.5f, WILD_A, nullptr, 0.25f, 3.0f, 45, 20, -1, 10, 0.5f, RGBA(209, 96, 36), RGBA(), RGBA(186, 118, 82), 1, 0, 18, 18, "Cat");
	BoomCat boomCat = BoomCat(&boomCatData, 4.5f, 1.5f, 2.0f, 12.0f, 0.5f, WILD_A, nullptr, 1.0f, 4.0f, 45, 20, -1, 10, 1.5f, RGBA(255, 120, 97), RGBA(), RGBA(158, 104, 95), 9, 0, 18, 18, "Boom Cat");
	Spoobderb spoobderb = Spoobderb(&spiderData, &centicrawler, spiderLeg, 30, 25.0f, 3.0f, 2.5f, WILD_A, nullptr, 0.5f, 2, 2, 0.5f, 50, 20, -1, 10, 3.5f, RGBA(77, 14, 35), RGBA(), 50, 0, 1000, 1000, "Spoobderb - The 30 footed beast");

	vector<Enemy*> wildSpawns
	{
		&spider, &babySnake, &frog, &miniSpiderParent, &spiderParent, &snake, &bigSnake/*, &pouncerSnake*/, &cat, &boomCat, &spoobderb
	};
#pragma endregion
#pragma region Faction 1
	//Predefinitions: Never Spawns Naturally
	Projectile tinyTankProjectile = Projectile(&projectileData, 15.0f, 10, 8.0f, 0.5f, ::VUpdates::GravityTrueVU, RGBA(51, 51, 51), 1, 0, 1, 1, "Tiny Tank Projectile");
	ExplodeOnLanding gigaTankItem = ExplodeOnLanding(ITEMTYPE::GIGA_TANK_ITEM, 7.5f, 10, "Giga Tank Item", "Ammo", ::VUpdates::EntityVU, 1, RGBA(65, 224, 150), 0, 30, 0.25f, 30.f, 0.4f);
	ShotItem gigaTankProjectile = ShotItem(&shotItemData, gigaTankItem.Clone(), "Giga Tank Projectile");
	Enemy child = Enemy(&enemyData, COMPANY_A, nullptr, 2.0f, 12, 12, 0.5f, 1, -1, -1, 10, 0.5f, RGBA(255, 0, 255), RGBA(), 1, 0, 1, 1, "Child");
	Enemy larva = Enemy(&enemyData, COMPANY_A, nullptr, 0.5f, 3, 1, 0.5f, 0, 1, -1, 10, 0.25f, RGBA(141, 100, 143), RGBA(), 0.5f, 0, 1, 1, "Larva");
	
	// Earlies: 0 - 8
	Enemy walker = Enemy(&enemyData, COMPANY_A, nullptr, 0.75f, 2, 2, 0.5f, 1, 0, 8, 10, 0.5f, RGBA(0, 255, 255), RGBA(), 1, 0, 30, 30, "Walker");
	Enemy tanker = Enemy(&enemyData, COMPANY_A, nullptr, 1.0f, 1.5f, 1.5f, 0.5f, 2, 0, 8, 10, 1.5f, RGBA(255), RGBA(), 5, 0, 120, 120, "Tanker");
	
	// Mids: 4 - 12
	Tank tinyTank = Tank(&mortarTankData, &tinyTankProjectile, 0.78f, COMPANY_A, nullptr, 1.0f, 4, 4, 0.5f, 3, 4, 12, 0, 0.5f, RGBA(127, 127), RGBA(), 1, 0, 30, 30, "Tiny Tank");
	Deceiver deceiver = Deceiver(&deceiverData, COMPANY_A, nullptr, 0.5f, 4, 4, 0.5f, 4, 4, 12, 10, 0.5f, RGBA(255, 255, 255), RGBA(), RGBA(192, 192, 192, 153), 1, 0, 30, 30, "Deceiver");
	Exploder exploder = Exploder(&exploderData, 2.5f, COMPANY_A, nullptr, 1.0f, 3, 3, 0.5f, 4, 4, 12, 10, 0.5f, RGBA(153, 255, 0), RGBA(), 1, 0, 30, 30, "Exploder");
	Vacuumer vacuumer = Vacuumer(&vacuumerData, 4, 16, 2, 8, COMPANY_A, nullptr, 0.125f, 8, 8, 0.5f, 3, 4, 12, 0, 0.5f, RGBA(255, 255, 255), RGBA(), 1, 0, 30, 30, "Vacuumer");
	Vacuumer pusher = Vacuumer(&vacuumerData, 6, -32, 2, 2, COMPANY_A, nullptr, 0.125f, 8, 8, 0.5f, 3, 4, 12, 0, 0.5f, RGBA(255, 153, 255), RGBA(), 1, 0, 30, 30, "Pusher");
	Thief thief = Thief(&thiefData, COMPANY_A, nullptr, 0.75f, 64, 2, 0.5f, 2, 4, 12, 10, 0.5f, RGBA(145, 145, 118), RGBA(), 1, 0, 30, 30, "Thief");
	
	// Mid-lates: 6 - 14
	Parent parent = Parent(&parentData, &child, COMPANY_A, nullptr, 1, 1, 1, 0.5f, 2, 6, 14, 10, 2.5f, RGBA(127, 0, 127), RGBA(), 10, 0, 100, 100, "Parent");
	Enemy megaTanker = Enemy(&enemyData, COMPANY_A, nullptr, 1, .5, 1, 0.5f, 20, 6, 14, 10, 2.5f, RGBA(174, 0, 255), RGBA(), 25, 0, 1000, 1000, "Mega Tanker");
	LaserTank laserTank = LaserTank(&laserTankData, 10, 0.78f, COMPANY_A, nullptr, 0.1f, 4, 4, 0.5f, 3, 6, 14, 10, 1.5f, RGBA(168, 145, 50), RGBA(), RGBA(156, 79, 31, 63), 1, 0, 120, 120, "Laser Tank");

	// Lates: 8+
	ColorCycler hyperSpeedster = ColorCycler(&colorCyclerData, { RGBA(255), RGBA(255, 255), RGBA(0, 0, 255) }, 2.0f, COMPANY_A, nullptr, 0.5f, 3, 6, 0.5f, 8, 8, -1, 10, 0.5f, RGBA(), 1, 0, 240, 240, "Hyper Speedster");
	Exploder gigaExploder = Exploder(&exploderData, 7.5f, COMPANY_A, nullptr, 1.0f, 4, 4, 0.5f, 8, 8, -1, 10, 1.5f, RGBA(153, 255), RGBA(), 1, 0, 30, 30, "Giga Exploder");
	Parent grandParent = Parent(&parentData, &parent, COMPANY_A, nullptr, 1, 1, 1, 0.5f, 10, 8, -1, 10, 4.5f, RGBA(63, 0, 63), RGBA(), 50, 0, 500, 500, "Grand Parent");

	// Very lates: 12+
	Tank gigaTank = Tank(&tankData, &gigaTankProjectile, 0.78f, COMPANY_A, nullptr, 2, 2, 4, 0.5f, 120, 12, -1, 0, 3, RGBA(201, 193, 119), RGBA(), 15, 0, 600, 600, "Giga Tank");
	GenericTank larvaTank = GenericTank(&genericTankData, &larva, 20, 0.78f, COMPANY_A, nullptr, 1, 2, 4, 0.5f, 120, 12, -1, 0, 3, RGBA(86, 16, 89), RGBA(), 15, 0, 600, 600, "Larva Tank");

	vector<Enemy*> faction1Spawns
	{
		&walker, &tanker, &tinyTank, &deceiver, &exploder, &vacuumer, &pusher/*, &thief*/, &parent,
		&megaTanker, &laserTank, &hyperSpeedster, &gigaExploder, &grandParent, &gigaTank, &larvaTank
	};
#pragma endregion
#pragma region Player
	Enemy grub = Enemy(&enemyData, PLAYER_A, nullptr, 0.5f, 3, 1, 0.5f, 0, -1, -1, 10, 1.5f, RGBA(161, 85, 35), RGBA(), 3.5f, 0, 60, 60, "Grub");
#pragma endregion
#pragma region Bosses
	// Bosses - Special
	Projectile catProjectile = Projectile(&projectileData, 25.0f, 10, cat.speed, 0.4f, ::VUpdates::EntityVU, cat.color, 1, 0, 1, 1, "Cataclysmic Bullet", false, true);
	Projectile catProjectile2 = Projectile(&projectileData, 25.0f, 10, cat.speed * 2, 0.4f, ::VUpdates::EntityVU, cat.color, 1, 0, 1, 1, "Cataclysmic Bullet", false, true);
	Cataclysm cataclysm = Cataclysm(&cataclysmData, 10.0f, 25.0f, PI_F / 5, &catProjectile, &catProjectile2, 0.0625f, 6.5f, 0.5f, 4.0f, 12.0f, 0.5f, WILD_A, nullptr, 0.5f, 5.0f, 1000, -1, -1, 10, 3.5f, RGBA(), RGBA(), RGBA(158, 104, 95), RGBA(127), 50, 0, 9, 9, "Cataclysm - The nine lived feind");

	vector<Enemy*> spawnableBosses
	{
		&cataclysm
	};
#pragma endregion
#pragma endregion
}

namespace Resources
{
	PlacedOnLanding grubium = PlacedOnLanding(ITEMTYPE::GRUBIUM, &Enemies::grub, "Grubium", "Summon Ammo", VUpdates::GravityVU, 1, RGBA(161, 85, 35), 0, 15.0f, true, 0.5f, 12, 0.4f, false, true, true);
}