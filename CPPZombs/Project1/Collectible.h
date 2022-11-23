#include "Item.h"

class Collectible
{
public:
	Item baseClass;
	Vec2 pos;
	Color color;
	bool active;

	Collectible(Item baseClass, Vec2 pos = vZero) :
		baseClass(baseClass), pos(pos), color(baseClass.color), active(true) { }

	Collectible(Item baseClass, Vec2 pos, Color color) :
		baseClass(baseClass), pos(pos), color(color), active(true) { }

	virtual void DUpdate(Screen* screen, void* entities, int frameCount, Inputs inputs)
	{
		screen->Draw(ToRSpace2(pos), color);
	}

	virtual Collectible* Clone(Vec2 pos = vZero, Vec2 dir = vZero)
	{
		return new Collectible(baseClass, pos, this->color);
	}

	virtual bool TryMove(Vec2 direction, int force, void* entities, void* ignore = nullptr); // returns if item was hit.
	virtual bool TryMove(Vec2 direction, int force, void* entities, void** hitEntity, void* ignore = nullptr); // returns if item was hit.

	void DestroySelf(void* entities);
};

void Item::OnDeath(vector<void*>* collectibles, vector<void*>* entities, Vec2 pos)
{
	((vector<Collectible*>*)collectibles)->push_back(new Collectible(*this, ToRandomCSpace(pos)));
}

namespace Collectibles
{
	Collectible* copper = new Collectible(*Resources::copper, vZero);
	Collectible* iron = new Collectible(*Resources::iron, vZero);
}