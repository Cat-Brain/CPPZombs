#include "Collectible.h"

class LightSource
{
protected:
public:
	Vec2f pos;
	JRGB color;
	float range;

	LightSource(Vec2f pos = Vec2f(0, 0), JRGB color = JRGB(255, 255, 255), float range = 15.0f) :
		pos(pos), color(color), range(range) { }
};