#include "Enemy.h"

class Player : public LightBlock
{
public:
	Entity* currentMenuedEntity = nullptr;
	Items items;
	int vacDist;
	bool placedBlock;
	Vec2 placingDir = up;
	float moveSpeed = 8.0f, maxSpeed = 4.0f, vacSpeed = 16.0f, lastClick = 0.0f,
		timePerHoldMove = 0.125, lastHoldMove = 0.0f;

	Player(iVec2 pos = vZero, float radius = 0.5f, int vacDist = 6, RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127),
		bool lightOrDark = true, RGBA subScat = RGBA(), float range = 10, float mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		LightBlock(lightColor, lightOrDark, range, pos, radius, color, color2, subScat, mass, maxHealth, health, name), vacDist(vacDist), lastClick(tTime)
	{
		update = UPDATE::PLAYERU;
		onDeath = ONDEATH::PLAYEROD;
		Start();
	}

	void Start() override
	{
		LightBlock::Start();
		items = Items();
		items.push_back(Resources::copper->Clone(10));
		items.push_back(Resources::cheese->Clone(3));
		items.push_back(Resources::shades->Clone(3));
		items.push_back(Resources::Seeds::copperTreeSeed->Clone(2));
		for (Item* item : Resources::Seeds::plantSeeds)
			items.push_back(item->Clone());
		items.currentIndex = 0; // Copper
	}
};

namespace Updates
{
	void PlayerU(Entity* entity)
	{
		Player* player = static_cast<Player*>(entity);

		bool exitedMenu = false;
		if (player->currentMenuedEntity != nullptr && (game->inputs.leftMouse.pressed || game->inputs.rightMouse.pressed)/* &&
			!currentMenuedEntity->PosInUIBounds(game->inputs.mousePosition + pos)*/)
		{
			player->currentMenuedEntity->shouldUI = false;
			player->currentMenuedEntity = nullptr;
			exitedMenu = true;
			player->lastClick = tTime;
		}

		vector<Entity*> hitEntities;
		if (game->inputs.rightMouse.pressed && game->inputs.mousePosition != Vec2(0) &&
			(player->currentMenuedEntity == nullptr || !player->currentMenuedEntity->Overlaps(game->inputs.mousePosition + Vec2(player->pos), 0.5f))
			&& (hitEntities = game->entities->FindCorpOverlaps(game->inputs.mousePosition + Vec2(player->pos), player->radius)).size())
		{
			if (player->currentMenuedEntity != nullptr)
				player->currentMenuedEntity->shouldUI = false;
			player->currentMenuedEntity = hitEntities[0];
			player->currentMenuedEntity->shouldUI = true;
		}

#pragma region Movement

		iVec2 direction(0, 0);

#pragma region Inputs
		if (game->inputs.a.held || game->inputs.left.held)
			direction.x--;
		if (game->inputs.d.held || game->inputs.right.held)
			direction.x++;
		if (game->inputs.s.held || game->inputs.down.held)
			direction.y--;
		if (game->inputs.w.held || game->inputs.up.held)
			direction.y++;
#pragma endregion

		player->TryMove(Vec2(direction) * game->dTime * player->moveSpeed, 3);
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
			if (tTime - player->lastHoldMove > player->timePerHoldMove)
			{
				player->lastHoldMove = tTime;
				player->heldEntity->TryMove(Rormalized(game->inputs.mousePosition + Vec2(player->pos - player->heldEntity->pos)), player->mass);
			}
		}
		std::cout << game->inputs.mousePosition.x << ", " << game->inputs.mousePosition.y << '\n';

		if (player->heldEntity == nullptr && game->inputs.middleMouse.pressed && game->inputs.mousePosition != Vec2(0) &&
			(hitEntities = game->entities->FindCorpOverlaps(game->inputs.mousePosition + Vec2(player->pos), 0.5f)).size())
		{
			player->heldEntity = hitEntities[0];
			player->heldEntity->holder = player;
		}
		else if (!game->inputs.space.held && tTime - player->lastClick > currentShootingItem.shootSpeed && player->currentMenuedEntity == nullptr &&
			game->inputs.mousePosition != Vec2(0) && currentShootingItem != *dItem && game->inputs.leftMouse.held && player->items.TryTake(currentShootingItem))
		{
			player->lastClick = tTime;
			game->entities->push_back(basicShotItem->Clone(currentShootingItem,
				player->pos, game->inputs.mousePosition, player));
		}

		if (player->heldEntity == nullptr && game->inputs.space.held)
			game->entities->Vacuum(player->pos, player->vacDist, player->vacSpeed);

		vector<Entity*> collectibles = EntitiesOverlaps(player->pos, player->radius, game->entities->collectibles);
		for (Entity* collectible : collectibles)
		{
			player->items.push_back(((Collectible*)collectible)->baseItem);
			collectible->DestroySelf(player);
		}

		game->inputs.mouseScroll = 0;
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