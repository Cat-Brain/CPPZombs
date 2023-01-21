#include "Entity.h"

class Collectible : public Entity
{
public:
	Item baseItem;

	Collectible(Item baseItem, Vec2 pos = vZero) :
		Entity(pos, baseItem.dimensions, baseItem.color, 1, 1, 1, baseItem.name), baseItem(baseItem) { }

	Collectible(Item baseItem, Vec2 pos, Color color) :
		Entity(pos, baseItem.dimensions, color, 1, 1, 1, baseItem.name), baseItem(baseItem) { }

	Collectible(Collectible* baseClass, Vec2 pos) : Collectible(*baseClass) { this->pos = pos; }

	Entity* Clone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr) override
	{
		return new Collectible(this, pos);
	}


	virtual Collectible* Clone(int count)
	{
		return new Collectible(baseItem.Clone(count), pos, color);
	}

	bool Corporeal() override
	{
		return false;
	}

	bool IsCollectible() override
	{
		return true;
	}
};

namespace Collectibles
{
	Collectible* copper = new Collectible(*Resources::copper, vZero);
	Collectible* iron = new Collectible(*Resources::iron, vZero);
	Collectible* rock = new Collectible(*Resources::rock, vZero);
}


#pragma region Other Collectible funcitons
vector<Entity*> EntitiesAtPos(Vec2 pos, vector<Entity*> entities)
{
	vector<Entity*> foundCollectibles(0);
	for (vector<Entity*>::iterator i = entities.begin(); i != entities.end(); i++)
		if ((*i)->pos == pos)
			foundCollectibles.push_back(*i);
	return foundCollectibles;
}

Entity* FindAEntity(Vec2 pos, vector<Entity*> entities)
{
	for (vector<Entity*>::iterator i = entities.begin(); i != entities.end(); i++)
		if ((*i)->pos == pos)
			return *i;
	return nullptr;
}

vector<Entity*> EntitiesOverlaps(Vec2 pos, Vec2 dimensions, vector<Entity*> entities)
{
	vector<Entity*> foundCollectibles(0);
	for (vector<Entity*>::iterator i = entities.begin(); i != entities.end(); i++)
		if ((*i)->Overlaps(pos, dimensions))
			foundCollectibles.push_back(*i);
	return foundCollectibles;
}
#pragma endregion