#include "Include.h"

class Collectible
{
public:
	void* baseClass;
	Vec2 pos;
	Color color;
	bool active;

	Collectible(void* baseClass, Vec2 pos = vZero, Color color = olc::MAGENTA) :
		baseClass(baseClass), pos(pos), color(color), active(true) { }

	virtual void DUpdate(Screen* screen, void* entities, int frameCount, Inputs inputs)
	{
		screen->Draw(ToRSpace2(pos), color);
	}

	virtual bool TryMove(Vec2 direction, int force, void* entities, void* ignore = nullptr); // returns if item was hit.
	virtual bool TryMove(Vec2 direction, int force, void* entities, void** hitEntity, void* ignore = nullptr); // returns if item was hit.

	void DestroySelf(void* entities);
};