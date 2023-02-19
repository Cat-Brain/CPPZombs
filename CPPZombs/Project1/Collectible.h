#include "Particles.h"

class Collectible : public Entity
{
public:
	Item baseItem;

	Collectible(Item baseItem, Vec2 pos = vZero) :
		Entity(pos, baseItem.dimensions, baseItem.color, baseItem.subScat, 1, 1, 1, baseItem.name), baseItem(baseItem)
	{
		corporeal = false;
		isCollectible = true;
	}

	Collectible(Item baseItem, Vec2 pos, RGBA color) :
		Entity(pos, baseItem.dimensions, color, color, 1, 1, 1, baseItem.name), baseItem(baseItem)
	{
		corporeal = false;
		isCollectible = true;
	}

	Collectible(Collectible* baseClass, Vec2 pos) : Collectible(*baseClass) { this->pos = pos; }

	unique_ptr<Entity> Clone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr) override
	{
		return make_unique<Collectible>(this, pos);
	}


	virtual Collectible* Clone(int count)
	{
		return new Collectible(baseItem.Clone(count), pos, color);
	}
};

namespace Collectibles
{
	Collectible* copper = new Collectible(*Resources::copper, vZero);
	Collectible* iron = new Collectible(*Resources::iron, vZero);
	Collectible* rock = new Collectible(*Resources::rock, vZero);
	Collectible* bowler = new Collectible(*Resources::bowler, vZero);
}