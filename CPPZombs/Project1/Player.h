#include "Enemy.h"

class Player : public Entity
{
public:
	Entity* currentMenuedEntity = nullptr;
	Items items;
	int vacDist;
	bool placedBlock;
	Vec2 placingDir = up;
	float lastMove = -1.0f, moveSpeed = 0.125f, lastBMove = -1.0f, bMoveSpeed = 0.125f,
		lastVac = -1.0f, vacSpeed = 0.0625f, lastClick = -1.0f, clickSpeed = 0.25f;

	Player(Vec2 pos = Vec2(0, 0), Vec2 dimensions = Vec2(1, 1), int vacDist = 6, Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Entity(pos, dimensions, color, mass, maxHealth, health, name), vacDist(vacDist)
	{
		Start();
	}

	void Start() override
	{
		items = Items();
		items.push_back(Resources::copper->Clone(10));
		items.push_back(Resources::iron->Clone());
		items.push_back(copperTreeSeed->Clone(3));
		items.push_back(ironTreeSeed->Clone());
		items.push_back(cheeseTreeSeed->Clone());
		items.push_back(Shootables::smallPrinter->Clone());
		items.push_back(Shootables::smallVacuum->Clone());
		items.currentIndex = 0; // Copper
	}

	void Update(Game* game, Entities* entities, int frameCount, Inputs inputs, float dTime) override
	{
		bool exitedMenu = false;
		if (currentMenuedEntity != nullptr && (inputs.leftMouse.bPressed || inputs.rightMouse.bPressed) && !currentMenuedEntity->PosInUIBounds(game->GetMousePos()))
		{
			currentMenuedEntity->shouldUI = false;
			currentMenuedEntity = nullptr;
			exitedMenu = true;
			lastClick = tTime;
		}

		vector<Entity*> hitEntity;
		if (inputs.rightMouse.bPressed && inputs.mousePosition != pos &&
			(currentMenuedEntity == nullptr || currentMenuedEntity->pos != inputs.mousePosition)
			&& (hitEntity = entities->FindCorpOverlaps(inputs.mousePosition, dimensions)).size())
		{
			if (currentMenuedEntity != nullptr)
				currentMenuedEntity->shouldUI = false;
			currentMenuedEntity = hitEntity[0];
			currentMenuedEntity->shouldUI = true;
		}
		if (inputs.w.bPressed || inputs.up.bPressed || inputs.a.bPressed || inputs.left.bPressed ||
			inputs.s.bPressed || inputs.down.bPressed || inputs.d.bPressed || inputs.right.bPressed)
			lastMove = tTime;
		// Player movement code:
		if (tTime - lastMove >= moveSpeed)
		{
			Vec2 direction(0, 0);

			#pragma region Inputs
			if (inputs.a.bHeld || inputs.left.bHeld)
			{
				game->inputs.a.bHeld = false;
				game->inputs.left.bHeld = false;
				direction.x--;
			}
			if (inputs.d.bHeld || inputs.right.bHeld)
			{
				game->inputs.d.bHeld = false;
				game->inputs.right.bHeld = false;
				direction.x++;
			}
			if (inputs.s.bHeld || inputs.down.bHeld)
			{
				game->inputs.s.bHeld = false;
				game->inputs.down.bHeld = false;
				direction.y--;
			}
			if (inputs.w.bHeld || inputs.up.bHeld)
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
				TryMove(direction, 3, entities, nullptr);
			}

			playerVel = pos - oldPos;
		}


		if (heldEntity == nullptr && items.size() > 0) // You can't mod by 0.
			items.currentIndex = JMod(items.currentIndex + inputs.mouseScroll, static_cast<int>(items.size()));

		Item currentShootingItem = items.GetCurrentItem();

		if (inputs.middleMouse.bReleased && heldEntity != nullptr)
		{
			heldEntity->holder = nullptr;
			heldEntity = nullptr;
		}

		if (heldEntity != nullptr)
		{
			RotateLeft(heldEntity->dir, inputs.mouseScroll);
			RotateRight(heldEntity->dir, -inputs.mouseScroll);
		}

		if (tTime - lastBMove >= bMoveSpeed && heldEntity != nullptr && heldEntity->pos != inputs.mousePosition && heldEntity->pos != pos)
		{
			lastBMove = tTime;
			heldEntity->TryMove(Squarmalized(inputs.mousePosition - heldEntity->pos), 1, entities);
		}

		if (heldEntity == nullptr && inputs.middleMouse.bPressed && inputs.mousePosition != pos &&
			(hitEntity = entities->FindCorpOverlaps(inputs.mousePosition, dimensions)).size())
		{
			heldEntity = hitEntity[0];
			heldEntity->holder = this;
		}
		else if (!inputs.space.bHeld && tTime - lastClick > clickSpeed && currentMenuedEntity == nullptr &&
			currentShootingItem != *dItem && inputs.leftMouse.bHeld && items.TryTake(currentShootingItem))
		{
			lastClick = tTime;
			Entity* shot = basicShotItem->Clone(currentShootingItem.Clone(), pos, inputs.mousePosition - pos, this);
			entities->push_back(shot);
		}
		playerPos = pos;

		if (tTime - lastVac >= vacSpeed && heldEntity == nullptr && inputs.space.bHeld)
		{
			lastVac = tTime;
			entities->Vacuum(pos, vacDist);
		}

		vector<Entity*> collectibles = EntitiesAtPos(pos, entities->collectibles);
		for (Entity* collectible : collectibles)
		{
			items.push_back(((Collectible*)collectible)->baseItem);
			collectible->DestroySelf(entities, this);
		}
	}

	void OnDeath(Entities* entities, Entity* damageDealer) override
	{
		playerAlive = false;
		deathCauseName = damageDealer->name;
	}
};