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

class Player : public LightBlock
{
public:
	Entity* currentMenuedEntity = nullptr;
	Items items, startItems;
	vector<int> blacklistedSeeds;
	Vec2 placingDir = up;
	float moveSpeed, maxSpeed, vacDist, vacSpeed, maxVacSpeed, lastClick,
		holdMoveSpeed;
	bool vacBoth, vacCollectibles;

	Player(bool vacBoth = false, bool vacCollectibles = true, float radius = 0.5f, float moveSpeed = 8, float maxSpeed = 8, float holdMoveSpeed = 8,
		float vacDist = 6, float vacSpeed = 16, float maxVacSpeed = 16,
		RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127), bool lightOrDark = true, float range = 10, float mass = 1,
		int maxHealth = 1, int health = 1, string name = "NULL NAME", Items startItems = {}, vector<int> blacklistedSeeds = {}) :
		LightBlock(lightColor, lightOrDark, range, vZero, radius, color, color2, mass, maxHealth, health, name), vacDist(vacDist), lastClick(tTime),
		moveSpeed(moveSpeed), holdMoveSpeed(holdMoveSpeed), startItems(startItems), blacklistedSeeds(blacklistedSeeds), vacBoth(vacBoth),
		vacCollectibles(vacCollectibles), vacSpeed(vacSpeed), maxSpeed(maxSpeed), maxVacSpeed(maxVacSpeed)
	{
		update = UPDATE::PLAYERU;
		dUpdate = DUPDATE::PLAYERDU;
		onDeath = ONDEATH::PLAYEROD;
	}

	void Start() override
	{
		LightBlock::Start(); // Set up all of the light stuff.

		lastClick = tTime;

		items = startItems;

		vector<int> spawnedIndices(NUM_START_ITEMS + blacklistedSeeds.size(), -1);
		for (int i = 0; i < blacklistedSeeds.size(); i++)
			spawnedIndices[size_t(i) + NUM_START_ITEMS] = blacklistedSeeds[i];
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
		Start();
	}

	unique_ptr<Player> PClone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr)
	{
		return make_unique<Player>(this, pos);
	}

	int DealDamage(int damage, Entity* damageDealer) override
	{
		game->screenShake += damage;
		return LightBlock::DealDamage(damage, damageDealer);
	}
};

unique_ptr<Player> commander = make_unique<Player>(false, true, 0.5f, 32.f, 8.f, 8.f, 6.f, 64.f, 16.f, RGBA(0, 0, 255), RGBA(), JRGB(127, 127, 127), true, 20.f, 5.f, 10, 5,
	"Commander", Items({ Resources::copper->Clone(10), Resources::Seeds::shadeTreeSeed->Clone(3), Resources::Seeds::cheeseVineSeed->Clone(1),
		Resources::Seeds::copperTreeSeed->Clone(3), Resources::waveModifier->Clone(1) }), vector<int>({ SEEDINDICES::COPPER, SEEDINDICES::SHADE, SEEDINDICES::CHEESE }));
unique_ptr<Player> messenger = make_unique<Player>(true, true, 0.25f, 32.f, 12.f, 2.f, 4.f, 64.f, 8.f, RGBA(255, 255), RGBA(0, 0, 255), JRGB(127, 127, 127), true, 5.f, 0.5f,
	3, 2, "Messenger", Items({ Resources::vacuumium->Clone(1000), Resources::Seeds::shadeTreeSeed->Clone(1), Resources::Seeds::cheeseVineSeed->Clone(3),
		Resources::Seeds::rockTreeSeed->Clone(3), Resources::waveModifier->Clone(1)}), vector<int>({SEEDINDICES::ROCK, SEEDINDICES::SHADE, SEEDINDICES::CHEESE}));

vector<Player*> characters = { commander.get(), messenger.get() };

namespace Updates
{
	void PlayerU(Entity* entity)
	{
		Player* player = static_cast<Player*>(entity);

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

#pragma region Movement
		iVec2 direction(0, 0);

		if (game->inputs.a.held || game->inputs.left.held)
			direction.x--;
		if (game->inputs.d.held || game->inputs.right.held)
			direction.x++;
		if (game->inputs.s.held || game->inputs.down.held)
			direction.y--;
		if (game->inputs.w.held || game->inputs.up.held)
			direction.y++;

		player->vel = TryAdd(player->vel, Normalized(direction) * game->dTime * player->moveSpeed, player->maxSpeed);
		std::cout << player->maxSpeed << ", " << glm::length(player->vel) << '\n';
#pragma endregion

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
		else if (game->inputs.leftMouse.held && !game->inputs.space.held && game->inputs.mousePosition != vZero &&
			currentShootingItem != *dItem && tTime - player->lastClick > currentShootingItem.useTime)
		{
			player->items[player->items.currentIndex].Use(player->pos, player, player->name, nullptr, 0);
			player->items.RemoveIfEmpty(player->items.currentIndex);
			player->lastClick = tTime;
		}

		if (player->heldEntity == nullptr && game->inputs.space.held)
			game->entities->Vacuum(player->pos, player->vacDist, player->vacSpeed, player->maxVacSpeed, player->vacBoth, player->vacCollectibles);

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
		player->DUpdate(DUPDATE::DTOCOLDU);
	}
}

namespace OnDeaths
{
	void PlayerOD(Entity* entity, Entity* damageDealer)
	{
		entity->OnDeath(ONDEATH::LIGHTBLOCKOD, damageDealer);
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