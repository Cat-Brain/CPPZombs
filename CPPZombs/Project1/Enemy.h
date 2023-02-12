#include "Defence.h"

namespace Enemies
{
#pragma region Enemy types
	// The base class of all enemies.
	class Enemy : public DToCol
	{
	public:
		float timePer, lastTime, timePerMove, lastMove;

		int points, firstWave;
		int damage;

		Enemy(float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1, Vec2 dimensions = vOne,
			RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA subScat = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			DToCol(vZero, dimensions, color, color2, subScat, mass, maxHealth, health, name),
			timePer(timePer), lastTime(0.0f), timePerMove(timePerMove), lastMove(0.0f), points(points), firstWave(firstWave), damage(damage)
		{
		}

		Enemy(Enemy* baseClass, Vec2f pos) :
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

		bool IsEnemy() override
		{
			return true;
		}

		void Update() override
		{
			if (tTime - lastMove >= timePerMove)
				if (MUpdate())
					lastMove = tTime;

			if (tTime - lastTime >= timePer)
				if (TUpdate())
					lastTime = tTime;
		}

		virtual bool MUpdate()
		{
			TryMove(Vec2f(game->PlayerPos() - pos).Rormalized(), mass + mass);
			return true;
		}

		virtual bool TUpdate()
		{
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(pos, dimensions + vOne);
			int randomization = rand();
			for (int i = 0; i < hitEntities.size(); i++)
			{
				Entity* entity = hitEntities[(i + randomization) % hitEntities.size()];
				if (entity != this && !entity->IsEnemy())
				{
					entity->DealDamage(damage, this);
					break;
				}
			}

			return true;
		}

		Vec2 TopRight() override
		{
			return BottomLeft() + Vec2(font.TextWidth(name + " " + to_string(health)) * COMMON_TEXT_SCALE / font.minimumSize, font.maxVertOffset / 2) / 2;
		}

		void UIUpdate() override
		{
			DrawUIBox(BottomLeft(), TopRight(), COMMON_BOARDER_WIDTH, name + " " + to_string(health), color);
		}

		bool PosInUIBounds(Vec2 screenSpacePos) override
		{
			Vec2 topLeft = BottomLeft();
			Vec2 bottomRight = TopRight();
			return screenSpacePos.x >= topLeft.x && screenSpacePos.x <= bottomRight.x &&
				screenSpacePos.y >= topLeft.y && screenSpacePos.y <= bottomRight.y;
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


	class Deceiver : public Enemy
	{
	public:
		RGBA color3;
		FastNoiseLite noise1, noise2, noise3; // <-For random colors.

		Deceiver(float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1, Vec2 dimensions = vOne,
			RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(), RGBA subScat = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, timePerMove, points, firstWave, damage, dimensions, color, color2, subScat, mass, maxHealth, health, name), color3(color3), noise1(), noise2(), noise3()
		{
			Start();
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

		void DUpdate() override
		{
			float r = noise1.GetNoise(tTime, 0.0f);
			float g = noise2.GetNoise(tTime, 0.0f);
			float b = noise3.GetNoise(tTime, 0.0f);
			color.r = r * 255;
			color.g = g * 255;
			color.b = b * 255;

			Enemy::DUpdate();

			float healthRatio = (float)health / maxHealth;
			color.r = r * color3.r;
			color.g = g * color3.g;
			color.b = b * color3.b;

			Vec2f tempPos = pos;

			pos = Vec2f(game->PlayerPos().x * 2 - pos.x, pos.y);
			Enemy::DUpdate();
			pos = Vec2f(pos.x, game->PlayerPos().y * 2 - pos.y);
			Enemy::DUpdate();
			pos = Vec2f(game->PlayerPos().x * 2 - pos.x, pos.y);
			Enemy::DUpdate();

			pos = tempPos;
		}

	};

	class Parent : public Enemy
	{
	public:
		Enemy* child;

		Parent(Enemy* child, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA subScat = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, timePerMove, points, firstWave, damage, dimensions, color, color2, subScat, mass, maxHealth, health, name), child(child)
		{
			Start();
		}

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Parent> newEnemy = make_unique<Parent>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
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

		Exploder(Vec2 explosionDimensions, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA subScat = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, timePerMove, points, firstWave, damage, dimensions, color, color2, subScat, mass, maxHealth, health, name), explosionDimensions(explosionDimensions)
		{ }

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Exploder> newEnemy = make_unique<Exploder>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}

		void DUpdate() override
		{
			Vec2 tempDimensions = dimensions;
			dimensions = explosionDimensions;
			byte tempAlpha = color.a;
			color.a /= 5;
			Enemy::DUpdate();
			dimensions = tempDimensions;
			color.a = tempAlpha;
			Enemy::DUpdate();
		}

		bool TUpdate() override
		{
			vector<Entity*> hitEntities = game->entities->FindCorpOverlaps(pos, explosionDimensions);
			int randomization = rand();
			for (int i = 0; i < hitEntities.size(); i++)
			{
				Entity* entity = hitEntities[(i + randomization) % hitEntities.size()];
				if (entity != this && !entity->IsEnemy())
				{
					CreateExplosion(pos, explosionDimensions, color, name, 0, damage, this);
					return true;
				}
			}
			return false;
		}

		void OnDeath(Entity* damageDealer) override
		{
			Enemy::OnDeath(damageDealer);
			CreateExplosion(pos, explosionDimensions, color, name, 0, damage, this);
		}
	};

	class Snake : public Enemy
	{
	public:
		RGBA color3, color4;
		Snake* back = nullptr, * front = nullptr;
		int length;

		Snake(int length, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA color2 = RGBA(),
			RGBA subScat = RGBA(), RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, timePerMove, points, firstWave, damage, dimensions, color, color2, subScat, mass, maxHealth, health, name), length(length), color3(color3), color4(color4)
		{ }

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Snake>* enemies = new unique_ptr<Snake>[length];
			for (int i = 0; i < length; i++)
			{
				enemies[i] = make_unique<Snake>(*this);
				enemies[i]->baseClass = baseClass;
				enemies[i]->pos = pos;
				enemies[i]->points = points / length;
				enemies[i]->Start();
				enemies[i]->color = color.Lerp(color4, sinf(static_cast<float>(i)) * 0.5f + 0.5f);
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
				game->entities->push_back(std::move(enemies[i]));
			return std::move(enemies[0]); // Do 0 here.
		}

		void DUpdate() override
		{
			if (front == nullptr)
				color = color3;
			Enemy::DUpdate();
		}

		bool TryMove(Vec2 direction, int force, Entity* ignore = nullptr, Entity** hitEntity = nullptr) override
		{
			return false;
		}

		void SetPos(Vec2 newPos) override
		{
			Vec2 lastPos = pos;
			Enemy::SetPos(newPos);
			if (lastPos != pos)
				if (back != nullptr)
					back->SetPos(lastPos);
		}

		bool MUpdate() override
		{
			if (front == nullptr)
				Enemy::TryMove(Vec2f(game->PlayerPos() - pos).Rormalized() * dimensions, mass + mass);

			return true;
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
			return points / 5;
		}
	};

	class PouncerSnake : public Snake
	{
	public:
		float pounceTime, speed;
		Vec2f offset, direction;

		PouncerSnake(float pounceTime, float speed, int length, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA color2 = RGBA(),
			RGBA subScat = RGBA(), RGBA color3 = RGBA(), RGBA color4 = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Snake(length, timePer, timePerMove, points, firstWave, damage, dimensions, color, color2, subScat, color3, color4, mass, maxHealth, health, name),
			pounceTime(pounceTime), speed(speed)
		{ }

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<PouncerSnake>* enemies = new unique_ptr<PouncerSnake>[length];
			for (int i = 0; i < length; i++)
			{
				enemies[i] = make_unique<PouncerSnake>(*this);
				enemies[i]->baseClass = baseClass;
				enemies[i]->pos = pos;
				enemies[i]->points = points / length;
				enemies[i]->Start();
				enemies[i]->color = color.Lerp(color4, sinf(static_cast<float>(i)) * 0.5f + 0.5f);
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
				game->entities->push_back(std::move(enemies[i]));
			return std::move(enemies[0]); // Do 0 here.
		}

		bool MUpdate() override
		{
			offset = vZero;
			direction = Vec2f(game->PlayerPos() - pos).Normalized();
			return true;
		}

		void Update() override
		{
			Enemy::Update();

			if (front == nullptr && tTime - lastMove <= pounceTime)
			{
				offset += direction * game->dTime * speed;
				if (Vec2(offset) != vZero)
				{
					Enemy::TryMove(offset, mass + mass);
					offset -= Vec2(offset);
				}
			}
		}

		void OnDeath(Entity* damageDealer) override
		{
			Snake::OnDeath(damageDealer);
			if (back != nullptr)
			{
				((PouncerSnake*)back)->offset = offset;
				((PouncerSnake*)back)->direction = direction;
			}
		}
	};

	class ColorCycler : public Enemy
	{
	public:
		vector<RGBA> colorsToCycle;
		float colorOffset, colorCycleSpeed;

		ColorCycler(vector<RGBA> colorsToCycle, float colorCycleSpeed = 1.0f, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1,
			int firstWave = 1, int damage = 1, Vec2 dimensions = vOne, RGBA color2 = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, timePerMove, points, firstWave, damage, dimensions, colorsToCycle[0], color2, RGBA(), mass, maxHealth, health, name),
			colorsToCycle(colorsToCycle), colorCycleSpeed(colorCycleSpeed), colorOffset(0.0f)
		{ }

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

		void DUpdate() override
		{
			float currentPlace = (tTime * colorCycleSpeed + colorOffset);
			int intCurrentPlace = static_cast<int>(currentPlace);
			color = colorsToCycle[intCurrentPlace % colorsToCycle.size()].Lerp(
				colorsToCycle[(intCurrentPlace + 1) % colorsToCycle.size()], currentPlace - floorf(currentPlace));

			Enemy::DUpdate();
		}
	};

	class Vacuumer : public Enemy
	{
	public:
		int vacDist, desiredDistance;
		Items items;

		Vacuumer(int vacDist, int desiredDistance, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2f dimensions = vOne, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA subScat = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, timePerMove, points, firstWave, damage, dimensions, color, color2, subScat, mass, maxHealth, health, name),
			vacDist(vacDist), desiredDistance(desiredDistance), items(0)
		{ }

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Vacuumer> newEnemy = make_unique<Vacuumer>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return newEnemy;
		}

		bool MUpdate() override
		{
			if (abs(pos.Squistance(game->PlayerPos()) - desiredDistance) > 2.0f)
				TryMove(Vec2f(game->PlayerPos() - pos).Rormalized() * (-1 + 2 * int(pos.Squistance(game->PlayerPos()) > desiredDistance)), mass + mass);
			return true;
		}

		bool TUpdate() override
		{
			game->entities->Vacuum(pos, vacDist);
			vector<Entity*> collectibles = EntitiesOverlaps(pos, dimensions, game->entities->collectibles);
			for (Entity* collectible : collectibles)
			{
				items.push_back(((Collectible*)collectible)->baseItem);
				collectible->DestroySelf(this);
			}
			return Enemy::TUpdate();
		}

		void OnDeath(Entity* damageDealer) override
		{
			Enemy::OnDeath(damageDealer);
			for (Item item : items)
			{
				game->entities->push_back(make_unique<Collectible>(item, pos));
			}
		}
	};

	class Ranger : public Vacuumer
	{
	public:
		using Vacuumer::Vacuumer;

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Ranger> newEnemy = make_unique<Ranger>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
		}

		bool TUpdate() override
		{
			Vacuumer::TUpdate();

			if (items.size() > 0)
			{
				Item shotItem = items[0].Clone(1);
				items.TryTakeIndex(0);
				game->entities->push_back(basicShotItem->Clone(shotItem, pos, Vec2f((game->PlayerPos() - pos) * static_cast<int>(shotItem.range)), this));
			}

			return true;
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
			Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA subScat = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, moveSpeed, points, firstWave, damage, dimensions, color, color2, subScat, mass, maxHealth, health, name),
			baseLeg(baseLeg), legCount(legCount), legLength(legLength), legTolerance(legTolerance), legCycleSpeed(legCycleSpeed) { }

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
					Vec2f desiredPos = LegPos(i) + Vec2f(game->PlayerPos() - pos).Normalized() * legTolerance * 0.5f;
					if (legs[i]->desiredPos.Distance(desiredPos) > legTolerance)
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
			float timePer = 0.5f, float moveSpeed = 2.0f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2f dimensions = vOne, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA subScat = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			baseChild(baseChild), Spider(baseLeg, legCount, legLength, legTolerance, legCycleSpeed,
				timePer, moveSpeed, points, firstWave, damage, dimensions, color, color2, subScat, mass, maxHealth, health, name) { }

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
				game->entities->push_back(baseChild->Clone(pos - dimensions + vOne + Vec2f(RandFloat() * ((dimensions.x + 1) * 2), RandFloat() * ((dimensions.y + 1) * 2)), up, this));
			return Spider::DealDamage(damage, damageDealer);
		}
	};

	class Pouncer : public Enemy
	{
	public:
		float pounceTime, speed;
		Vec2f offset, direction;

		Pouncer(float pounceTime, float speed, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA subScat = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Enemy(timePer, timePerMove, points, firstWave, damage, dimensions, color, color2, subScat, mass, maxHealth, health, name),
			pounceTime(pounceTime), speed(speed)
		{ }

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Pouncer> newEnemy = make_unique<Pouncer>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
		}

		bool MUpdate() override
		{
			offset = vZero;
			direction = Vec2f(game->PlayerPos() - pos).Normalized();
			return true;
		}

		void Update() override
		{
			Enemy::Update();

			if (tTime - lastMove <= pounceTime)
			{
				offset += direction * game->dTime * speed;
				if (Vec2(offset) != vZero)
				{
					TryMove(offset, mass + mass);
					offset -= Vec2(offset);
				}
			}
		}
	};

	class Cat : public Pouncer
	{
	public:
		RGBA color3;

		Cat(float pounceTime, float speed, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(), RGBA subScat = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Pouncer(pounceTime, speed, timePer, timePerMove, points, firstWave, damage, dimensions, color, color2, subScat, mass, maxHealth, health, name), color3(color3)
		{ }

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<Cat> newEnemy = make_unique<Cat>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
		}

		int DealDamage(int damage, Entity* damageDealer) override
		{
			if (damage > 0)
			{
				int count = maxHealth - health + 2;
				int index = rand() % count;
				offset = vZero;
				direction = direction.Rotate(PI_F * 2 * index / count);
				SetPos(game->PlayerPos() + Vec2f(pos - game->PlayerPos()).Rotate(PI_F * 2 * index / count));
			}
			return Enemy::DealDamage(Clamp(damage, -1, 1), damageDealer);
		}

		void DUpdate() override
		{
			int count = maxHealth - health + 1;

			Vec2 tempPos = pos;
			Vec2f fPos = pos;
			RGBA tempColor = color;
			color = color3;

			for (int i = 1; i < count; i++)
			{
				fPos = (fPos - game->PlayerPos()).Rotate(PI_F * 2 / count) + game->PlayerPos();
				pos = fPos;
				Pouncer::DUpdate();
			}

			color = tempColor;
			pos = tempPos;
			Pouncer::DUpdate();
		}
	};

	class BoomCat : public Cat
	{
	public:
		Vec2 explosionDimensions;

		BoomCat(Vec2 explosionDimensions, float pounceTime, float speed, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(), RGBA subScat = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			Cat(pounceTime, speed, timePer, timePerMove, points, firstWave, damage, dimensions, color, color2, color3, subScat, mass, maxHealth, health, name), explosionDimensions(explosionDimensions)
		{ }

		unique_ptr<Entity> Clone(Vec2 pos, Vec2 dir = up, Entity* creator = nullptr) override
		{
			unique_ptr<BoomCat> newEnemy = make_unique<BoomCat>(*this);
			newEnemy->baseClass = baseClass;
			newEnemy->pos = pos;
			newEnemy->Start();
			return std::move(newEnemy);
		}

		bool TUpdate() override
		{
			CreateExplosion(pos, explosionDimensions, color, name, 0, damage, this);
			return true;
		}
	};

	class Cataclysm : public BoomCat
	{
	public:
		Cat* normalChild, * boomChild;

		Cataclysm(Cat* normalChild, Cat* boomChild, Vec2 explosionDimensions, float pounceTime, float speed, float timePer = 0.5f, float timePerMove = 0.5f, int points = 1, int firstWave = 1, int damage = 1,
			Vec2 dimensions = vOne, RGBA color = RGBA(), RGBA color2 = RGBA(), RGBA color3 = RGBA(), RGBA subScat = RGBA(),
			int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
			BoomCat(explosionDimensions, pounceTime, speed, timePer, timePerMove, points, firstWave, damage, dimensions, color, color2, color3, subScat, mass, maxHealth, health, name), normalChild(normalChild), boomChild(boomChild)
		{ }

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
			if (health == 1 && damage > 0)
				game->entities->push_back(boomChild->Clone(pos, up, this));
			else if (damage > 0)
				game->entities->push_back(normalChild->Clone(pos, up, this));
			return Cat::DealDamage(Clamp(damage, -1, 1), damageDealer);
		}
	};
#pragma endregion


#pragma region Enemies
	//Predefinitions - Special
	LegParticle* spiderLeg = new LegParticle(vZero, nullptr, RGBA(0, 0, 0, 150), 32.0f);
	Enemy* child = new Enemy(1.0f, 0.125f, 0, 0, 1, vOne, RGBA(255, 0, 255), RGBA(), RGBA(0, 50), 1, 1, 1, "Child");

	// Earlies - 1
	Enemy* walker = new Enemy(0.75f, 0.5f, 1, 1, 1, vOne, RGBA(0, 255, 255), RGBA(), RGBA(50), 1, 3, 3, "Walker");
	Enemy* tanker = new Enemy(1.0f, 0.75f, 2, 1, 1, vOne * 3, RGBA(255), RGBA(), RGBA(0, 25, 25), 5, 12, 12, "Tanker");
	Spider* spider = new Spider(*spiderLeg, 6, 3.0f, 0.25f, 1.0f, 0.5f, 0.25f, 2, 1, 1, vOne, RGBA(79, 0, 26), RGBA(), RGBA(55, 55, 55), 1, 2, 2, "Spider");

	// Mids - 4
	Deceiver* deceiver = new Deceiver(0.5f, 0.25f, 4, 4, 1, vOne, RGBA(255, 255, 255), RGBA(), RGBA(255, 255, 255, 153), RGBA(), 1, 3, 3, "Deceiver");
	Exploder* exploder = new Exploder(vOne * 5, 1.0f, 0.375f, 4, 4, 1, vOne, RGBA(153, 255, 0), RGBA(), RGBA(25, 0, 25), 1, 3, 3, "Exploder");
	Vacuumer* vacuumer = new Vacuumer(12, 12, 0.125f, 0.125f, 3, 4, 0, vOne, RGBA(255, 255, 255), RGBA(), RGBA(50, 50, 50), 1, 3, 3, "Vacuumer");
	Pouncer* frog = new Pouncer(2.0f, 16.0f, 1.0f, 4.0f, 4, 4, 1, vOne, RGBA(107, 212, 91), RGBA(), RGBA(25, 0, 25), 3, 3, 3, "Frog");

	// Mid-lates - 6
	Parent* parent = new Parent(child, 1.0f, 1.0f, 4, 6, 1, vOne * 5, RGBA(127, 0, 127), RGBA(), RGBA(0, 50, 0), 1, 10, 10, "Parent");
	Parent* spiderParent = new Parent(spider, 1.0f, 1.0f, 4, 6, 1, vOne * 5, RGBA(140, 35, 70), RGBA(), RGBA(0, 50, 0), 5, 10, 10, "Spider Parent");
	Snake* snake = new Snake(30, 0.5f, 0.25f, 30, 6, 1, vOne, RGBA(0, 255), RGBA(), RGBA(50, 0, 0), RGBA(255, 255), RGBA(0, 127), 2, 3, 3, "Snake");
	
	// Lates - 8
	ColorCycler* hyperSpeedster = new ColorCycler({ RGBA(255), RGBA(255, 255), RGBA(0, 0, 255) }, 2.0f, 0.5f, 0.25f, 8, 8, 1, vOne, RGBA(), 1, 24, 24, "Hyper Speedster");
	Enemy* megaTanker = new Enemy(1.0f, 1.0f, 20, 8, 1, vOne * 5, RGBA(174, 0, 255), RGBA(), RGBA(0, 25, 25), 10, 48, 48, "Mega Tanker");
	Exploder* gigaExploder = new Exploder(vOne * 15, 1.0f, 0.25f, 8, 8, 1, vOne * 3, RGBA(153, 255), RGBA(), RGBA(25, 0, 25), 1, 3, 3, "Giga Exploder");
	Ranger* ranger = new Ranger(12, 12, 0.125f, 0.125f, 6, 8, 0, vOne * 5, RGBA(127, 127, 127), RGBA(), RGBA(50, 50, 50), 5, 12, 12, "Ranger");
	Snake* bigSnake = new Snake(30, 0.5f, 0.5f, 60, 8, 1, vOne * 3, RGBA(0, 255), RGBA(), RGBA(50, 0, 0), RGBA(255, 255), RGBA(0, 127), 2, 9, 9, "Big Snake");
	PouncerSnake* pouncerSnake = new PouncerSnake(3.0f, 24.0f, 30, 0.5f, 8.0f, 60, 8, 1, vOne, RGBA(0, 0, 255), RGBA(), RGBA(50, 0, 0), RGBA(0, 255, 255), RGBA(0, 0, 127), 2, 3, 3, "Pouncer Snake");

	// Very lates - 12
	Cat* cat = new Cat(2.0f, 16.0f, 0.25f, 3.0f, 45, 12, 1, vOne, RGBA(209, 96, 36), RGBA(), RGBA(186, 118, 82), RGBA(), 1, 9, 9, "Cat");
	BoomCat* boomCat = new BoomCat(vOne * 9, 2.0f, 12.0f, 1.0f, 4.0f, 45, 12, 1, vOne * 3, RGBA(255, 120, 97), RGBA(), RGBA(158, 104, 95), RGBA(), 9, 9, 9, "Boom Cat");
	Spoobderb* spoobderb = new Spoobderb(spider, *spiderLeg, 30, 25.0f, 3.0f, 2.5f, 0.5f, 0.5f, 50, 12, 1, vOne * 7, RGBA(77, 14, 35), RGBA(), RGBA(55, 55, 55), 50, 100, 100, "Spoobderb - The 30 footed beast");

	// Bosses - Special
	Cataclysm* cataclysm = new Cataclysm(cat, boomCat, vOne * 13, 5.0f, 12.0f, 2.0f, 5.0f, 250, 12, 1, vOne * 7, RGBA(), RGBA(), RGBA(158, 104, 95), RGBA(), 50, 9, 9, "Cataclysm - The nine lived feind");
#pragma endregion

	class Instance;
	class Types : public vector<vector<Enemy*>>
	{
	public:
		using vector<vector<Enemy*>>::vector;

		static int GetRoundPoints()
		{
			return static_cast<int>(pow(1.37, waveCount)) + waveCount * 3 - 1;
		}

		Instance RandomClone();
	};

	class Instance : public vector<Enemy*>
	{
	public:
		using vector<Enemy*>::vector;

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
				float randomValue = RandFloat() * 6.283184f;
				game->entities->push_back(currentlySpawnableEnemies[currentIndex]->Clone(Vec2f(cosf(randomValue), sinf(randomValue)) * ScrDim() * 0.5f * 1.415f + game->PlayerPos()));
				totalCost += currentlySpawnableEnemies[currentIndex]->Cost();
			}
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
				float randomValue = RandFloat() * 6.283184f;
				game->entities->push_back(currentlySpawnableEnemies[currentIndex]->Clone(Vec2f(cosf(randomValue), sinf(randomValue)) * ScrDim() * 0.5f * 1.415f + game->PlayerPos()));
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
		{walker, tanker, spider},
		{deceiver, exploder, vacuumer, frog},
		{parent, spiderParent, snake},
		{hyperSpeedster, megaTanker, gigaExploder, ranger, bigSnake, pouncerSnake},
		{cat, boomCat, spoobderb}
	};

	Types spawnableBosses
	{
		{cataclysm}
	};
}