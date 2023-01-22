#include "Enemy.h"

class Player : public LightBlock
{
public:
	Entity* currentMenuedEntity = nullptr;
	Items items;
	int vacDist;
	bool placedBlock;
	Vec2 placingDir = up;
	float lastMove = -1.0f, moveSpeed = 0.125f, lastBMove = -1.0f, bMoveSpeed = 0.125f,
		lastVac = -1.0f, vacSpeed = 0.0625f, lastClick = -1.0f, clickSpeed = 0.25f;

	Player(Vec2 pos = vZero, Vec2 dimensions = vOne, int vacDist = 6, Color color = olc::WHITE, Color color2 = olc::BLACK, JRGB lightColor = JRGB(127, 127, 127),
		Color subsurfaceResistance = olc::WHITE, int lightFalloff = 50, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		LightBlock(lightColor, lightFalloff, pos, dimensions, color, color2, subsurfaceResistance, mass, maxHealth, health, name), vacDist(vacDist)
	{
		Start();
	}

	void Start() override
	{
		LightBlock::Start();
		items = Items();
		items.push_back(Resources::copper->Clone(10));
		items.push_back(Resources::iron->Clone());
		items.push_back(Resources::cheese->Clone(10));
		items.push_back(Resources::Seeds::copperTreeSeed->Clone(2));
		for (Item* item : Resources::Seeds::plantSeeds)
			items.push_back(item->Clone());
		items.currentIndex = 0; // Copper
	}

	void Update() override
	{
		bool exitedMenu = false;
		if (currentMenuedEntity != nullptr && (game->inputs.leftMouse.bPressed || game->inputs.rightMouse.bPressed) &&
			!currentMenuedEntity->PosInUIBounds(game->GetMousePos()))
		{
			currentMenuedEntity->shouldUI = false;
			currentMenuedEntity = nullptr;
			exitedMenu = true;
			lastClick = tTime;
		}

		vector<Entity*> hitEntities;
		if (game->inputs.rightMouse.bPressed && game->inputs.mousePosition != pos &&
			(currentMenuedEntity == nullptr || currentMenuedEntity->pos != game->inputs.mousePosition)
			&& (hitEntities = game->entities->FindCorpOverlaps(game->inputs.mousePosition, dimensions)).size())
		{
			if (currentMenuedEntity != nullptr)
				currentMenuedEntity->shouldUI = false;
			currentMenuedEntity = hitEntities[0];
			currentMenuedEntity->shouldUI = true;
		}
		if (game->inputs.w.bPressed || game->inputs.up.bPressed || game->inputs.a.bPressed || game->inputs.left.bPressed ||
			game->inputs.s.bPressed || game->inputs.down.bPressed || game->inputs.d.bPressed || game->inputs.right.bPressed)
			lastMove = tTime;
		// Player movement code:
		if (tTime - lastMove >= moveSpeed)
		{
			Vec2 direction(0, 0);

			#pragma region Inputs
			if (game->inputs.a.bHeld || game->inputs.left.bHeld)
			{
				game->inputs.a.bHeld = false;
				game->inputs.left.bHeld = false;
				direction.x--;
			}
			if (game->inputs.d.bHeld || game->inputs.right.bHeld)
			{
				game->inputs.d.bHeld = false;
				game->inputs.right.bHeld = false;
				direction.x++;
			}
			if (game->inputs.s.bHeld || game->inputs.down.bHeld)
			{
				game->inputs.s.bHeld = false;
				game->inputs.down.bHeld = false;
				direction.y--;
			}
			if (game->inputs.w.bHeld || game->inputs.up.bHeld)
			{
				game->inputs.w.bHeld = false;
				game->inputs.up.bHeld = false;
				direction.y++;
			}
			#pragma endregion

			Vec2 oldPos = pos;
			if (direction != Vec2(0, 0))
			{
				lastMove = tTime;
				TryMove(direction, 3);
			}

			playerVel = pos - oldPos;
			game->inputs.mousePosition += playerVel;
		}


		if (heldEntity == nullptr && items.size() > 0) // You can't mod by 0.
			items.currentIndex = JMod(items.currentIndex + game->inputs.mouseScroll, static_cast<int>(items.size()));

		Item currentShootingItem = items.GetCurrentItem();

		if (game->inputs.middleMouse.bReleased && heldEntity != nullptr)
		{
			heldEntity->holder = nullptr;
			heldEntity = nullptr;
		}

		if (heldEntity != nullptr)
		{
			RotateLeft(heldEntity->dir, game->inputs.mouseScroll);
			RotateRight(heldEntity->dir, -game->inputs.mouseScroll);
		}

		if (tTime - lastBMove >= bMoveSpeed && heldEntity != nullptr && heldEntity->pos != game->inputs.mousePosition && heldEntity->pos != pos)
		{
			lastBMove = tTime;
			heldEntity->TryMove(Squarmalized(game->inputs.mousePosition - heldEntity->pos), 1);
		}
		
		
		Vec2f normalizedDir = Normalized(game->inputs.mousePosition - pos);
		Vec2 shootOffset = (dimensions + currentShootingItem.dimensions - vOne) * Vec2(static_cast<int>(roundf(normalizedDir.x)), static_cast<int>(roundf(normalizedDir.y)));
		if (heldEntity == nullptr && game->inputs.middleMouse.bPressed && game->inputs.mousePosition != pos &&
			(hitEntities = game->entities->FindCorpOverlaps(game->inputs.mousePosition, vOne)).size())
		{
			heldEntity = hitEntities[0];
			heldEntity->holder = this;
		}
		else if (!game->inputs.space.bHeld && tTime - lastClick > clickSpeed && currentMenuedEntity == nullptr && game->inputs.mousePosition != pos &&
			(!bool((hitEntities = game->entities->FindCorpOverlaps(pos + shootOffset, currentShootingItem.dimensions)).size()) ||
				currentShootingItem.damage >= hitEntities[0]->health || currentShootingItem.typeName == "Ammo") &&
			currentShootingItem != *dItem && game->inputs.leftMouse.bHeld && items.TryTake(currentShootingItem))
		{
			lastClick = tTime;
			game->entities->push_back(basicShotItem->Clone(currentShootingItem, pos + shootOffset, game->inputs.mousePosition - pos - shootOffset, this));
		}
		playerPos = pos;

		if (tTime - lastVac >= vacSpeed && heldEntity == nullptr && game->inputs.space.bHeld)
		{
			lastVac = tTime;
			game->entities->Vacuum(pos, vacDist);
		}

		vector<Entity*> collectibles = EntitiesOverlaps(pos, dimensions, game->entities->collectibles);
		for (Entity* collectible : collectibles)
		{
			items.push_back(((Collectible*)collectible)->baseItem);
			collectible->DestroySelf(this);
		}
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