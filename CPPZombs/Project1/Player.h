#include "Livestock.h"

class UseableItem : public Item
{
public:
	UseableItem(ITEMTYPE type, ITEMU useFunction, string name = "NULL NAME", string typeName = "NULL TYPE NAME", int intType = 0, RGBA color = RGBA(), float useTime = 0) :
		Item(type, name, typeName, intType, color, 0, 0, useTime)
	{
		itemU = useFunction;
	}
};

namespace Resources
{
	UseableItem* waveModifier = new UseableItem(ITEMTYPE::WAVE_MODIFIER, ITEMU::WAVEMODIFIER, "Wave Modifier", "Special Item", 0, RGBA(194, 99, 21), 1.f);
}

#define NUM_START_ITEMS 3

int difficultySeedSpawnQuantity[] = { 7, 5, 3 };


enum class PMOVEMENT
{
	DEFAULT, JETPACK
};
vector<std::function<void(Player* player)>> pMovements;

enum class PRIMARY
{
	SLINGSHOT, ENG_SHOOT, CIRCLEGUN, CHOMP
};
vector< std::function<bool(Player* player)>> primaries;

enum class SECONDARY
{
	GRENADE_THROW, TORNADO_SPIN, ENGMODEUSE, VINE_SHOT
};
vector< std::function<bool(Player* player)>> secondaries;

enum class UTILITY
{
	TACTICOOL_ROLL, MIGHTY_SHOVE, ENGMODESWAP
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
	bool vacBoth, vacCollectibles, shouldVacuum = true, shouldPickup = true, shouldScroll = true;
	PMOVEMENT movement;
	PRIMARY primary;
	SECONDARY secondary;
	UTILITY utility;
	vector<TimedEvent> events{};
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
		uiUpdate = UIUPDATE::PLAYER;
		uiActive = true;
		onDeath = ONDEATH::PLAYER;
	}

	void Start() override
	{
		LightBlock::Start(); // Set up all of the light stuff.

		iTime = 0;
		sTime = 0;

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
			items.push_back(Resources::Seeds::plantSeeds[newIndex]->Clone(difficultySeedSpawnQuantity[game->settings.difficulty]));
			i++;
		}
		items.currentIndex = 0; // Set active ammo type to the first ammo type in inventory.
	}

	Player(Player* baseClass, Vec3 pos) :
		Player(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		Start();
	}

	virtual unique_ptr<Player> PClone(Vec3 pos = vZero, Vec3 dir = up, Entity* creator = nullptr)
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

enum class ENGMODE
{
	TURRET, DRONE, ROVER, REMOVE_DRONE, PLACE, COUNT
};

string engModeStr[] = {"Turret", "Drone", "Rover", "Remove Drone", "Place"};

class Drone;
class Engineer : public Player
{
public:
	ENGMODE engMode = ENGMODE::TURRET;
	vector<FadeCircle*> drones;

	Engineer(bool vacBoth = false, bool vacCollectibles = true, float radius = 0.5f, float moveSpeed = 8, float maxSpeed = 8,
		float holdMoveSpeed = 32, float maxHoldMoveSpeed = 8, float holdMoveWeight = 4, float vacDist = 6, float vacSpeed = 16,
		float maxVacSpeed = 16, float shootSpeed = 1, float primaryTime = 1, float secondaryTime = 1,
		float utilityTime = 1, PMOVEMENT movement = PMOVEMENT::DEFAULT, PRIMARY primary = PRIMARY::SLINGSHOT,
		SECONDARY secondary = SECONDARY::GRENADE_THROW, UTILITY utility = UTILITY::TACTICOOL_ROLL,
		RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127), bool lightOrDark = true, float range = 10, float mass = 1,
		int maxHealth = 1, int health = 1, string name = "NULL NAME", Items startItems = {}, vector<SEEDINDICES> blacklistedSeeds = {}) :
		Player(vacBoth, vacCollectibles, radius, moveSpeed, maxSpeed, holdMoveSpeed, maxHoldMoveSpeed, holdMoveWeight, vacDist, vacSpeed,
			maxVacSpeed, shootSpeed, primaryTime, secondaryTime, utilityTime, movement, primary, secondary, utility, color, color2,
			lightColor, lightOrDark, range, mass, maxHealth, health, name, startItems, blacklistedSeeds)
	{
		update = UPDATE::ENGINEER;
		uiUpdate = UIUPDATE::ENGINEER;
	}

	Engineer(Engineer* baseClass, Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) :
		Engineer(*baseClass)
	{
		this->baseClass = baseClass;
		this->pos = pos;
		this->creator = creator;
		engMode = ENGMODE::TURRET;
		drones = {};
		Start();
	}

	unique_ptr<Player> PClone(Vec3 pos = vZero, Vec3 dir = up, Entity* creator = nullptr) override
	{
		return make_unique<Engineer>(this, pos, dir, creator);
	}
};

class Turret : public LightBlock
{
public:
	float timeTill, timePer;
	Items* items = nullptr;
	Engineer* leader = nullptr;
	
	Turret(float timePer,
		JRGB lightColor, float range, float radius, RGBA color, RGBA color2, float mass, int maxHealth, int health, string name) :
		LightBlock(lightColor, true, range, vZero, radius, color, color2, mass, maxHealth, health, name),
		timePer(timePer), timeTill(timePer)
	{
		update = UPDATE::TURRET;
		dUpdate = DUPDATE::TURRET;
	}

	Turret(Turret* baseClass, Vec3 pos, Vec3 dir, Engineer* creator) :
		Turret(*baseClass)
	{
		this->pos = pos;
		this->dir = dir;
		this->creator = creator;
		leader = creator;
		items = &creator->items;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<Turret>(this, pos, dir, static_cast<Engineer*>(creator));
	}
};

class Rover : public LightBlock
{
public:
	float vacSpeed, maxVacSpeed, vacDist, moveSpeed, maxSpeed, timeTillJump, timePerJump, remainingLifetime, lifetime;
	Items* items = nullptr;
	Engineer* leader = nullptr;

	Rover(float vacSpeed, float maxVacSpeed, float vacDist, float moveSpeed, float maxSpeed, float timePerJump, float lifetime,
		JRGB lightColor, float range, float radius, RGBA color, RGBA color2, float mass, int maxHealth, int health, string name) :
		LightBlock(lightColor, true, range, vZero, radius, color, color2, mass, maxHealth, health, name),
		vacSpeed(vacSpeed), maxVacSpeed(maxVacSpeed), vacDist(vacDist), moveSpeed(moveSpeed), maxSpeed(maxSpeed),
		timeTillJump(0), timePerJump(timePerJump), remainingLifetime(lifetime), lifetime(lifetime)
	{
		update = UPDATE::ROVER;
		dUpdate = DUPDATE::ROVER;
	}

	Rover(Rover* baseClass, Vec3 pos, Vec3 dir, Engineer* creator) :
		Rover(*baseClass)
	{
		this->pos = pos;
		this->dir = Normalized(dir);
		this->creator = creator;
		leader = creator;
		items = &creator->items;
		remainingLifetime = lifetime;
		Start();
	}

	unique_ptr<Entity> Clone(Vec3 pos = vZero, Vec3 dir = vZero, Entity* creator = nullptr) override
	{
		return make_unique<Rover>(this, pos, dir, static_cast<Engineer*>(creator));
	}
};

unique_ptr<Turret> turret = make_unique<Turret>(2.f, JRGB(255), 10.f, 0.5f, RGBA(255), RGBA(), 0.5f, 5, 5, "Turret");
unique_ptr<Rover> rover = make_unique<Rover>(32.f, 8.f, 4.f, 4.f, 4.f, 2.f, 10.f, JRGB(255, 255), 3.f, 0.25f, RGBA(255, 255), RGBA(), 0.1f, 0, 0, "Drone");

unique_ptr<Player> soldier = make_unique<Player>(false, true, 0.4f, 32.f, 8.f, 32.f, 8.f, 4.f, 6.f, 64.f, 32.f, 1.f, 0.f, 2.f, 4.f, PMOVEMENT::JETPACK,
	PRIMARY::SLINGSHOT, SECONDARY::GRENADE_THROW, UTILITY::TACTICOOL_ROLL, RGBA(0, 0, 255), RGBA(), JRGB(127, 127, 127), true, 20.f, 5.f, 10, 5,
	"Soldier", Items({ Resources::copper->Clone(10), Resources::Seeds::shadeTreeSeed->Clone(3), Resources::Seeds::cheeseVineSeed->Clone(1),
		Resources::Seeds::rubyTreeSeed->Clone(2), Resources::Seeds::copperTreeSeed->Clone(3), Resources::waveModifier->Clone(1), Resources::Eggs::kiwiEgg->Clone(2)}),
	vector<SEEDINDICES>({ SEEDINDICES::COPPER, SEEDINDICES::SHADE, SEEDINDICES::CHEESE, SEEDINDICES::RUBY }));

unique_ptr<Player> flicker = make_unique<Player>(false, true, 0.25f, 32.f, 12.f, 32.f, 8.f, 2.f, 4.f, 64.f, 32.f, 1.f, 0.f, 2.f, 5.f, PMOVEMENT::DEFAULT,
	PRIMARY::SLINGSHOT, SECONDARY::TORNADO_SPIN, UTILITY::MIGHTY_SHOVE, RGBA(255, 255), RGBA(0, 0, 255), JRGB(127, 127, 127), true, 5.f, 1.5f,
	3, 2, "Flicker", Items({ Resources::rock->Clone(10), Resources::Seeds::shadeTreeSeed->Clone(1), Resources::Seeds::cheeseVineSeed->Clone(3),
		Resources::Seeds::quartzVineSeed->Clone(2), Resources::Seeds::rockTreeSeed->Clone(3), Resources::waveModifier->Clone(1), Resources::Eggs::kiwiEgg->Clone(2) }),
	vector<SEEDINDICES>({ SEEDINDICES::ROCK, SEEDINDICES::SHADE, SEEDINDICES::CHEESE, SEEDINDICES::QUARTZ_V }));

unique_ptr<Engineer> engineer = make_unique<Engineer>(false, true, 0.49f, 32.f, 8.f, 32.f, 8.f, 2.f, 4.f, 0.f, 0.f, 1.f, 0.f, 2.f, 0.f, PMOVEMENT::DEFAULT,
	PRIMARY::ENG_SHOOT, SECONDARY::ENGMODEUSE, UTILITY::ENGMODESWAP, RGBA(255, 0, 255), RGBA(0, 0, 0), JRGB(127, 127, 127), true, 20.f, 5.f,
	10, 5, "Engineer", Items({ Resources::silver->Clone(10), Resources::Seeds::shadeTreeSeed->Clone(2), Resources::Seeds::cheeseVineSeed->Clone(2),
		Resources::Seeds::quartzTreeSeed->Clone(2), Resources::Seeds::silverTreeSeed->Clone(3), Resources::waveModifier->Clone(1), Resources::Eggs::kiwiEgg->Clone(2) }),
	vector<SEEDINDICES>({ SEEDINDICES::SILVER, SEEDINDICES::SHADE, SEEDINDICES::CHEESE, SEEDINDICES::QUARTZ_T }));

vector<Player*> characters = { soldier.get(), flicker.get(), engineer.get() };

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

unique_ptr<Grenade> grenade = make_unique<Grenade>(6, 5.f, 3.f, JRGB(255, 255), 15.f, 0.25f, RGBA(255, 255), 0.25f, 5, "Grenade");

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
	void WaveModifierU(ItemInstance item, Vec3 pos, Vec3 dir, Entity* creator, string creatorName, Entity* callReason, int callType)
	{
		game->planet->enemies->superWave ^= true;
		game->entities->push_back(make_unique<FadeOutGlow>(8.f, 3.f, pos, 4.f, item->color));
		game->screenShake++;
	}
}

namespace Updates
{
	void PlayerU(Entity* entity)
	{
		Player* player = static_cast<Player*>(entity);

		player->shouldVacuum ^= game->inputs.crouch.pressed;
		printf("%i=%c ", player->shouldVacuum, player->shouldVacuum ? 't' : 'f');

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

			if (player->shouldScroll && player->items.size() > 0) // You can't mod by 0.
				player->items.currentIndex = JMod(player->items.currentIndex + game->inputs.mouseScroll, static_cast<int>(player->items.size()));

			ItemInstance currentShootingItem = player->items.GetCurrentItem();

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

		if (player->shouldPickup)
		{
			vector<Entity*> collectibles = EntitiesOverlaps(player->pos, player->radius, game->entities->collectibles);
			for (Entity* collectible : collectibles)
			{
				player->items.push_back(((Collectible*)collectible)->baseItem);
				collectible->DestroySelf(player);
			}
		}
		if (player->shouldScroll)
			game->inputs.mouseScroll = 0;
	}

	void TurretU(Entity* entity)
	{
		Turret* turret = static_cast<Turret*>(entity);
		turret->timeTill -= game->dTime;
		ItemInstance item = turret->items->GetCurrentItem();
		Entity* hitEntity = nullptr;
		if (turret->leader->shouldVacuum && item.count > 0 && turret->timeTill <= 0 && (hitEntity = game->entities->FirstOverlap(turret->pos, item->range, MaskF::IsEnemy, turret)) != nullptr)
		{
			turret->dir = hitEntity->pos - turret->pos;
			item->Use((*turret->items)[turret->items->currentIndex], turret->pos, turret->dir * item->range, turret, turret->name, hitEntity, 0);
			turret->items->RemoveIfEmpty(turret->items->currentIndex);
			turret->timeTill = turret->timePer * item->useTime;
		}
	}

	void RoverU(Entity* entity)
	{
		Rover* rover = static_cast<Rover*>(entity);

		rover->vel = Vec3(TryAdd2V2(rover->vel, rover->dir * game->dTime * (rover->moveSpeed + game->planet->friction),
			rover->maxSpeed), rover->vel.z);
		
		rover->remainingLifetime -= game->dTime;
		if (rover->remainingLifetime < 0)
			rover->DestroySelf(rover);
		rover->timeTillJump -= game->dTime;
		if (rover->timeTillJump < 0 &&
			game->entities->OverlapsTile(rover->pos + Vec3(0, 0, rover->radius * 2), rover->radius * 1.9f))
		{
			rover->vel.z += 7.f;
			rover->timeTillJump = rover->timePerJump;
		}
		game->entities->Vacuum(rover->pos, rover->vacDist, rover->vacSpeed, rover->maxVacSpeed, false, true);

		vector<Entity*> collectibles = EntitiesOverlaps(rover->pos, rover->radius, game->entities->collectibles);
		for (Entity* collectible : collectibles)
		{
			rover->leader->items.push_back(((Collectible*)collectible)->baseItem);
			collectible->DestroySelf(rover);
		}
	}

	void EngineerU(Entity* entity)
	{
		Engineer* engineer = static_cast<Engineer*>(entity);
		float offset = tTime * PI_F * 2 / 5;
		for (int i = 0; i < engineer->drones.size(); i++)
		{
			engineer->drones[i]->startTime = tTime;
			engineer->drones[i]->pos = engineer->pos + Vec3(engineer->radius * 2 * CircPoint2(offset + PI_F * 2 * i / engineer->drones.size()), 0.f);
		}
		engineer->shouldScroll = !game->inputs.shift.held;
		engineer->shouldPickup = false;
		engineer->Update(UPDATE::PLAYER);
	}

	void GrenadeU(Entity* entity)
	{
		Grenade* grenade = static_cast<Grenade*>(entity);
		if (tTime - grenade->startTime > grenade->timeTill)
		{
			CreateUpExplosion(grenade->pos, grenade->explosionRadius, grenade->color, grenade->name, 0, grenade->damage, grenade->creator);
			if (grenade->creator != nullptr && grenade->creator->Overlaps(grenade->pos, grenade->explosionRadius))
				grenade->creator->vel += 10.f * Normalized(grenade->creator->pos - grenade->pos);
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

		Vec3 dir = Normalized(game->inputs.mousePosition3);
		float ratio = player->radius / SQRTTWO_F;
		player->DUpdate(DUPDATE::DTOCOL);
		game->DrawRightTri(player->pos + dir * ratio, vOne * ratio,
			atan2f(dir.y, dir.x) - PI_F * 0.5f, player->Color());
	}

	void TurretDU(Entity* entity)
	{
		Turret* turret = static_cast<Turret*>(entity);

		turret->DUpdate(DUPDATE::DTOCOL);
	}

	void RoverDU(Entity* entity)
	{
		Rover* rover = static_cast<Rover*>(entity);

		rover->DUpdate(DUPDATE::DTOCOL);
	}
}

namespace UIUpdates
{
	void PlayerUIU(Entity* entity)
	{
		Player* player = static_cast<Player*>(entity);
		player->items.DUpdate();
	}

	void EngineerUIU(Entity* entity)
	{
		Engineer* engineer = static_cast<Engineer*>(entity);
		if (!game->inputs.shift.held)
		{
			engineer->items.DUpdate();
			return;
		}
		float scale = ScrHeight() / (3.0f * max(8, int(UnEnum(ENGMODE::COUNT)))), scale2 = scale / 5.0f;
		for (int i = 0; i < UnEnum(ENGMODE::COUNT); i++)
		{
			font.Render(engModeStr[i],
				iVec2(static_cast<int>(-ScrWidth() + scale * 2), static_cast<int>(-ScrHeight() + scale * 2 * i)), scale * 2,
				i == UnEnum(engineer->engMode) ? RGBA(127, 127, 127) : RGBA(255, 255, 255));
		}
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

	void Jetpack(Player* player)
	{
		pMovements[UnEnum(PMOVEMENT::DEFAULT)](player);
		if (game->inputs.space.held)
			player->vel.z += game->planet->gravity * game->dTime;
	}
}

namespace Primaries
{
	bool Slingshot(Player* player)
	{
		ItemInstance currentShootingItem = player->items.GetCurrentItem();
		if (currentShootingItem == dItem->Clone() || tTime - player->lastPrimary <= currentShootingItem->useTime * player->shootSpeed)
			return false;
		player->items[player->items.currentIndex]->Use(player->items[player->items.currentIndex], player->pos, game->inputs.mousePosition3, player, player->name, nullptr, 0);
		player->items.RemoveIfEmpty(player->items.currentIndex);
		player->lastPrimary = tTime + player->primaryTime;
		return false;
	}

	bool EngShoot(Player* player)
	{
		ItemInstance currentShootingItem = player->items.GetCurrentItem();
		if (currentShootingItem == dItem->Clone() || tTime - player->lastPrimary <= currentShootingItem->useTime * player->shootSpeed)
			return false;
		player->items[player->items.currentIndex]->Use(player->items[player->items.currentIndex], player->pos, game->inputs.mousePosition3, player, player->name, nullptr, 0);
		player->lastPrimary = tTime + player->primaryTime;
		Engineer* engineer = static_cast<Engineer*>(player);
		if (player->items.RemoveIfEmpty(player->items.currentIndex) != Items::TRYTAKE::DECREMENTED || !engineer->shouldVacuum) return false;
		for (Circle* circle : engineer->drones)
			if (!game->entities->OverlapsTile(circle->pos, currentShootingItem->radius))
			{
				player->items[player->items.currentIndex]->Use(player->items[player->items.currentIndex], circle->pos, game->inputs.mousePosition3, player, player->name, nullptr, 0);
				if (player->items.RemoveIfEmpty(player->items.currentIndex) != Items::TRYTAKE::DECREMENTED) return false;
			}
		return false;
	}

	bool CircleGun(Player* player)
	{
		std::cout << "Circle gun\t";
		return true;
	}
}

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
			game->entities->TryDealDamageAllUp(1, player->pos, 3.f, MaskF::IsCorporealNotCollectible, player);
		return false;
	}

	bool TornadoSpin(Player* player)
	{
		player->sTime += 0.5f;
		player->iTime++;
		player->vUpdate = VUPDATE::ENTITY;
		player->vel = TryAdd2(player->vel, Normalized(game->inputs.mousePosition3) * 5.f, 20.f);
		player->events.push_back(TimedEvent(TornadoSpinUndo));
		game->entities->particles.push_back(make_unique<TrackCircle>(player, 3.f, RGBA(255, 255, 255, 102), 1.f));
		return true;
	}

	bool EngModeUse(Player* player)
	{
		Engineer* engineer = static_cast<Engineer*>(player);
		switch (engineer->engMode)
		{
		case ENGMODE::TURRET:
		{
			game->entities->push_back(turret->Clone(engineer->pos, game->inputs.mousePosition3, engineer));
			break;
		}
		case ENGMODE::DRONE:
		{
			unique_ptr<FadeCircle> newDrone = make_unique<FadeCircle>(0.1f, engineer->pos, RGBA(0, 255), 1.f);
			engineer->drones.push_back(newDrone.get());
			game->entities->particles.push_back(std::move(newDrone));
			break;
		}
		case ENGMODE::ROVER:
		{
			game->entities->push_back(rover->Clone(engineer->pos, game->inputs.mousePosition3, engineer));
			break;
		}
		case ENGMODE::REMOVE_DRONE:
		{
			engineer->drones.pop_back();
			break;
		}
		case ENGMODE::PLACE:
		{
			Vec3 pos = engineer->pos + game->inputs.mousePosition3;
			if (!game->entities->CubeDoesOverlap(ToIV3(pos - engineer->radius), ToIV3(pos + engineer->radius), MaskF::IsCorporealNotCollectible))
				game->entities->ChunkAtFPos(pos)->SetTileAtPos(ToIV3(pos), UnEnum(TILE::ROCK));
			return false;
		}
		}
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

	bool EngModeSwap(Player* player)
	{
		Engineer* engineer = static_cast<Engineer*>(player);
		engineer->engMode = ENGMODE(JMod(UnEnum(engineer->engMode) + game->inputs.mouseScroll, UnEnum(ENGMODE::COUNT)));
		game->inputs.mouseScroll = 0;
		return false;
	}
}