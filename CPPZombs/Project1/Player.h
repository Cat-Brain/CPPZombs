#include "Enemy.h"

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
	unique_ptr<UseableItem> waveModifier = make_unique<UseableItem>(ITEMU::WAVEMODIFIERU, "Wave Modifier", "Special Item", 0, RGBA(194, 99, 21), 1.f);
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
	SLINGSHOT, CIRCLEGUN
};
vector< std::function<bool(Player* player)>> primaries;

enum class SECONDARY
{
	BAYONET, TORNADO_SPIN
};
vector< std::function<bool(Player* player)>> secondaries;

enum class UTILITY
{
	TACTICOOL_ROLL, MIGHTY_SHOVE
};
vector< std::function<bool(Player* player)>> utilities;

class Player : public LightBlock
{
public:
	Entity* currentMenuedEntity = nullptr;
	Items items, startItems;
	vector<SEEDINDICES> blacklistedSeeds;
	Vec2 placingDir = up;
	float moveSpeed, maxSpeed, vacDist, vacSpeed, maxVacSpeed, holdMoveSpeed, shootSpeed,
		lastPrimary = 0, primaryTime, lastSecondary = 0, secondaryTime, lastUtility = 0, utilityTime;
	bool vacBoth, vacCollectibles;
	PMOVEMENT movement;
	PRIMARY primary;
	SECONDARY secondary;
	UTILITY utility;
	float iTime = 0, sTime = 0; // Invincibility time, if <= 0 takes damage, else doesn't.

	Player(bool vacBoth = false, bool vacCollectibles = true, float radius = 0.5f, float moveSpeed = 8, float maxSpeed = 8, float holdMoveSpeed = 8,
		float vacDist = 6, float vacSpeed = 16, float maxVacSpeed = 16, float shootSpeed = 1, float primaryTime = 1, float secondaryTime = 1,
		float utilityTime = 1,PMOVEMENT movement = PMOVEMENT::DEFAULT, PRIMARY primary = PRIMARY::SLINGSHOT, SECONDARY secondary = SECONDARY::BAYONET,
		UTILITY utility = UTILITY::TACTICOOL_ROLL,
		RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127), bool lightOrDark = true, float range = 10, float mass = 1,
		int maxHealth = 1, int health = 1, string name = "NULL NAME", Items startItems = {}, vector<SEEDINDICES> blacklistedSeeds = {}) :
		LightBlock(lightColor, lightOrDark, range, vZero, radius, color, color2, mass, maxHealth, health, name), vacDist(vacDist),
		moveSpeed(moveSpeed), holdMoveSpeed(holdMoveSpeed), startItems(startItems), blacklistedSeeds(blacklistedSeeds), vacBoth(vacBoth),
		vacCollectibles(vacCollectibles), vacSpeed(vacSpeed), maxSpeed(maxSpeed), maxVacSpeed(maxVacSpeed), shootSpeed(shootSpeed),
		primaryTime(primaryTime), secondaryTime(secondaryTime), utilityTime(utilityTime), movement(movement), primary(primary), secondary(secondary),
		utility(utility)
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

	Player(Player* baseClass, Vec2 pos) :
		Player(*baseClass)
	{
		this->pos = pos;
		this->baseClass = baseClass;
		iTime = 0;
		sTime = 0;
		Start();
	}

	unique_ptr<Player> PClone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr)
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
	}
};

unique_ptr<Player> commander = make_unique<Player>(false, true, 0.5f, 32.f, 8.f, 8.f, 6.f, 64.f, 16.f, 1.f, 0.f, 3.f, 2.f, PMOVEMENT::DEFAULT,
	PRIMARY::SLINGSHOT, SECONDARY::BAYONET, UTILITY::TACTICOOL_ROLL, RGBA(0, 0, 255), RGBA(), JRGB(127, 127, 127), true, 20.f, 5.f, 10, 5,
	"Commander", Items({ Resources::copper->Clone(10), Resources::Seeds::shadeTreeSeed->Clone(3), Resources::Seeds::cheeseVineSeed->Clone(1),
		Resources::Seeds::copperTreeSeed->Clone(3), Resources::waveModifier->Clone(1) }), vector<SEEDINDICES>({ SEEDINDICES::COPPER, SEEDINDICES::SHADE, SEEDINDICES::CHEESE }));
unique_ptr<Player> messenger = make_unique<Player>(true, true, 0.25f, 32.f, 12.f, 2.f, 4.f, 64.f, 8.f, 1.f, 0.f, 5.f, 5.f, PMOVEMENT::DEFAULT,
	PRIMARY::SLINGSHOT, SECONDARY::TORNADO_SPIN, UTILITY::MIGHTY_SHOVE, RGBA(255, 255), RGBA(0, 0, 255), JRGB(127, 127, 127), true, 5.f, 0.5f,
	3, 2, "Messenger", Items({ Resources::vacuumium->Clone(1000), Resources::Seeds::shadeTreeSeed->Clone(1), Resources::Seeds::cheeseVineSeed->Clone(3),
		Resources::Seeds::rockTreeSeed->Clone(3), Resources::waveModifier->Clone(1)}), vector<SEEDINDICES>({SEEDINDICES::ROCK, SEEDINDICES::SHADE, SEEDINDICES::CHEESE}));

vector<Player*> characters = { commander.get(), messenger.get() };

namespace Updates
{
	void PlayerU(Entity* entity)
	{
		Player* player = static_cast<Player*>(entity);

		player->iTime -= game->dTime;
		player->sTime -= game->dTime;
		player->iTime = max(0.f, player->iTime);
		player->sTime = max(0.f, player->sTime);

		bool exitedMenu = false;
		if (player->currentMenuedEntity != nullptr && !player->currentMenuedEntity->Overlaps(game->inputs.mousePosition + player->pos, 0))
		{
			player->currentMenuedEntity->uiActive = false;
			player->currentMenuedEntity = nullptr;
			exitedMenu = true;
		}

		vector<Entity*> hitEntities;
		if (player->currentMenuedEntity == nullptr &&
			(hitEntities = game->entities->FindCorpOverlaps(game->inputs.mousePosition + player->pos, 0)).size())
		{
			if (player->currentMenuedEntity != nullptr)
				player->currentMenuedEntity->uiActive = false;
			player->currentMenuedEntity = hitEntities[0];
			player->currentMenuedEntity->uiActive = true;
		}

		if (player->sTime <= 0)
		{
			pMovements[UnEnum(player->movement)](player); // This handles all of the locomotion of the player.

			if (player->heldEntity == nullptr && player->items.size() > 0) // You can't mod by 0.
				player->items.currentIndex = JMod(player->items.currentIndex + game->inputs.mouseScroll, static_cast<int>(player->items.size()));

			Item currentShootingItem = player->items.GetCurrentItem();

			if (game->inputs.middleMouse.released && player->heldEntity != nullptr)
			{
				player->heldEntity->holder = nullptr;
				player->heldEntity = nullptr;
			}

			if (player->heldEntity != nullptr)
			{
				RotateLeft(player->heldEntity->dir, game->inputs.mouseScroll);
				player->heldEntity->SetPos(FromTo(player->heldEntity->pos, player->pos + game->inputs.mousePosition,
					game->dTime * player->holdMoveSpeed / player->heldEntity->mass));
			}

			if (player->heldEntity == nullptr && game->inputs.middleMouse.pressed &&
				(hitEntities = game->entities->FindCorpOverlaps(game->inputs.mousePosition + player->pos, 0)).size() && hitEntities[0] != player)
			{
				player->heldEntity = hitEntities[0];
				player->heldEntity->holder = player;
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

			if (player->heldEntity == nullptr && game->inputs.space.held)
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
}

namespace DUpdates
{
	void PlayerDU(Entity* entity)
	{
		Player* player = static_cast<Player*>(entity);
		
		Vec2 dir = Normalized(game->inputs.mousePosition);
		float ratio = player->radius / SQRTTWO_F;
		game->DrawRightTri(player->pos + dir * ratio, vOne * (ratio * 2),
			atan2f(dir.y, dir.x) - PI_F * 0.5f, player->color);
		player->DUpdate(DUPDATE::DTOCOL);
	}
}

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
	void WaveModifierU(Item* stack, Vec2 pos, Entity* creator, string creatorName, Entity* callReason, int callType)
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
		iVec2 direction(0, 0);

		if (game->inputs.a.held || game->inputs.left.held)
			direction.x--;
		if (game->inputs.d.held || game->inputs.right.held)
			direction.x++;
		if (game->inputs.s.held || game->inputs.down.held)
			direction.y--;
		if (game->inputs.w.held || game->inputs.up.held)
			direction.y++;

		player->vel = TryAdd2(player->vel, Normalized(direction) * game->dTime * player->moveSpeed, player->maxSpeed);
	}
}

namespace Primaries
{
	bool Slingshot(Player* player)
	{
		Item currentShootingItem = player->items.GetCurrentItem();
		if (currentShootingItem == *dItem || tTime - player->lastPrimary <= currentShootingItem.useTime * player->shootSpeed)
			return false;
		player->items[player->items.currentIndex].Use(player->pos, player, player->name, nullptr, 0);
		player->items.RemoveIfEmpty(player->items.currentIndex);
		player->lastPrimary = tTime - currentShootingItem.useTime;
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
	bool Bayonet(Player* player)
	{
		std::cout << "Bayonet\t";
		return true;
	}

	bool TornadoSpin(Player* player)
	{
		std::cout << "Tornado spin\t";
		return true;
	}
}

namespace Utilities
{
	bool TacticoolRoll(Player* player)
	{
#define TACTICOOL_ROLL_SPEED 15.f
		iVec2 direction(0, 0);

		if (game->inputs.a.held || game->inputs.left.held)
			direction.x--;
		if (game->inputs.d.held || game->inputs.right.held)
			direction.x++;
		if (game->inputs.s.held || game->inputs.down.held)
			direction.y--;
		if (game->inputs.w.held || game->inputs.up.held)
			direction.y++;

		if (direction == vZeroI)
			return false;
		player->vel += Normalized(direction) * TACTICOOL_ROLL_SPEED;
		player->iTime++;
		player->sTime++;
		return true;
	}

	bool MightyShove(Player* player)
	{
#define MIGHTY_SHOVE_DIST 10.f
#define MIGHTY_SHOVE_SPEED -200.f
#define MIGHTY_SHOVE_MAX_SPEED 15.f
		game->entities->VacuumBurst(player->pos, MIGHTY_SHOVE_DIST, MIGHTY_SHOVE_SPEED, MIGHTY_SHOVE_MAX_SPEED, true);
		player->iTime++;
		return true;
	}
}