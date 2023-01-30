#include "Particles.h"

class Collectible : public Entity
{
public:
	Item baseItem;

	Collectible(Item baseItem, Vec2 pos = vZero) :
		Entity(pos, baseItem.dimensions, baseItem.color, baseItem.color, 1, 1, 1, baseItem.name), baseItem(baseItem) { }

	Collectible(Item baseItem, Vec2 pos, RGBA color) :
		Entity(pos, baseItem.dimensions, color, color, 1, 1, 1, baseItem.name), baseItem(baseItem) { }

	Collectible(Collectible* baseClass, Vec2 pos) : Collectible(*baseClass) { this->pos = pos; }

	unique_ptr<Entity> Clone(Vec2 pos = vZero, Vec2 dir = up, Entity* creator = nullptr) override
	{
		return make_unique<Collectible>(this, pos);
	}

	void VUpdate() override
	{
		vel *= powf(0.25f, game->dTime);
		Entity::VUpdate();
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