#include "Enemy.h"

class Player : public Entity
{
public:
	Items items;
	//Entity* currentPlacingItem;
	int vacDist;
	bool placedBlock;
	Vec2 placingDir = up;
	float lastMove = -1.0f, moveSpeed = 0.125f, lastBMove = -1.0f, bMoveSpeed = 0.125f,
		lastVac = -1.0f, vacSpeed = 0.0625f, lastClick = -1.0f, clickSpeed = 0.25f;
	bool isCrafting = false;

	Player(Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), Recipe cost = Recipes::dRecipe, int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Entity(pos, color, cost, mass, maxHealth, health, name), vacDist(6 * GRID_SIZE)
	{
		Start();
	}

	void Start() override
	{
		items = Items();
		items.push_back(Resources::copper->Clone(10));
		items.push_back(Resources::iron->Clone(1));
		items.push_back(copperTreeSeed->Clone(1));
		items.currentIndex = 0; // Copper
	}

	void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs, float dTime) override
	{
		/*RotateLeft(placingDir, inputs.mouseScroll);
		RotateRight(placingDir, -inputs.mouseScroll);*/

		if (items.size() > 0)
			items.currentIndex = JMod(items.currentIndex + inputs.mouseScroll, items.size());

		isCrafting = isCrafting;

		if (isCrafting)
		{
			if inputs.mo
		}
		else
		{

			Item currentShootingItem = items.GetCurrentItem();

			if (inputs.rightMouse.bReleased && heldEntity != nullptr)
			{
				heldEntity->holder = nullptr;
				heldEntity = nullptr;
			}

			if (tTime - lastBMove >= bMoveSpeed && heldEntity != nullptr && heldEntity->pos != inputs.mousePosition && heldEntity->pos != pos)
			{
				lastBMove = tTime;
				heldEntity->TryMove(Squarmalized(inputs.mousePosition - heldEntity->pos), 1, entities);
			}

			vector<Entity*>::iterator hitEntity;
			if (heldEntity == nullptr && inputs.rightMouse.bPressed && inputs.mousePosition != pos && (hitEntity = ((Entities*)entities)->FindCorpPos(inputs.mousePosition)) != ((Entities*)entities)->corporeals.end() && *hitEntity != nullptr)
			{
				heldEntity = *hitEntity;
				heldEntity->holder = this;
			}
			else if (!inputs.space.bHeld && tTime - lastClick > clickSpeed && currentShootingItem != *dItem && inputs.leftMouse.bHeld && inputs.mousePosition != playerPos && items.TryTake(currentShootingItem))
			{
				lastClick = tTime;
				ShotItem* shot = new ShotItem(basicShotItem, { currentShootingItem }, pos, inputs.mousePosition - pos, this);
				entities->push_back(shot);
			}



			if (tTime - lastMove >= moveSpeed)
			{
				Vec2 direction(0, 0);

				#pragma region Inputs
				if (inputs.a.bHeld || inputs.left.bHeld)
				{
					screen->inputs.a.bHeld = false;
					screen->inputs.a.bPressed = false;
					screen->inputs.a.bReleased = false;
					inputs.left.bHeld = false;
					inputs.left.bPressed = false;
					inputs.left.bReleased = false;
					direction.x--;
				}
				if (inputs.d.bHeld || inputs.right.bHeld)
				{
					screen->inputs.d.bHeld = false;
					screen->inputs.d.bPressed = false;
					screen->inputs.d.bReleased = false;
					inputs.right.bHeld = false;
					inputs.right.bPressed = false;
					inputs.right.bReleased = false;
					direction.x++;
				}
				if (inputs.s.bHeld || inputs.down.bHeld)
				{
					screen->inputs.s.bHeld = false;
					screen->inputs.s.bPressed = false;
					screen->inputs.s.bReleased = false;
					inputs.down.bHeld = false;
					inputs.down.bPressed = false;
					inputs.down.bReleased = false;
					direction.y--;
				}
				if (inputs.w.bHeld || inputs.up.bHeld)
				{
					screen->inputs.w.bHeld = false;
					screen->inputs.w.bPressed = false;
					screen->inputs.w.bReleased = false;
					inputs.up.bHeld = false;
					inputs.up.bPressed = false;
					inputs.up.bReleased = false;
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
			playerPos = pos;

			if (tTime - lastVac >= vacSpeed && heldEntity == nullptr && inputs.space.bHeld)
			{
				lastVac = tTime;
				((Entities*)entities)->Vacuum(pos, vacDist);
			}
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