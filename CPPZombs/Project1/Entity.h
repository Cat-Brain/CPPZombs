#include "Include.h"

class Entity
{
public:
	Vec2 pos;
	Color color;
	int mass;
	int maxHealth, health;

	Entity(Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1) :
		pos(ToSpace(pos)), color(color), mass(mass), maxHealth(maxHealth), health(health)
	{}

	virtual void Update(olc::PixelGameEngine* screen, vector<Entity*> entities, int frameCount, Inputs inputs) // Also draws.
	{
		screen->Draw(pos.x, screenHeight - pos.y - 1, color);
	}

	virtual bool TryMove(Vec2 direction, int force, vector<Entity*> entities)
	{
		Vec2 newPos = pos + direction;
		newPos.x = JMod(newPos.x, screenWidth);
		newPos.y = JMod(newPos.y, screenHeight);

		if (force > 0)
		{
			for (int i = 0; i < entities.size(); i++)
				if (entities[i]->pos == newPos)
				{
					if (!entities[i]->TryMove(direction, force - entities[i]->mass, entities))
						return false;
					break;
				}
		}
		else return false;

		pos = newPos;
	}

	virtual bool CanAttack()
	{
		return true;
	}

	static Vec2 ToSpace(Vec2 positionInWorldSpace)
	{
		return Vec2(positionInWorldSpace.x, screenHeight - positionInWorldSpace.y - 1);
	}
};

bool EmptyFromEntities(Vec2 pos, vector<Entity*> entities)
{
	for (int i = 0; i < entities.size(); i++)
		if (entities[i]->pos == pos)
			return false;
	return true;
}

bool TryAndAttack(Vec2 pos, int damage, vector<Entity*>* entities) // Returns 0 if successful, 1 if damaged, 2 if missed.
{
	for (int i = 0; i < entities->size(); i++)
		if ((*entities)[i]->pos == pos && (*entities)[i]->CanAttack())
		{
			(*entities)[i]->health -= damage;
			if ((*entities)[i]->health < 1)
			{
				delete (*entities)[i];
				entities->erase(entities->begin() + i);
				return 0;
			}
			return 1;
		}
	return 2;
}