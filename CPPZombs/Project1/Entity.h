#include "Include.h"

class Entity
{
public:
	Vec2 pos;
	Color color;
	int mass;
	int maxHealth, health;
	vector<Entity*> containedEntities;

	Entity(Vec2 pos = Vec2(0, 0), Color color = Color(olc::WHITE), int mass = 1, int maxHealth = 1, int health = 1) :
		pos(ToSpace(pos)), color(color), mass(mass), maxHealth(maxHealth), health(health), containedEntities()
	{
		Start();
	}

	virtual void Start()
	{}

	virtual void Update(olc::PixelGameEngine* screen, vector<Entity*>* entities, int frameCount, Inputs inputs) // Also draws.
	{
		screen->FillRect(ToSpace(pos - camPos + screenDimH) * 3, Vec2(3, 3), color);
	}

	virtual int TryMove(Vec2 direction, int force, vector<Entity*> entities) // returns index of hit item.
	{
		int index = 0;
		Vec2 newPos = pos + direction;

		if (force > 0)
		{
			for (int i = 0; i < entities.size(); i++)
				if (entities[i]->pos == newPos)
				{
					if ((index = entities[i]->TryMove(direction, force - entities[i]->mass, entities)) == -1)
						return -1;
					break;
				}
		}
		else return -1;

		pos = newPos;
		return index;
	}

	virtual bool CanAttack()
	{
		return true;
	}

	virtual bool CanConveyer()
	{
		return true;
	}

	virtual bool IsConveyer()
	{
		return false;
	}

	static Vec2 ToSpace(Vec2 positionInWorldSpace)
	{
		return Vec2(positionInWorldSpace.x, screenHeight - positionInWorldSpace.y - 1);
	}
};




#pragma region Other Entity funcitons
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
#pragma endregion



class Entities : public vector<Entity*>
{
public:
	vector<Entity*> conveyers;
	vector<Entity*> movers;

	void Update()
	{
#pragma region Conveyers
		conveyers.clear();
		for (int i = 0; i < size(); i++)
		{
			if ((*this)[i]->IsConveyer())
				conveyers.push_back((*this)[i]);
		}
#pragma endregion
	}
};