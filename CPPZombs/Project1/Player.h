#include "Enemy.h"

class Player : public LightBlock
{
public:
	Entity* currentMenuedEntity = nullptr;
	Items items;
	int vacDist;
	bool placedBlock;
	Vec2f placingDir = up;
	float timePerMove = 0.125f, lastMove = 0.0f, maxSpeed = 4.0f, lastVac = -1.0f,
		timePerVac = 0.125f, lastClick = 0.0f,
		timePerHoldMove = timePerMove, lastHoldMove = 0.0f;

	Player(Vec2 pos = vZero, Vec2 dimensions = vOne, int vacDist = 6, RGBA color = RGBA(), RGBA color2 = RGBA(), JRGB lightColor = JRGB(127, 127, 127),
		bool lightOrDark = true, RGBA subScat = RGBA(), float range = 10, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		LightBlock(lightColor, lightOrDark, range, pos, dimensions, color, color2, subScat, mass, maxHealth, health, name), vacDist(vacDist)
	{
		update = UPDATE::PLAYERU;
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

	void OnDeath(Entity* damageDealer) override
	{
		LightBlock::OnDeath(damageDealer);
		playerAlive = false;
		if (damageDealer != nullptr)
			deathCauseName = damageDealer->name;
		else
			deathCauseName = "The planet";
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
		if (game->inputs.rightMouse.pressed && game->inputs.mousePosition != vZero &&
			(player->currentMenuedEntity == nullptr || !player->currentMenuedEntity->Overlaps(game->inputs.mousePosition + player->pos, vOne))
			&& (hitEntities = game->entities->FindCorpOverlaps(game->inputs.mousePosition + player->pos, player->dimensions)).size())
		{
			if (player->currentMenuedEntity != nullptr)
				player->currentMenuedEntity->shouldUI = false;
			player->currentMenuedEntity = hitEntities[0];
			player->currentMenuedEntity->shouldUI = true;
		}

#pragma region Movement

		if (game->inputs.w.pressed || game->inputs.up.pressed || game->inputs.a.pressed || game->inputs.left.pressed ||
			game->inputs.s.pressed || game->inputs.down.pressed || game->inputs.d.pressed || game->inputs.right.pressed)
			player->lastMove = tTime;

		if (tTime - player->lastMove >= player->timePerMove)
		{
			Vec2 direction(0, 0);

#pragma region Inputs
			if (game->inputs.a.held || game->inputs.left.held)
			{
				game->inputs.a.held = false;
				game->inputs.left.held = false;
				direction.x--;
			}
			if (game->inputs.d.held || game->inputs.right.held)
			{
				game->inputs.d.held = false;
				game->inputs.right.held = false;
				direction.x++;
			}
			if (game->inputs.s.held || game->inputs.down.held)
			{
				game->inputs.s.held = false;
				game->inputs.down.held = false;
				direction.y--;
			}
			if (game->inputs.w.held || game->inputs.up.held)
			{
				game->inputs.w.held = false;
				game->inputs.up.held = false;
				direction.y++;
			}
#pragma endregion

			Vec2 oldPos = player->pos;
			if (direction != vZero)
			{
				player->lastMove = tTime;
				player->TryMove(direction, 3);
			}

			game->inputs.mousePosition += player->pos - oldPos;
		}

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
			player->heldEntity->dir.RotateLeft(game->inputs.mouseScroll);
			if (tTime - player->lastHoldMove > player->timePerHoldMove)
			{
				player->lastHoldMove = tTime;
				player->heldEntity->TryMove(Vec2f(player->pos + game->inputs.mousePosition - player->heldEntity->pos).Rormalized(), player->mass);
			}
		}


		Vec2 normalizedDir = Vec2f(game->inputs.mousePosition).Normalized();
		if (player->heldEntity == nullptr && game->inputs.middleMouse.pressed && game->inputs.mousePosition != vZero &&
			(hitEntities = game->entities->FindCorpOverlaps(game->inputs.mousePosition + player->pos, vOne)).size())
		{
			player->heldEntity = hitEntities[0];
			player->heldEntity->holder = player;
		}
		else if (!game->inputs.space.held && tTime - player->lastClick > currentShootingItem.shootSpeed && player->currentMenuedEntity == nullptr &&
			game->inputs.mousePosition != vZero && currentShootingItem != *dItem && game->inputs.leftMouse.held && player->items.TryTake(currentShootingItem))
		{
			player->lastClick = tTime;
			game->entities->push_back(basicShotItem->Clone(currentShootingItem,
				player->pos, game->inputs.mousePosition, player));
		}

		if (player->heldEntity == nullptr && game->inputs.space.held && tTime - player->lastVac > player->timePerVac)
		{
			game->entities->Vacuum(player->pos, player->vacDist);
			player->lastVac = tTime;
		}

		vector<Entity*> collectibles = EntitiesOverlaps(player->pos, player->dimensions, game->entities->collectibles);
		for (Entity* collectible : collectibles)
		{
			player->items.push_back(((Collectible*)collectible)->baseItem);
			collectible->DestroySelf(player);
		}

		game->inputs.mouseScroll = 0;
	}
}