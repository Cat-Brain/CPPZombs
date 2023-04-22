#include "Livestock.h"

class UseableItem : public Item
{
public:
	UseableItem(ITEMU useFunction, string name = "NULL NAME", string typeName = "NULL TYPE NAME", int intType = 0, RGBA color = RGBA(), float useTime = 0) :
		Item(name, typeName, intType, color, 0, 1, 0, useTime)
	{
		itemU = useFunction;
	}

	UseableItem(Item* baseClass, ITEMU useFunction, string name = "NULL NAME", string typeName = "NULL TYPE NAME", int intType = 0, RGBA color = RGBA(), float useTime = 0) :
		Item(baseClass, name, typeName, intType, color, 0, 1, 0, useTime)
	{
		itemU = useFunction;
	}

	Item Clone(int count) override
	{
		return UseableItem(baseClass, itemU, name, typeName, intType, color, useTime);
	}

	Item* Clone2(int count) override
	{
		return new UseableItem(baseClass, itemU, name, typeName, intType, color, useTime);
	}
};

namespace Resources
{
	unique_ptr<UseableItem> waveModifier = make_unique<UseableItem>(ITEMU::WAVEMODIFIER, "Wave Modifier", "Special Item", 0, RGBA(194, 99, 21), 1.f);
}

#define NUM_START_ITEMS 3

int difficultySeedSpawnQuantity[] = { 7, 5, 3 };


enum class PMOVEMENT
{
	DEFAULT
};
vector<std::function<void(Player* player)>> pMovements;

enum class PRIMARY
{
	SLINGSHOT, CIRCLEGUN, CHOMP
};
vector< std::function<bool(Player* player)>> primaries;

enum class SECONDARY
{
	GRENADE_THROW, TORNADO_SPIN, VINE_SHOT
};
vector< std::function<bool(Player* player)>> secondaries;

enum class UTILITY
{
	TACTICOOL_ROLL, MIGHTY_SHOVE, 
};
vector< std::function<bool(Player* player)>> utilities;



class Player : public LightBlock
{
public:
	Entity* heldEntity = nullptr, *currentMenuedEntity = nullptr;
	Items items, startItems;
	vector<SEEDINDICES> blacklistedSeeds;
	Vec2 placingDir = up;
	float moveSpeed, maxSpeed, vacDist, vacSpeed, maxVacSpeed, holdMoveSpeed, maxHoldMoveSpeed, holdMoveWeight, shootSpeed,
		lastPrimary = 0, primaryTime, lastSecondary = 0, secondaryTime, lastUtility = 0, utilityTime, lastJump = 0;
	bool vacBoth, vacCollectibles, shouldVacuum = true;
	PMOVEMENT movement;
	PRIMARY primary;
	SECONDARY secondary;
	UTILITY utility;
	vector<TimedEvent> events{};
	vector<Particle*> particles{};
	float iTime = 0, sTime = 0; // Invincibility time, if <= 0 takes damage, else doesn't.

	Player(bool vacBoth = false, bool vacCollectibles = true, float radius = 0.5f, float moveSpeed = 8, float maxSpeed = 8,
		float holdMoveSpeed = 32, float maxHoldMoveSpeed = 8, float holdMoveWeight =  4, float vacDist = 6, float vacSpeed = 16,
		float maxVacSpeed = 16, float shootSpeed = 1, float primaryTime = 1, float secondaryTime = 1,
		float utilityTime = 1, PMOVEMENT movement = PMOVEMENT::DEFAULT, PRIMARY primary = PRIMARY::SLINGSHOT,
		SECONDARY secondary = SECONDARY::GRENADE_THROW, UTILITY utility = UTILITY::TACTICOOL_ROLL,
		RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127), bool lightOrDark = true, float range = 10, float mass = 1,
		int maxHealth = 1, int health = 1, string name = "NULL NAME", Items startItems = {}, vector<SEEDINDICES> blacklistedSeeds = {}) :
		LightBlock(lightColor, lightOrDark, range, vZero, radius, color, color2, mass, maxHealth, health, name), vacDist(vacDist),
		moveSpeed(moveSpeed), holdMoveSpeed(holdMoveSpeed), startItems(startItems), blacklistedSeeds(blacklistedSeeds), vacBoth(vacBoth),
		vacCollectibles(vacCollectibles), vacSpeed(vacSpeed), maxSpeed(maxSpeed), maxVacSpeed(maxVacSpeed), shootSpeed(shootSpeed),
		primaryTime(primaryTime), secondaryTime(secondaryTime), utilityTime(utilityTime), movement(movement), primary(primary), secondary(secondary),
		utility(utility), maxHoldMoveSpeed(maxHoldMoveSpeed), holdMoveWeight(holdMoveWeight)
	{
		update = UPDATE::PLAYER;
		dUpdate = DUPDATE::PLAYER;
		onDeath = ONDEATH::PLAYER;
	}

	void Start() override
	{
		LightBlock::Start(); // Set up all of the light stuff.

		lastPrimary = tTime;
		lastSecondary = tTime;
		lastUtility = tTime;
		shouldVacuum = true;

		items = startItems;

		vector<int> spawnedIndices(NUM_START_ITEMS + blacklistedSeeds.size(), -1);
		for (int i = 0; i < blacklistedSeeds.size(); i++)
			spawnedIndices[size_t(i) + NUM_START_ITEMS] = UnEnum(blacklistedSeeds[i]);
		for (int i = 0; i < NUM_START_ITEMS;)
		{
			int newIndex = rand() % Resources::Seeds::plantSeeds.size();
			if (find(spawnedIndices.begin(), spawnedIndices.end(), newIndex) != spawnedIndices.end())
				continue;
			spawnedIndices[i] = newIndex;
			items.push_back(Resources::Seeds::plantSeeds[newIndex]->Clone(difficultySeedSpawnQuantity[game->difficulty]));
			i++;
		}
		items.currentIndex = 0; // Set active ammo type to the first ammo type in inventory.
	}

	Player(Player* baseClass, Vec3 pos) :
		Player(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		iTime = 0;
		sTime = 0;
		Start();
	}

	unique_ptr<Player> PClone(Vec3 pos = vZero, Vec3 dir = up, Entity* creator = nullptr)
	{
		return make_unique<Player>(this, pos);
	}

	int DealDamage(int damage, Entity* damageDealer) override
	{
		if (iTime <= 0)
		{
			game->screenShake += damage;
			iTime += damage;
			return LightBlock::DealDamage(damage, damageDealer);
		}
		return -1;
	}

	void UnAttach(Entity* entity) override
	{
		heldEntity = nullptr;
	}
};

unique_ptr<Player> soldier = make_unique<Player>(false, true, 0.5f, 32.f, 8.f, 32.f, 8.f, 4.f, 6.f, 64.f, 16.f, 1.f, 0.f, 2.f, 4.f, PMOVEMENT::DEFAULT,
	PRIMARY::SLINGSHOT, SECONDARY::GRENADE_THROW, UTILITY::TACTICOOL_ROLL, RGBA(0, 0, 255), RGBA(), JRGB(127, 127, 127), true, 20.f, 5.f, 10, 5,
	"Soldier", Items({ Resources::copper->Clone(10), Resources::Seeds::shadeTreeSeed->Clone(3), Resources::Seeds::cheeseVineSeed->Clone(1),
		Resources::Seeds::quartzVineSeed->Clone(2), Resources::Seeds::copperTreeSeed->Clone(3), Resources::waveModifier->Clone(1), Resources::Eggs::kiwiEgg->Clone(2)}),
	vector<SEEDINDICES>({ SEEDINDICES::COPPER, SEEDINDICES::SHADE, SEEDINDICES::CHEESE, SEEDINDICES::QUARTZ }));
unique_ptr<Player> flicker = make_unique<Player>(false, true, 0.25f, 32.f, 12.f, 32.f, 8.f, 2.f, 4.f, 64.f, 8.f, 1.f, 0.f, 2.f, 5.f, PMOVEMENT::DEFAULT,
	PRIMARY::SLINGSHOT, SECONDARY::TORNADO_SPIN, UTILITY::MIGHTY_SHOVE, RGBA(255, 255), RGBA(0, 0, 255), JRGB(127, 127, 127), true, 5.f, 1.5f,
	3, 2, "Flicker", Items({ Resources::ruby->Clone(100), Resources::Seeds::shadeTreeSeed->Clone(1), Resources::Seeds::cheeseVineSeed->Clone(3),
		Resources::Seeds::quartzVineSeed->Clone(2), Resources::Seeds::rockTreeSeed->Clone(3), Resources::waveModifier->Clone(1), Resources::Eggs::kiwiEgg->Clone(2) }),
	vector<SEEDINDICES>({ SEEDINDICES::ROCK, SEEDINDICES::SHADE, SEEDINDICES::CHEESE, SEEDINDICES::QUARTZ }));

vector<Player*> characters = { soldier.get(), flicker.get() };

namespace OnDeaths
{
	void PlayerOD(Entity* entity, Entity* damageDealer)
	{
		entity->OnDeath(ONDEATH::LIGHTBLOCK, damageDealer);
		playerAlive = false;
		if (damageDealer != nullptr)
			deathCauseName = damageDealer->name;
		else
			deathCauseName = "The planet";
	}
}

namespace ItemUs
{
	void WaveModifierU(Item* stack, Vec3 pos, Vec3 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		game->planet->enemies->superWave ^= true;
		game->entities->push_back(make_unique<FadeOutGlow>(8.f, 3.f, pos, 4.f, stack->color));
		game->screenShake++;
	}
}

namespace PMovements
{
	void Default(Player* player)
	{
		player->vel = TryAdd2(player->vel, game->inputs.MoveDir() * game->dTime * player->moveSpeed, player->maxSpeed);
		if (game->inputs.space.pressed && tTime - player->lastJump > 0.25f &&
			game->entities->OverlapsTile(player->pos + Vec3(0, 0, -player->radius), player->radius - 0.01f))
		{
			player->vel.z += 7;
			player->lastJump = tTime;
		}
	}
}

namespace Primaries
{
	bool Slingshot(Player* player)
	{
		Item currentShootingItem = player->items.GetCurrentItem();
		if (currentShootingItem == *dItem || tTime - player->lastPrimary <= currentShootingItem.useTime * player->shootSpeed)
			return false;
		player->items[player->items.currentIndex].Use(player->pos, game->inputs.mousePosition3, player, player->name, nullptr, 0);
		player->items.RemoveIfEmpty(player->items.currentIndex);
		player->lastPrimary = tTime + player->primaryTime;
		return false;
	}

	bool CircleGun(Player* player)
	{
		std::cout << "Circle gun\t";
		return true;
	}
}

class Grenade : public LightBlock
{
public:
	int damage;
	float explosionRadius, startTime, timeTill, baseRange;

	Grenade(int damage, float explosionRadius, float timeTill, JRGB lightColor, float range, float radius, RGBA color, float mass, int health, string name) :
		LightBlock(lightColor, true, range, vZero, radius, color, RGBA(), mass, health, health, name),
		damage(damage), explosionRadius(explosionRadius), startTime(0), timeTill(timeTill), baseRange(range)
	{
		update = UPDATE::GRENADE;
	}

	Grenade(Grenade* baseClass, Vec3 pos, Vec3 dir, Entity* creator) :
		Grenade(*baseClass)
	{
		this->baseClass = baseClass;
		this->pos = pos;
		this->dir = dir;
		vel = dir;
		this->creator = creator;
		startTime = tTime;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = up, Entity* creator = nullptr) override
	{
		return make_unique<Grenade>(this, pos, dir, creator);
	}
};

namespace Updates
{
	void PlayerU(Entity* entity)
	{
		Player* player = static_cast<Player*>(entity);

		player->shouldVacuum ^= game->inputs.crouch.pressed;

		player->iTime = max(0.f, player->iTime - game->dTime);
		player->sTime = max(0.f, player->sTime - game->dTime);

		for (int i = 0; i < player->events.size(); i++)
			if (player->events[i].function(player, &player->events[i]))
				player->events.erase(player->events.begin() + i);

		if (player->currentMenuedEntity != nullptr && !player->currentMenuedEntity->Overlaps(game->inputs.mousePosition3 + player->pos, 0))
		{
			player->currentMenuedEntity->uiActive = false;
			player->currentMenuedEntity = nullptr;
		}

		Entity* hitEntity = nullptr;
		if (player->currentMenuedEntity == nullptr &&
			(hitEntity = game->entities->FirstOverlap(game->inputs.mousePosition3 + player->pos, 0, MaskF::IsCorporeal, player)) != nullptr)
		{
			if (player->currentMenuedEntity != nullptr)
				player->currentMenuedEntity->uiActive = false;
			player->currentMenuedEntity = hitEntity;
			player->currentMenuedEntity->uiActive = true;
		}

		if (player->sTime <= 0)
		{
			pMovements[UnEnum(player->movement)](player); // This handles all of the locomotion of the player.

			if (player->items.size() > 0) // You can't mod by 0.
				player->items.currentIndex = JMod(player->items.currentIndex + game->inputs.mouseScroll, static_cast<int>(player->items.size()));

			Item currentShootingItem = player->items.GetCurrentItem();

			if (game->inputs.middleMouse.released && player->heldEntity != nullptr)
			{
				player->heldEntity->observers.erase(std::find(player->heldEntity->observers.begin(), player->heldEntity->observers.end(), player));
				player->heldEntity = nullptr;
			}

			// Dragging code:
			if (player->heldEntity != nullptr)
				player->heldEntity->vel = TryAdd2(player->heldEntity->vel, Normalized(player->pos + game->inputs.mousePosition3 - player->heldEntity->pos) *
					game->dTime * player->holdMoveSpeed, player->maxHoldMoveSpeed * player->holdMoveWeight / max(player->holdMoveWeight, player->heldEntity->mass));

			// If can and should grab then do grag.
			if (player->heldEntity == nullptr && game->inputs.middleMouse.pressed &&
				(hitEntity = game->entities->FirstOverlap(game->inputs.mousePosition3 + player->pos, 0, MaskF::IsCorporeal, player)) != nullptr)
			{
				player->heldEntity = hitEntity;
				player->heldEntity->observers.push_back(player);
			}
			else // Do primaries, secondaries, and/or utilities.
			{
				if (game->inputs.leftMouse.held && tTime - player->lastPrimary >= player->primaryTime && primaries[UnEnum(player->primary)](player))
					player->lastPrimary = tTime;
				if (game->inputs.rightMouse.held && tTime - player->lastSecondary >= player->secondaryTime && secondaries[UnEnum(player->secondary)](player))
					player->lastSecondary = tTime;
				if (game->inputs.shift.held && tTime - player->lastUtility >= player->utilityTime && utilities[UnEnum(player->utility)](player))
					player->lastUtility = tTime;
			}

			 if (player->shouldVacuum)
				 game->entities->Vacuum(player->pos, player->vacDist, player->vacSpeed, player->maxVacSpeed, player->vacBoth, player->vacCollectibles);
		}

		vector<Entity*> collectibles = EntitiesOverlaps(player->pos, player->radius, game->entities->collectibles);
		for (Entity* collectible : collectibles)
		{
			player->items.push_back(((Collectible*)collectible)->baseItem);
			collectible->DestroySelf(player);
		}

		game->inputs.mouseScroll = 0;
	}

	void GrenadeU(Entity* entity)
	{
		Grenade* grenade = static_cast<Grenade*>(entity);
		if (tTime - grenade->startTime > grenade->timeTill)
		{
			CreateUpExplosion(grenade->pos, grenade->explosionRadius, grenade->color, grenade->name, 0, grenade->damage, grenade->creator);
			grenade->DestroySelf(nullptr);
			return;
		}
		grenade->range = grenade->baseRange * (0.5f - 0.5f * cosf(6 * PI_F * (tTime - grenade->startTime) / grenade->timeTill));
		grenade->lightSource->range = grenade->range;
	}
}

namespace DUpdates
{
	void PlayerDU(Entity* entity)
	{
		Player* player = static_cast<Player*>(entity);
		
		for (int i = 0; i < player->particles.size(); i++)
		{
			player->particles[i]->Update();
			if (player->particles[i]->ShouldEnd())
			{
				delete player->particles[i];
				player->particles.erase(player->particles.begin() + i);
				i--;
			}
		}
		
		Vec3 dir = Normalized(game->inputs.mousePosition3);
		float ratio = player->radius / SQRTTWO_F;
		player->DUpdate(DUPDATE::DTOCOL);
		game->DrawRightTri(player->pos + dir * ratio, vOne * ratio,
			atan2f(dir.y, dir.x) - PI_F * 0.5f, player->Color());
	}
}

unique_ptr<Grenade> grenade = make_unique<Grenade>(6, 5.f, 5.f, JRGB(255, 255), 15.f, 0.25f, RGBA(255, 255), 0.25f, 5, "Grenade");

namespace Secondaries
{
	bool GrenadeThrow(Player* player)
	{
		game->entities->push_back(grenade->Clone(player->pos, game->inputs.mousePosition3, player));
		return true;
	}

	bool TornadoSpinUndo(void* entity, TimedEvent* mEvent)
	{
		Player* player = static_cast<Player*>(entity);
		if (tTime - mEvent->startTime >= 1)
		{
			player->vUpdate = VUPDATE::FRICTION;
			return true;
		}
		else if (roundf(tTime * 5) != roundf((tTime - game->dTime) * 5))
			game->entities->TryDealDamageAll(1, player->pos, 3.f, MaskF::IsCorporealNotCollectible, player);
		return false;
	}

	bool TornadoSpin(Player* player)
	{
		player->sTime += 0.5f;
		player->iTime++;
		player->vUpdate = VUPDATE::ENTITY;
		player->vel = TryAdd2(player->vel, Normalized(game->inputs.mousePosition3) * 5.f, 20.f);
		player->events.push_back(TimedEvent(TornadoSpinUndo));
		player->particles.push_back(new TrackCircle(player, 3.f, RGBA(255, 255, 255, 102), 1.f));
		return true;
	}
}

namespace Utilities
{
	bool TacticoolRoll(Player* player)
	{
#define TACTICOOL_ROLL_SPEED 15.f
		Vec3 dir = game->inputs.MoveDir();

		if (dir == vZero)
			return false;
		player->vel = TryAdd2(player->vel, dir * TACTICOOL_ROLL_SPEED, 20.f);
		player->iTime++;
		player->sTime += 0.5f;
		return true;
	}

	bool MightyShove(Player* player)
	{
#define MIGHTY_SHOVE_DIST 20.f
#define MIGHTY_SHOVE_SPEED -200.f
#define MIGHTY_SHOVE_MAX_SPEED 15.f
		game->entities->VacuumBurst(player->pos, MIGHTY_SHOVE_DIST, MIGHTY_SHOVE_SPEED, MIGHTY_SHOVE_MAX_SPEED, true);
		game->entities->push_back(make_unique<FadeOutGlow>(MIGHTY_SHOVE_DIST * 2.f, 2.f, player->pos, MIGHTY_SHOVE_DIST, RGBA(255, 255, 255)));
		player->iTime++;
		return true;
	}
}