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
	int vacDist;

	Player(Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1, string name = "NULL NAME") :
		Entity(pos, color, mass, maxHealth, health, name), vacDist(6)
	{
		Start();
	}

	void Start() override
	{
		items = Items(0);
		items.push_back(Item(cheese, 50));
		items.push_back(Item(basicBullet, 100));
	}

	void Update(Screen* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) override
	{
		if (inputs.leftMouse.bPressed && !inputs.space.bHeld && inputs.mousePosition != playerPos && items.TryTake(Item(basicBullet, -1)))
		{
			Projectile* projectile = new Projectile(basicBullet, this, pos, inputs.mousePosition);
			entities->push_back(projectile);
		}
		if (inputs.rightMouse.bHeld && !inputs.space.bHeld && ((Entities*)entities)->FindCorpPos(inputs.mousePosition) == ((Entities*)entities)->corporeals.end() && items.TryTake(Item(cheese, -1)))
		{
			Placeable* placed = new Placeable(cheese, inputs.mousePosition);
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
			for (Entity* entity : ((Entities*)entities)->incorporeals)
			{
				int distance = Diagnistance(pos, entity->pos);
				if (distance > 0 && distance <= vacDist)
				{
					entity->pos += Squarmalized(pos - entity->pos);
				}
			}
		vector<Entity*> incorporeals = IncorporealsAtPos(pos, entities);
		for (Entity* entity : incorporeals)
		{
			items.push_back(Item(entity->baseClass, 1));
			entity->DestroySelf(entities);
		}

		Entity::Update(screen, entities, frameCount, inputs);
	}

	void OnDeath(vector<Entity*>* entities) override
	{
		playerAlive = false;
	}
};