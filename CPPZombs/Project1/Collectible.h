#include "Particles.h"

class Collectible : public Entity
{
public:
	Item baseItem;

	Collectible(Item baseItem, Vec3 pos = vZero) :
		Entity(pos, baseItem.radius, baseItem.color, 1, 1, 1, baseItem.name), baseItem(baseItem)
	{
		isCollectible = true;
		vUpdate = VUPDATE::FRICTION;
	}

	Collectible(Item baseItem, Vec3 pos, RGBA color) :
		Entity(pos, baseItem.radius, color, 1, 1, 1, baseItem.name), baseItem(baseItem)
	{
		corporeal = false;
		isCollectible = true;
		vUpdate = VUPDATE::FRICTION;
	}

	Collectible(Collectible* baseClass, Vec3 pos) : Collectible(*baseClass) { this->pos = pos; }

	unique_ptr<Entity> Clone(Vec3 pos = Vec3(0), Vec3 dir = up, Entity* creator = nullptr) override
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