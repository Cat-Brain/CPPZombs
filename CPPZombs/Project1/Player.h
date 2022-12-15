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

	Player(Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Entity(pos, color, mass, maxHealth, health, name), vacDist(6 * GRID_SIZE)
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
		items.push_back(Shootables::smallPrinter->Clone());
		items.push_back(Shootables::smallVacuum->Clone());
		items.currentIndex = 0; // Copper
	}

	void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime) override
	{
		bool exitedMenu = false;
		if (currentMenuedEntity != nullptr && (inputs.leftMouse.bPressed || inputs.rightMouse.bPressed) && !currentMenuedEntity->PosInUIBounds(screen->GetMousePos()))
		{
			currentMenuedEntity->shouldUI = false;
			currentMenuedEntity = nullptr;
			exitedMenu = true;
			lastClick = tTime;
		}

		vector<Entity*>::iterator hitEntity;
		if (inputs.rightMouse.bPressed && inputs.mousePosition != pos &&
			(currentMenuedEntity == nullptr || currentMenuedEntity->pos != inputs.mousePosition)
			&& (hitEntity = ((Entities*)entities)->FindCorpPos(inputs.mousePosition)) != ((Entities*)entities)->corporeals.end())
		{
			if (currentMenuedEntity != nullptr)
				currentMenuedEntity->shouldUI = false;
			currentMenuedEntity = *hitEntity;
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
				screen->inputs.a.bHeld = false;
				inputs.left.bHeld = false;
				direction.x--;
			}
			if (inputs.d.bHeld || inputs.right.bHeld)
			{
				screen->inputs.d.bHeld = false;
				inputs.right.bHeld = false;
				direction.x++;
			}
			if (inputs.s.bHeld || inputs.down.bHeld)
			{
				screen->inputs.s.bHeld = false;
				inputs.down.bHeld = false;
				direction.y--;
			}
			if (inputs.w.bHeld || inputs.up.bHeld)
			{
				screen->inputs.w.bHeld = false;
				inputs.up.bHeld = false;
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
			items.currentIndex = JMod(items.currentIndex + inputs.mouseScroll, items.size());

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
			(hitEntity = ((Entities*)entities)->FindCorpPos(inputs.mousePosition)) != ((Entities*)entities)->corporeals.end() && *hitEntity != nullptr)
		{
			heldEntity = *hitEntity;
			heldEntity->holder = this;
		}
		else
		{
			Vec2 movePos = pos + Squarmalized(inputs.mousePosition - pos);
			if (!inputs.space.bHeld && tTime - lastClick > clickSpeed && currentMenuedEntity == nullptr && currentShootingItem != *dItem &&
				inputs.leftMouse.bHeld && inputs.mousePosition != playerPos &&
				((Entities*)entities)->FindCorpPos(movePos) == ((Entities*)entities)->corporeals.end() &&
				items.TryTake(currentShootingItem))
			{
				lastClick = tTime;
				ShotItem* shot = new ShotItem(basicShotItem, { currentShootingItem }, pos, inputs.mousePosition - pos, this);
				entities->push_back(shot);
			}
		}
		playerPos = pos;

		if (tTime - lastVac >= vacSpeed && heldEntity == nullptr && inputs.space.bHeld)
		{
			lastVac = tTime;
			((Entities*)entities)->Vacuum(pos, vacDist);
		}

		vector<Collectible*> collectibles = CollectiblesAtEPos(pos, ((Entities*)entities)->collectibles);
		for (Collectible* collectible : collectibles)
		{
			items.push_back(collectible->baseClass);
			collectible->DestroySelf(entities);
		}
	}

	void OnDeath(vector<Entity*>* entities, Entity* damageDealer) override
	{
		playerAlive = false;
		deathCauseName = damageDealer->name;
	}
};