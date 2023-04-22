#include "Enemy.h"


namespace Livestock
{
	class Egg : public DToCol
	{
	public:
		Entity* entityToSpawn;
		float timeTill;

		Egg(Entity* entityToSpawn, float timeTill, float radius, RGBA color, RGBA color2, float mass, int maxHealth, int health, string name) :
			DToCol(vZero, radius, color, color2, mass, maxHealth, health, name), entityToSpawn(entityToSpawn), timeTill(timeTill)
		{
			update = UPDATE::EGG;
		}

		Egg(Egg* baseClass, Vec3 pos) :
			Egg(*baseClass)
		{
			this->pos = pos;
			this->baseClass = baseClass;
			Start();
		}

		unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) override
		{
			return make_unique<Egg>(this, pos);
		}
	};

	enum class AI_MODE : byte
	{
		PASSIVE, HUNTING, RUNNING, SLEEPY
	};

	class Livestock : public DToCol
	{
	public:
		Entity* birthEntity = nullptr;

		using DToCol::DToCol;
	};

	class Kiwi : public Livestock
	{
	public:
		AI_MODE ai = AI_MODE::PASSIVE;
		Entity* observing = nullptr;
		int points; float lifetime, startTime = 0, // You get points whenever they reach max lifetime.
			moveSpeed, maxSpeed, turnSpeed, sightDist, sightAngle, // Locomotion
			fullness, awakeFullness, maxFullness, foodPerSecond, foodPerSleepSecond, // Food
			eggLayFullness, timeTillEggLay, timePerEggLay; // Egg laying stats.
		Vec3 lastEatPos = vZero;
		FastNoiseLite noise;

		Kiwi(int points, float lifetime, float moveSpeed, float maxSpeed, float turnSpeed, float sightDist, float sightAngle,
			float startFullness, float awakeFullness, float maxFullness, float foodPerSecond, float foodPerSleepSecond,
			float eggLayFullness, float timePerEggLay,
			float radius, RGBA color, RGBA color2, float mass, int maxHealth, int health, string name) :
			Livestock(vZero, radius, color, color2, mass, maxHealth, health, name),
			points(points), lifetime(lifetime),
			moveSpeed(moveSpeed), maxSpeed(maxSpeed), turnSpeed(turnSpeed), sightDist(sightDist), sightAngle(sightAngle),
			fullness(startFullness), awakeFullness(awakeFullness), maxFullness(maxFullness),
			foodPerSecond(foodPerSecond), foodPerSleepSecond(foodPerSleepSecond),
			eggLayFullness(eggLayFullness), timeTillEggLay(timePerEggLay), timePerEggLay(timePerEggLay)
		{
			update = UPDATE::KIWI;
			dUpdate = DUPDATE::KIWI;
			onDeath = ONDEATH::KIWI;
			noise = FastNoiseLite();
			noise.SetFrequency(0.25f);
		}

		Kiwi(Kiwi* baseClass, Vec3 pos) :
			Kiwi(*baseClass)
		{
			this->pos = pos;
			lastEatPos = pos;
			this->dir = Vec3(RandCircPoint2(), 0.f);
			this->baseClass = baseClass;
			startTime = tTime;
			noise.SetSeed(rand());
			Start();
		}

		unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) override
		{
			return make_unique<Kiwi>(this, pos);
		}

		void UnAttach(Entity* entity)
		{
			if (entity == observing)
			{
				ai = AI_MODE::PASSIVE;
				observing = nullptr;
			}
		}

		void LayEgg()
		{
			game->entities->push_back(birthEntity->Clone(pos, up, this));
			fullness -= eggLayFullness;
		}
	};

	namespace Updates
	{
		void EggU(Entity* entity)
		{
			Egg* egg = static_cast<Egg*>(entity);

			egg->timeTill -= game->dTime;
			if (egg->timeTill <= 0)
			{
				game->entities->push_back(egg->entityToSpawn->Clone(egg->pos, up, egg));
				egg->DestroySelf(egg);
			}
		}

		void KiwiU(Entity* entity)
		{
			Kiwi* kiwi = static_cast<Kiwi*>(entity);
			if (tTime - kiwi->startTime > kiwi->lifetime)
			{
				totalGamePoints += kiwi->points;
				for (int i = 0; i < 4; i++)
				{
					unique_ptr<Entity> newSeed = Collectibles::Seeds::plantSeeds[rand() % Collectibles::Seeds::plantSeeds.size()]->Clone(kiwi->pos, up, kiwi);
					newSeed->vel = kiwi->dir * 5.f;
					game->entities->push_back(std::move(newSeed));
					kiwi->dir = RotateRight(kiwi->dir);
				}
				return kiwi->DestroySelf(kiwi);
			}
			if (kiwi->fullness >= kiwi->eggLayFullness)
				kiwi->timeTillEggLay -= game->dTime;
			if (kiwi->timeTillEggLay <= 0)
			{
				kiwi->LayEgg();
				kiwi->timeTillEggLay += kiwi->timePerEggLay;
			}
			if (kiwi->fullness > kiwi->maxFullness)
			{
				int random = rand();
				if (kiwi->ai != AI_MODE::SLEEPY)
				{
					unique_ptr<Entity> newSeed = Collectibles::Seeds::plantSeeds[rand() % Collectibles::Seeds::plantSeeds.size()]->Clone(kiwi->pos, up, kiwi);
					newSeed->vel = kiwi->dir * -5.f;
					game->entities->push_back(std::move(newSeed));
				}
				kiwi->ai = AI_MODE::SLEEPY;
			}
			if (kiwi->ai == AI_MODE::SLEEPY)
			{
				kiwi->fullness -= game->dTime * kiwi->foodPerSleepSecond;
				if (kiwi->fullness < kiwi->awakeFullness)
					kiwi->ai = AI_MODE::PASSIVE;
			}
			else
			{
				kiwi->fullness -= game->dTime * kiwi->foodPerSecond;
				if (kiwi->fullness < 0 && int(tTime) != int(tTime - game->dTime) && kiwi->DealDamage(1, kiwi) == 1)
					return;
			}
			switch (kiwi->ai)
			{
			case AI_MODE::PASSIVE:
			{
				float randomness = kiwi->noise.GetNoise(tTime, 0.f);
				if (glm::distance2(kiwi->pos, kiwi->lastEatPos) > 15 * 15)
					kiwi->dir = RotateTowardsNorm(kiwi->dir, kiwi->lastEatPos - kiwi->pos, game->dTime * kiwi->turnSpeed);
				else
					kiwi->dir = Normalized(RotateBy(kiwi->dir, kiwi->turnSpeed * game->dTime * randomness));
				kiwi->vel = TryAdd2(kiwi->vel, kiwi->dir * (game->planet->friction + kiwi->moveSpeed * game->dTime), kiwi->maxSpeed);
				vector<Entity*> seenEntities = EntitiesOverlaps(kiwi->pos, kiwi->sightDist, game->entities->collectibles);
				for (int i = 0; i < seenEntities.size(); i++)
					if (seenEntities[i]->isCollectible && kiwi->sightAngle <= glm::dot(kiwi->dir, Normalized(seenEntities[i]->pos - kiwi->pos)))
					{
						kiwi->observing = seenEntities[i];
						kiwi->observing->observers.push_back(kiwi);
						kiwi->ai = AI_MODE::HUNTING;
						break;
					}
				break;
			}
			case AI_MODE::HUNTING:
			{
				kiwi->dir = RotateTowardsNorm(kiwi->vel != vZero && glm::normalize(kiwi->vel) != kiwi->dir ? kiwi->vel : kiwi->dir,
					kiwi->observing->pos - kiwi->pos, game->dTime * kiwi->turnSpeed);
				if (glm::dot(kiwi->dir, Normalized(kiwi->observing->pos - kiwi->pos)) > 0.75f)
					kiwi->vel = TryAdd2(kiwi->vel, kiwi->dir * (game->planet->friction + kiwi->moveSpeed * game->dTime), kiwi->maxSpeed);
				vector<Entity*> collectibles = EntitiesOverlaps(kiwi->pos, kiwi->radius, game->entities->collectibles);
				for (Entity* collectible : collectibles)
				{
					kiwi->lastEatPos = kiwi->pos;
					kiwi->fullness += ((Collectible*)collectible)->baseItem.mass;
					collectible->DestroySelf(kiwi);
				}
				break;
			}
			}
		}
	}

	namespace DUpdates
	{
		void KiwiDU(Entity* entity)
		{
			Kiwi* kiwi = static_cast<Kiwi*>(entity);

			float ratio = kiwi->radius / SQRTTWO_F;
			game->DrawRightTri(kiwi->pos + kiwi->dir * ratio, vOne * ratio,
				atan2f(kiwi->dir.y, kiwi->dir.x) - PI_F * 0.5f, kiwi->color);

			kiwi->DUpdate(DUPDATE::DTOCOL);
		}
	}

	namespace OnDeaths
	{
		void KiwiOD(Entity* entity, Entity* damageDealer)
		{
			Kiwi* kiwi = static_cast<Kiwi*>(entity);

			if (kiwi->observing != nullptr)
				kiwi->observing->observers.erase(find(kiwi->observing->observers.begin(), kiwi->observing->observers.end(), entity));
		}
	}

	unique_ptr<Kiwi> kiwi = make_unique<Kiwi>(50, 180.f, 1.f, 5.f, PI_F, 15.f, 0.f, 5.f, 10.f, 15.f, 0.1f, 0.2f, 10.f, 60.f, 0.5f, RGBA(168, 109, 61), RGBA(45, 89, 26), 0.375f, 5, 5, "Kiwi");
	unique_ptr<Egg> kiwiEgg = make_unique<Egg>(kiwi.get(), 30.f, 0.4f, RGBA(217, 200, 158), RGBA(), 0.25f, 5, 5, "Kiwi Egg");

	vector<Livestock*> livestocks{ kiwi.get() };
	vector<Entity*> livestockBirths{ kiwiEgg.get()};
}

namespace Resources::Eggs
{
	unique_ptr<PlacedOnLanding> kiwiEgg = make_unique<PlacedOnLanding>(Livestock::kiwiEgg.get(), "Kiwi Egg", "Egg", 0, Livestock::kiwiEgg->color, 0, 1, 15.f, false, 0.25f, Livestock::kiwiEgg->radius);
}