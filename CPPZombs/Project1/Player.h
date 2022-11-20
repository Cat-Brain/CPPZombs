#include "Enemy.h"

struct Item
{
	Entity* type;
	int count;

	Item(Entity* type, int count = 0):
		type(type), count(count) { }

	Item() = default;
};

class Items : public vector<Item>
{
public:
	using vector<Item>::vector;
	void push_back(Item item)
	{
		for (int i = 0; i < size(); i++)
			if ((*this)[i].type == item.type)
			{
				(*this)[i].count += item.count;
				return;
			}
		vector<Item>::push_back(item);
	}

	bool TryTake(Item item)// Count should be negative.
	{
		for (int i = 0; i < size(); i++)
		{
			if ((*this)[i].type == item.type)
			{
				if ((*this)[i].count + item.count < 0)
					return false;
				(*this)[i].count += item.count;
				return true;
			}
		}
		return false;
	}

	void DUpdate(Screen* screen)
	{
		for (int i = 0; i < size(); i++)
		{
			screen->DrawString(Vec2(0, screen->ScreenWidth() - 7 * (i + 1)), to_string((*this)[i].count), (*this)[i].type->color);
		}
	}
};

class Player : public Entity
{
public:
	Items items;
	Entity* currentPlacingItem;
	int vacDist;
	bool placedBlock;

	Player(Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Entity(pos, color, mass, maxHealth, health, name), vacDist(6 * GRID_SIZE)
	{
		Start();
	}

	void Start() override
	{
		items = Items(0);
		items.push_back(Item(duct, 50));
		items.push_back(Item(basicBullet, 100));
		currentPlacingItem = duct;
	}

	void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		duct->RotateLeft(inputs.mouseScroll);
		duct->RotateRight(-inputs.mouseScroll);

		if (inputs.leftMouse.bPressed && !inputs.space.bHeld && inputs.mousePosition != playerPos && items.TryTake(Item(basicBullet, -1)))
		{
			Projectile* projectile = new Projectile(basicBullet, this, pos, inputs.mousePosition);
			entities->push_back(projectile);
		}
		if (placedBlock = (inputs.rightMouse.bHeld && !inputs.space.bHeld && ((Entities*)entities)->FindCorpPos(inputs.mousePosition) == ((Entities*)entities)->corporeals.end() && items.TryTake(Item(currentPlacingItem, -1))))
		{
			Entity* placed = currentPlacingItem->Clone(inputs.mousePosition);
			entities->push_back(placed);
		}


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

		if(inputs.space.bHeld)
			for (Collectible* collectible : ((Entities*)entities)->collectibles)
			{
				int distance = Diagnistance(ToCSpace(pos), collectible->pos);
				if (collectible->active && distance > 0 && distance <= vacDist)
				{
					collectible->pos += Squarmalized(ToCSpace(pos) - collectible->pos);
				}
			}
		vector<Collectible*> collectibles = CollectiblesAtEPos(pos, ((Entities*)entities)->collectibles);
		for (Collectible* collectible : collectibles)
		{
			items.push_back(Item((Entity*)collectible->baseClass, 1));
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
		duct->Draw(inputs.mousePosition, duct->dir, Color(duct->color.r, duct->color.g, duct->color.b, 63), screen, entities, frameCount, inputs);
	}
};