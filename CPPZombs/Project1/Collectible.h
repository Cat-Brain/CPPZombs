#include "Particles.h"

class Collectible : public Entity
{
public:
	ItemInstance baseItem;

	Collectible(ItemInstance baseItem, Vec3 pos = vZero) :
		Entity(pos, baseItem->radius, baseItem->color, 1, 1, 1, baseItem->name), baseItem(baseItem)
	{
		isCollectible = true;
		vUpdate = VUPDATE::FRICTION;
		update = UPDATE::COLLECTIBLE;
	}

	Collectible(ItemInstance baseItem, Vec3 pos, RGBA color) :
		Entity(pos, baseItem->radius, color, 1, 1, 1, baseItem->name), baseItem(baseItem)
	{
		corporeal = false;
		isCollectible = true;
		vUpdate = VUPDATE::FRICTION;
		radius = FindRadius();
		update = UPDATE::COLLECTIBLE;
	}

	float FindRadius()
	{
		return cbrtf(baseItem->radius * baseItem->radius * baseItem->radius * baseItem.count);
	}

	Collectible(Collectible* baseClass, Vec3 pos) : Collectible(*baseClass) { this->pos = pos; }

	unique_ptr<Entity> Clone(Vec3 pos = Vec3(0), Vec3 dir = north, Entity* creator = nullptr) override
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
	Collectible* copper = new Collectible(Resources::copper.Clone());
	Collectible* iron = new Collectible(Resources::iron.Clone());
	Collectible* rock = new Collectible(Resources::rock.Clone());
	Collectible* bowler = new Collectible(Resources::bowler.Clone());
	Collectible* silver = new Collectible(Resources::silver.Clone());
}