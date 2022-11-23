#include "Enemy.h"

class Player : public Entity
{
public:
	Items items;
	//Entity* currentPlacingItem;
	int vacDist;
	bool placedBlock;
	Vec2 placingDir = up;

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
		items.currentIndex = 0; // Copper
	}

	void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		/*RotateLeft(placingDir, inputs.mouseScroll);
		RotateRight(placingDir, -inputs.mouseScroll);*/

		if (items.size() > 0)
			items.currentIndex = JMod(items.currentIndex + inputs.mouseScroll, items.size());

		if (inputs.leftMouse.bPressed && !inputs.space.bHeld && inputs.mousePosition != playerPos && items.TryTake(items.GetCurrentItem()))
		{
			ShotItem* shot = new ShotItem(basicShotItem, { items.GetCurrentItem() }, pos, inputs.mousePosition - pos, this);
			entities->push_back(shot);
		}
		/*if (placedBlock = (inputs.rightMouse.bHeld && !inputs.space.bHeld && ((Entities*)entities)->FindCorpPos(inputs.mousePosition) == ((Entities*)entities)->corporeals.end() && items.TryMake(currentPlacingItem->cost)))
		{
			Entity* placed = currentPlacingItem->Clone(inputs.mousePosition, placingDir, this);
			entities->push_back(placed);
		}*/


		if (frameCount % 4 == 0)
		{
			Vec2 direction(0, 0);

			if (inputs.a.bHeld || inputs.left.bHeld)
				direction.x--;
			if (inputs.d.bHeld || inputs.right.bHeld)
				direction.x++;
			if (inputs.s.bHeld || inputs.down.bHeld)
				direction.y--;
			if (inputs.w.bHeld || inputs.up.bHeld)
				direction.y++;

			Vec2 oldPos = pos;
			if (direction != Vec2(0, 0))
				Entity::TryMove(direction, 3, entities);

			playerVel = pos - oldPos;
		}
		playerPos = pos;

		if (inputs.space.bHeld)
			((Entities*)entities)->Vacuum(pos, vacDist);
		vector<Collectible*> collectibles = CollectiblesAtEPos(pos, ((Entities*)entities)->collectibles);
		for (Collectible* collectible : collectibles)
		{
			items.push_back(collectible->baseClass);
			collectible->DestroySelf(entities);
		}

		Entity::Update(screen, entities, frameCount, inputs);
	}

	void OnDeath(vector<Entity*>* entities) override
	{
		playerAlive = false;
	}

	void DUpdate(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		Entity::DUpdate(screen, entities, frameCount, inputs);
		//currentPlacingItem->Draw(inputs.mousePosition, Color(currentPlacingItem->color.r, currentPlacingItem->color.g, currentPlacingItem->color.b, 63), screen, entities, frameCount, inputs, placingDir);
	}
};